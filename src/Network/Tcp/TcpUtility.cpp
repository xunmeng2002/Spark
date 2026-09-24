#include "Tcp/TcpUtility.h"
#include <Spark/Core/Platform/Platform.h>
#include <Spark/Core/Logger/Logger.h>
#include <string.h>
#include <mutex>

using namespace std;
using namespace Spark::Core;

namespace Spark::Network
{
void TcpUtility::ParseIPAddress(const std::string& addressName, std::string& ip, std::string& port)
{
    auto index = addressName.find(':', 0);
    ip = addressName.substr(0, index);
    port = addressName.substr(index + 1);
}
int TcpUtility::GetAddrinfo(const char* ip, const char* port, addrinfo*& addrInfo)
{
    struct addrinfo hints;
    ::memset(&hints, 0, sizeof(hints));
    hints.ai_flags = AI_PASSIVE;
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    auto ret = getaddrinfo(ip, port, &hints, &addrInfo);
    WriteLog(LogLevel::Info, "GetAddrinfo: IP:%s Port:%s GetAddrinfo ret:%d", ip, port, ret);
    return ret;
}
int TcpUtility::GetClientAddrinfo(const char* ip, const char* port, addrinfo*& addrInfo, int serverAddressFamily)
{
    struct addrinfo hints;
    ::memset(&hints, 0, sizeof(hints));
    hints.ai_flags = 0;
    hints.ai_family = serverAddressFamily;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    auto ret = getaddrinfo(ip, port, &hints, &addrInfo);
    WriteLog(LogLevel::Info, "GetClientAddrinfo: IP:%s Port:%s GetAddrinfo ret:%d", ip, port, ret);
    return ret;
}
int TcpUtility::GetNameinfo(const sockaddr* sockAddr, int len, std::string& ip, std::string& port, int flags)
{
    static mutex getnameinfoMutex;
    lock_guard<mutex> guard(getnameinfoMutex);
    char ipBuff[NI_MAXHOST]{0};
    char portBuff[NI_MAXSERV]{0};
    auto ret = getnameinfo(sockAddr, len, ipBuff, NI_MAXHOST, portBuff, NI_MAXSERV, NI_NUMERICHOST);
    ip = ipBuff;
    port = portBuff;
    return ret;
}

SOCKET TcpUtility::CreateSocket(int family)
{
    return socket(family, SOCK_STREAM, IPPROTO_TCP);
}
bool TcpUtility::SetSockUnblock(SOCKET socketId, unsigned long unblock)
{
#ifdef _WIN32
    if (::ioctlsocket(socketId, FIONBIO, &unblock) == SOCKET_ERROR)
    {
        WriteLog(LogLevel::Error, "ioctlsocket FIONBIO:%d, Failed. ErrorId:%d", unblock, WSAGetLastError());
        return false;
    }
    WriteLog(LogLevel::Info, "ioctlsocket FIONBIO:%d Success.", unblock);
#endif //  _WIN32
#ifdef __linux__
    if (ioctl(socketId, FIONBIO, &unblock) == SOCKET_ERROR)
    {
        WriteLog(LogLevel::Error, "ioctl FIONBIO:%d Failed. ErrorId:%d", unblock, WSAGetLastError());
        return false;
    }
    WriteLog(LogLevel::Info, "ioctl FIONBIO:%d Success.", unblock);
#endif // __linux__
    return true;
}
bool TcpUtility::SetSockReuse(SOCKET socketId, int resue)
{
    if (::setsockopt(socketId, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<char*>(&resue), sizeof(int)) == SOCKET_ERROR)
    {
        WriteLog(LogLevel::Error, "setsockopt SO_REUSEADDR:%d Failed. ErrorId:%d", resue, WSAGetLastError());
        return false;
    }
    WriteLog(LogLevel::Info, "setsockopt SO_REUSEADDR:%d Success.", resue);
    return true;
}
bool TcpUtility::SetSockNodelay(SOCKET socketId, int nodelay)
{
    if (::setsockopt(socketId, IPPROTO_TCP, TCP_NODELAY, reinterpret_cast<char*>(&nodelay), sizeof(int)) == SOCKET_ERROR)
    {
        WriteLog(LogLevel::Error, "setsockopt TCP_NODELAY:%d Failed. ErrorId:%d", nodelay, WSAGetLastError());
        return false;
    }
    WriteLog(LogLevel::Info, "setsockopt TCP_NODELAY:%d Success.", nodelay);
    return true;
}
bool TcpUtility::SetSockIPV6Only(SOCKET socketId, int ipv6Only)
{
    if (::setsockopt(socketId, IPPROTO_IPV6, IPV6_V6ONLY, reinterpret_cast<char*>(&ipv6Only), sizeof(int)) == SOCKET_ERROR)
    {
        WriteLog(LogLevel::Error, "setsockopt IPV6_V6ONLY:%d Failed. ErrorId:%d", ipv6Only, WSAGetLastError());
        return false;
    }
    WriteLog(LogLevel::Error, "setsockopt IPV6_V6ONLY:%d Success.", ipv6Only);
    return true;
}
bool TcpUtility::Bind(SOCKET socketId, addrinfo* bindAddressInfo)
{
    if (::bind(socketId, bindAddressInfo->ai_addr, int(bindAddressInfo->ai_addrlen)) == SOCKET_ERROR)
    {
        WriteLog(LogLevel::Error, "Bind Failed. ErrorId:%d", WSAGetLastError());
        return false;
    }
    return true;
}
bool TcpUtility::Listen(SOCKET socketId, int backLog)
{
    if (listen(socketId, backLog) == SOCKET_ERROR)
    {
        WriteLog(LogLevel::Error, "Listen Failed. ErrorId:%d", WSAGetLastError());
        return false;
    }
    return true;
}

bool TcpUtility::InitSocket(SOCKET socketId)
{
    if (!SetSockUnblock(socketId) || !SetSockReuse(socketId) || !SetSockNodelay(socketId))
    {
        WriteLog(LogLevel::Warning, "InitSocket Failed. ErrorId:%d", WSAGetLastError());
        return false;
    }
    return true;
}
SOCKET TcpUtility::PrepareSocket(int family)
{
    auto socketId = CreateSocket(family);
    if (!InitSocket(socketId))
    {
        closesocket(socketId);
        return INVALID_SOCKET;
    }
    return socketId;
}
}
