#include "Tcp/TcpEpoll/TcpEpollClient.h"
#include <Spark/Core/Logger/Logger.h>
#include "Tcp/TcpUtility.h"

using namespace spark::core;

namespace spark::network
{
TcpEpollClient::TcpEpollClient(const char* addressName, int milliSeconds)
	:TcpEpollBase(ServerTypeType::Client, addressName, milliSeconds)
{
}
TcpEpollClient::~TcpEpollClient()
{
}

bool TcpEpollClient::ConnectToServer(const char* ip, unsigned short port)
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
	if (ret == -1 && errno != EINPROGRESS)
	{
		WriteLog(LogLevel::Warning, "ConnectToServer Failed. IP:%s, Port:%d, errno:%d", ip, port, errno);
		closesocket(socketID);
		return false;
	}
	TcpConnect* tcpConnect = TcpConnect::Allocate(GetSessionID(), socketID, m_Address, m_Port);
	if (ret == 0)
	{
		AddConnect(tcpConnect);
	}
	else
	{
		AddEpollConnectEvent(tcpConnect);
	}
	return true;
}
void TcpEpollClient::AddEpollConnectEvent(TcpConnect* connect)
{
#ifdef __linux__
	epoll_event epollEvent;
	epollEvent.data.ptr = connect;
	epollEvent.events = EPOLLOUT;
	epoll_ctl(m_EpollFd, EPOLL_CTL_ADD, connect->SocketID, &epollEvent);
#endif
}
}
