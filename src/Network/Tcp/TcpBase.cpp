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
TcpBase::TcpBase(ServerTypeType serverType, const char* addressName, int milliSeconds)
	:IOBase(serverType, addressName, milliSeconds), m_AddressInfo(nullptr), m_Socket(INVALID_SOCKET), m_SocketNotify(nullptr), m_RemoteAddressLen(sizeof(m_RemoteAddress))
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
		ConnectToServer(m_Address.c_str(), atoi(m_Port.c_str()));
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
		CheckConnect();
	DoDisConnect();
	HandleTcpEvent();
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
