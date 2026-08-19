#pragma once
#include <Spark/Network/NetworkExport.h>
#include "Tcp/SocketInit.h"
#include <string>


namespace spark::network
{
class NETWORK_EXPORTS TcpUtility
{
public:
	static void ParseIPAddress(const std::string& addressName, std::string& ip, std::string& port);
	static int GetAddrinfo(const char* ip, const char* port, addrinfo*& addrInfo);
	static int GetClientAddrinfo(const char* ip, const char* port, addrinfo*& addrInfo, int serverAddressFamily);
	static int GetNameinfo(const sockaddr* sockAddr, int len, std::string& ip, std::string& port, int flags = NI_NUMERICHOST);

	static SOCKET CreateSocket(int family);
	static bool SetSockUnblock(SOCKET socketID, unsigned long unblock = 1);
	static bool SetSockReuse(SOCKET socketID, int resue = 1);
	static bool SetSockNodelay(SOCKET socketID, int nodelay = 1);
	static bool SetSockIPV6Only(SOCKET socketID, int ipv6Only = 0);
	static bool Bind(SOCKET socketID, addrinfo* bindAddressInfo);
	static bool Listen(SOCKET socketID, int backLog = 5);

	static bool InitSocket(SOCKET socketID);
	static SOCKET PrepareSocket(int family);
};
}
