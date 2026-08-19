#include "Tcp/TcpSelect/TcpSelectClient.h"
#include "Tcp/TcpUtility.h"
#include <Spark/Core/Logger/Logger.h>
#include <cstring>

using namespace std;
using namespace spark::core;

namespace spark::network
{
TcpSelectClient::TcpSelectClient(const char* addressName, int milliSeconds)
	:TcpSelectBase(ServerTypeType::Client, addressName, milliSeconds)
{
}

bool TcpSelectClient::ConnectToServer(const char* ip, unsigned short port)
{
	if (m_AddressInfo != nullptr)
	{
		freeaddrinfo(m_AddressInfo);
		m_AddressInfo = nullptr;
	}
	m_Address = ip;
	m_Port = std::to_string(port);
	auto ret = TcpUtility::GetAddrinfo(m_Address.c_str(), m_Port.c_str(), m_AddressInfo);
	if (ret < 0 || m_AddressInfo == nullptr)
	{
		WriteLog(LogLevel::Info, "GetAddrinfo Failed. Address:%s Port:%s ret:%d, Errno:%d", m_Address.c_str(), m_Port.c_str(), ret, errno);
		return false;
	}
	auto socketID = TcpUtility::PrepareSocket(m_AddressInfo->ai_family);
	ret = connect(socketID, m_AddressInfo->ai_addr, int(m_AddressInfo->ai_addrlen));
#ifdef WINDOWS
	auto error = WSAGetLastError();
	if (ret == -1 && error != WSAEWOULDBLOCK && error != WSAEINPROGRESS)
	{
		WriteLog(LogLevel::Warning, "ConnectToServer Failed. ip:%s, port:%d, errno:%d", ip, port, errno);
		closesocket(socketID);
		return false;
	}
#endif
#ifdef LINUX
	if (ret == -1 && errno != EINPROGRESS)
	{
		WriteLog(LogLevel::Warning, "ConnectToServer Failed. ip:%s, port:%d, errno:%d", ip, port, errno);
		closesocket(socketID);
		return false;
	}
#endif
	TcpConnect* tcpConnect = TcpConnect::Allocate(GetSessionID(), socketID, m_Address, m_Port);
	if (ret == 0)
	{
		AddConnect(tcpConnect);
	}
	else
	{
		m_Connectings.insert(make_pair(tcpConnect->SessionID, tcpConnect));
	}
	return true;
}
void TcpSelectClient::CheckConnect()
{
	if (m_Connectings.empty())
		return;
	FD_ZERO(&m_WriteFds);
	FD_ZERO(&m_ErrorFds);
	m_MaxID = 0;
	for (auto& it : m_Connectings)
	{
		auto connect = it.second;
		FD_SET(connect->SocketID, &m_WriteFds);
		FD_SET(connect->SocketID, &m_ErrorFds);
		if (connect->SocketID > m_MaxID)
		{
			m_MaxID = connect->SocketID;
		}
	}
	++m_MaxID;

	memcpy(&m_SelectSocketTimeOutTemp, &m_SelectSocketTimeOut, sizeof(timeval));
	::select((int)m_MaxID, nullptr, &m_WriteFds, &m_ErrorFds, &m_SelectSocketTimeOutTemp);
	for (auto& it : m_Connectings)
	{
		auto connect = (TcpConnect*)it.second;
		if (FD_ISSET(connect->SocketID, &m_WriteFds))
		{
			AddConnect(connect);
			m_ConnectSuccessedSessions.push_back(connect->SessionID);
		}
		if (FD_ISSET(connect->SocketID, &m_ErrorFds))
		{
			RemoveConnect(connect);
			m_ConnectFailedSessions.push_back(connect->SessionID);
		}
	}
	for (auto& sessionID : m_ConnectSuccessedSessions)
	{
		m_Connectings.erase(sessionID);
	}
	m_ConnectSuccessedSessions.clear();
	for (auto& sessionID : m_ConnectFailedSessions)
	{
		m_Connectings.erase(sessionID);
	}
	m_ConnectFailedSessions.clear();
}
}

