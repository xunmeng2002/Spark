#include "Tcp/SocketNotify.h"
#include "Tcp/TcpUtility.h"
#include "Tcp/TcpConnect.h"
#include <Spark/Core/Logger/Logger.h>

using namespace spark::core;

namespace spark::network
{
SocketNotify::SocketNotify()
	:m_Sockets{ INVALID_SOCKET, INVALID_SOCKET }, m_TcpConnect(nullptr), m_IP("127.0.0.1"), m_AddressInfo(nullptr), m_ReceiveBuffer{0}
{
}
SocketNotify::~SocketNotify()
{
	if (m_Sockets[0] != -1)
	{
		closesocket(m_Sockets[0]);
	}
	if (m_Sockets[1] != -1)
	{
		closesocket(m_Sockets[1]);
	}
}
bool SocketNotify::Init()
{
	if (!CreateSocketPair())
		return false;
	TcpUtility::SetSockUnblock(m_Sockets[0]);
	TcpUtility::SetSockUnblock(m_Sockets[1]);

	m_TcpConnect = TcpConnect::Allocate(0LL, m_Sockets[0], m_IP, "");
	return true;
}
bool SocketNotify::Notify()
{
	char buffer = 1;
	int len = send(m_Sockets[1], &buffer, 1, 0);
	return len == 1;
}
bool SocketNotify::Consume()
{
	int len = recv(m_Sockets[0], m_ReceiveBuffer, sizeof(m_ReceiveBuffer), 0);
	return len > 0;
}
SOCKET SocketNotify::GetReadSocket()
{
	return m_Sockets[0];
}
TcpConnect* SocketNotify::GetConnect()
{
	return m_TcpConnect;
}


bool SocketNotify::CreateSocketPair()
{
#ifdef _WIN32
	auto ret = TcpUtility::GetAddrinfo(m_IP.c_str(), 0, m_AddressInfo);
	if (ret < 0)
	{
		WriteLog(LogLevel::Error, "GetAddrinfo for m_ClientLocalAddressInfo Failed. ret:%d, Errno:%d", ret, WSAGetLastError());
		return false;
	}
	m_Sockets[0] = TcpUtility::CreateSocket(m_AddressInfo->ai_family);
	if (m_Sockets[0] == INVALID_SOCKET)
	{
		WriteLog(LogLevel::Error, "Create ReadSocket Failed.");
		return false;
	}
	if (!TcpUtility::Bind(m_Sockets[0], m_AddressInfo))
	{
		WriteLog(LogLevel::Error, "SocketNotify Bind Failed.");
		closesocket(m_Sockets[0]);
		return false;
	}
	if (!TcpUtility::Listen(m_Sockets[0]))
	{
		WriteLog(LogLevel::Error, "SocketNotify Listen Failed.");
		closesocket(m_Sockets[0]);
		return false;
	}
	sockaddr_in actualAddr;
	int addrLen = sizeof(actualAddr);
	if (getsockname(m_Sockets[0], (sockaddr*)&actualAddr, &addrLen) == SOCKET_ERROR)
	{
		WriteLog(LogLevel::Error, "getsockname failed: %d", WSAGetLastError());
		closesocket(m_Sockets[0]);
		return false;
	}

	m_Sockets[1] = TcpUtility::CreateSocket(m_AddressInfo->ai_family);
	if (m_Sockets[1] == INVALID_SOCKET)
	{
		WriteLog(LogLevel::Error, "Create Write Failed.");
		closesocket(m_Sockets[0]);
		return false;
	}
	ret = connect(m_Sockets[1], (sockaddr*)&actualAddr, addrLen);
	if (ret != 0)
	{
		WriteLog(LogLevel::Error, "Connect Failed. ret:%d, errorID:%d", ret, WSAGetLastError());
		closesocket(m_Sockets[0]);
		closesocket(m_Sockets[1]);
		return false;
	}
#endif
#ifdef __linux__
	auto ret = socketpair(AF_UNIX, SOCK_STREAM, 0, m_Sockets);
	if (ret != 0)
	{
		WriteLog(LogLevel::Warning, "socketpair failed. ret:%d, errorID:%d", ret, errno);
	}
#endif
	return true;
}
}

