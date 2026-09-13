#include "Tcp/TcpBase.h"
#include <Spark/Core/Platform/Platform.h>
#include <Spark/Core/Logger/Logger.h>
#include "Tcp/TcpUtility.h"
#include <Spark/Core/Utility/TimeUtility.h>
#include <cstring>
#include <assert.h>
#include <chrono>

using namespace std;
using namespace spark::core;

namespace spark::network
{
namespace
{
	// Client 断线自动重连的固定重试间隔
	constexpr int kAutoReconnectIntervalMs = 3000;
}

TcpBase::TcpBase(ServerTypeType serverType, const char* addressName, int milliSeconds)
	:IOBase(serverType, addressName, milliSeconds), m_AddressInfo(nullptr), m_Socket(INVALID_SOCKET), m_SocketNotify(nullptr), m_RemoteAddressLen(sizeof(m_RemoteAddress)),
	m_AutoConnectPending(false), m_LastConnectAttemptTime{}
{
	SocketInit::GetInstance().Init();
	memset(&m_RemoteAddress, 0, sizeof(m_RemoteAddress));
}
TcpBase::~TcpBase()
{
	if (m_Socket != INVALID_SOCKET)
	{
		closesocket(m_Socket);
		m_Socket = INVALID_SOCKET;
	}
}

bool TcpBase::Init()
{
	SocketInit::GetInstance().Init();
	m_SocketNotify = new SocketNotify();
	if (!m_SocketNotify->Init())
	{
		WriteLog(LogLevel::Error, "SocketNotify Init Failed.");
		return false;
	}
	if (m_ServerType == ServerTypeType::Client)
	{
		// 首连同步失败时复位,交由 IO 循环的自动重连兜底(修复首连失败后永久失联)
		m_AutoConnectPending = true;
		if (!ConnectToServer(m_Address.c_str(), atoi(m_Port.c_str())))
		{
			m_AutoConnectPending = false;
		}
	}
	else if (m_ServerType == ServerTypeType::Server)
	{
		auto ret = TcpUtility::GetAddrinfo(m_Address.c_str(), m_Port.c_str(), m_AddressInfo);
		if (ret < 0)
		{
			WriteLog(LogLevel::Info, "GetAddrinfo Failed. Address:%s Port:%s ret:%d, Errno:%d", m_Address.c_str(), m_Port.c_str(), ret, WSAGetLastError());
			return false;
		}
		m_Socket = TcpUtility::PrepareSocket(m_AddressInfo->ai_family);
		if (m_Socket == INVALID_SOCKET)
		{
			return false;
		}
		if (!TcpUtility::Bind(m_Socket, m_AddressInfo))
		{
			return false;
		}
		return TcpUtility::Listen(m_Socket);
	}
	return true;
}
void TcpBase::Send(SessionIDType sessionID, Buffer<BuffSize>* buffer)
{
	auto connect = (TcpConnect*)GetConnect(sessionID);
	if (connect == nullptr)
	{
		WriteLog(LogLevel::Warning, "Send Connect Not Exist, Drop Buffer. SessionID:%lld, Len:%d", sessionID, buffer->GetLength());
		buffer->Deallocate();
		return;
	}
	connect->PushBack(buffer);
	m_SocketNotify->Notify();
}
bool TcpBase::ConnectToServer(const char* address)
{
	ParseAddress(address, m_Address, m_Port);
	return ConnectToServer(m_Address.c_str(), atoi(m_Port.c_str()));
}
void TcpBase::HandleIOEvent()
{
	if (m_ServerType == ServerTypeType::Client)
	{
		CheckConnect();
		TryAutoReconnect();
	}
	DoDisConnect();
	HandleTcpEvent();
}
void TcpBase::AddConnect(Connect* connect)
{
	m_AutoConnectPending = false;
	IOBase::AddConnect(connect);
}
void TcpBase::RemoveConnect(Connect* connect)
{
	m_AutoConnectPending = false;
	IOBase::RemoveConnect(connect);
}
void TcpBase::TryAutoReconnect()
{
	if (m_AutoConnectPending || !m_Connects.empty())
	{
		return;
	}
	auto now = std::chrono::steady_clock::now();
	if (now - m_LastConnectAttemptTime < std::chrono::milliseconds(kAutoReconnectIntervalMs))
	{
		return;
	}
	m_LastConnectAttemptTime = now;
	WriteLog(LogLevel::Info, "TcpAutoReconnect: Attempt. Address:%s, Port:%s", m_Address.c_str(), m_Port.c_str());
	if (ConnectToServer(m_Address.c_str(), atoi(m_Port.c_str())))
	{
		m_AutoConnectPending = true;
	}
}
void TcpBase::DoSend(Connect* connect)
{
	auto buffer = connect->GetNextBuffer();
	while (buffer != nullptr)
	{
		int len = send(((TcpConnect*)connect)->SocketID, buffer->GetData(), buffer->GetLength(), 0);
		if (len > 0)
		{
			buffer->Shift(len);
			if (buffer->GetLength() > 0)
			{
				connect->PushFront(buffer);
				break;
			}
			else
			{
				buffer->Deallocate();
				buffer = connect->GetNextBuffer();
			}
		}
		else
		{
			auto errorID = WSAGetLastError();
#ifdef _WIN32
			if (errorID == WSAEWOULDBLOCK || errorID == WSAENOBUFS)
#endif
#ifdef __linux__
			if (errorID == EWOULDBLOCK || errorID == ENOBUFS || errorID == ENOMEM)
#endif
			{
				connect->PushFront(buffer);
				break;
			}
			else
			{
				WriteLog(LogLevel::Warning, "Tcp send Failed. SessionID:%lld, len:%d, errorID:%d", connect->SessionID, len, errorID);
				buffer->Deallocate();
				DisConnect(connect->SessionID);
			}
		}
	}
}
void TcpBase::DoRecv(Connect* connect)
{
	auto tcpConnect = (TcpConnect*)connect;
	Buffer<BuffSize>* buffer = Buffer<BuffSize>::Allocate();
	auto data = buffer->GetData();
	int len = recv(tcpConnect->SocketID, data, BuffSize - 1, 0);
	if (len <= 0)
	{
		WriteLog(LogLevel::Info, "DisConnect For Recv. SessionID:%lld, Socket:%lld, ErrorID:%d", tcpConnect->SessionID, tcpConnect->SocketID, len);
		buffer->Deallocate();
		DisConnect(tcpConnect->SessionID);
	}
	else if (m_IOSubscriber)
	{
		data[len] = '\0';
		WriteLog(LogLevel::Ignore, "OnRecv: SessionID:%lld, Socket:%lld, RecvLen:%d", tcpConnect->SessionID, tcpConnect->SocketID, len);
		buffer->SetLength(len);

		m_IOSubscriber->OnRecv(tcpConnect->SessionID, buffer);
	}
	else
	{
		buffer->Deallocate();
	}
}
void TcpBase::DoAccept()
{
	for (int i = 0; i < 5; i++)
	{
		SOCKET socketID = accept(m_Socket, (sockaddr*)&m_RemoteAddress, &m_RemoteAddressLen);
		if (socketID == INVALID_SOCKET)
		{
			break;
		}
		std::string ip, port;
		auto ret = TcpUtility::GetNameinfo((sockaddr*)&m_RemoteAddress, m_RemoteAddressLen, ip, port);
		TcpUtility::SetSockNodelay(socketID);
		auto connect = TcpConnect::Allocate(GetSessionID(), socketID, ip, port);
		AddConnect(connect);
	}
}
}
