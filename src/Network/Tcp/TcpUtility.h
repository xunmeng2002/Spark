#pragma once
#include <Spark/Network/NetworkExport.h>
#include "Tcp/SocketInit.h"
#include <string>

namespace Spark::Network
{
class NETWORK_EXPORTS TcpUtility
{
public:
    static void ParseIPAddress(const std::string& addressName, std::string& ip, std::string& port);
    static int GetAddrinfo(const char* ip, const char* port, addrinfo*& addrInfo);
    static int GetClientAddrinfo(const char* ip, const char* port, addrinfo*& addrInfo, int serverAddressFamily);
    static int GetNameinfo(const sockaddr* sockAddr, int len, std::string& ip, std::string& port, int flags = NI_NUMERICHOST);

    static SOCKET CreateSocket(int family);
    static bool SetSockUnblock(SOCKET socketId, unsigned long unblock = 1);
    static bool SetSockReuse(SOCKET socketId, int resue = 1);
    static bool SetSockNodelay(SOCKET socketId, int nodelay = 1);
    static bool SetSockIPV6Only(SOCKET socketId, int ipv6Only = 0);
    static bool Bind(SOCKET socketId, addrinfo* bindAddressInfo);
    static bool Listen(SOCKET socketId, int backLog = 5);

    static bool InitSocket(SOCKET socketId);
    static SOCKET PrepareSocket(int family);
};
}
