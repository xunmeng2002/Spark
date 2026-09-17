#include "Tcp/SocketNotify.h"
#include "Tcp/TcpUtility.h"
#include "Tcp/TcpConnect.h"
#include <Spark/Core/Logger/Logger.h>

using namespace Spark::Core;

namespace Spark::Network
{
SocketNotify::SocketNotify()
    :sockets_{ INVALID_SOCKET, INVALID_SOCKET }, tcpConnect_(nullptr), ip_("127.0.0.1"), addressInfo_(nullptr), receiveBuffer_{0}
{
}
SocketNotify::~SocketNotify()
{
    if (sockets_[0] != -1)
    {
        closesocket(sockets_[0]);
    }
    if (sockets_[1] != -1)
    {
        closesocket(sockets_[1]);
    }
}
bool SocketNotify::Init()
{
    if (!CreateSocketPair())
        return false;
    TcpUtility::SetSockUnblock(sockets_[0]);
    TcpUtility::SetSockUnblock(sockets_[1]);

    tcpConnect_ = TcpConnect::Allocate(0LL, sockets_[0], ip_, "");
    return true;
}
bool SocketNotify::Notify()
{
    char buffer = 1;
    int len = send(sockets_[1], &buffer, 1, 0);
    return len == 1;
}
bool SocketNotify::Consume()
{
    int len = recv(sockets_[0], receiveBuffer_, sizeof(receiveBuffer_), 0);
    return len > 0;
}
SOCKET SocketNotify::GetReadSocket()
{
    return sockets_[0];
}
TcpConnect* SocketNotify::GetConnect()
{
    return tcpConnect_;
}


bool SocketNotify::CreateSocketPair()
{
#ifdef _WIN32
    auto ret = TcpUtility::GetAddrinfo(ip_.c_str(), 0, addressInfo_);
    if (ret < 0)
    {
        WriteLog(LogLevel::Error, "GetAddrinfo for clientLocalAddressInfo_ Failed. ret:%d, Errno:%d", ret, WSAGetLastError());
        return false;
    }
    sockets_[0] = TcpUtility::CreateSocket(addressInfo_->ai_family);
    if (sockets_[0] == INVALID_SOCKET)
    {
        WriteLog(LogLevel::Error, "Create ReadSocket Failed.");
        return false;
    }
    if (!TcpUtility::Bind(sockets_[0], addressInfo_))
    {
        WriteLog(LogLevel::Error, "SocketNotify Bind Failed.");
        closesocket(sockets_[0]);
        return false;
    }
    if (!TcpUtility::Listen(sockets_[0]))
    {
        WriteLog(LogLevel::Error, "SocketNotify Listen Failed.");
        closesocket(sockets_[0]);
        return false;
    }
    sockaddr_in actualAddr;
    int addrLen = sizeof(actualAddr);
    if (getsockname(sockets_[0], (sockaddr*)&actualAddr, &addrLen) == SOCKET_ERROR)
    {
        WriteLog(LogLevel::Error, "getsockname failed: %d", WSAGetLastError());
        closesocket(sockets_[0]);
        return false;
    }

    sockets_[1] = TcpUtility::CreateSocket(addressInfo_->ai_family);
    if (sockets_[1] == INVALID_SOCKET)
    {
        WriteLog(LogLevel::Error, "Create Write Failed.");
        closesocket(sockets_[0]);
        return false;
    }
    ret = connect(sockets_[1], (sockaddr*)&actualAddr, addrLen);
    if (ret != 0)
    {
        WriteLog(LogLevel::Error, "Connect Failed. ret:%d, errorId:%d", ret, WSAGetLastError());
        closesocket(sockets_[0]);
        closesocket(sockets_[1]);
        return false;
    }
#endif
#ifdef __linux__
    auto ret = socketpair(AF_UNIX, SOCK_STREAM, 0, sockets_);
    if (ret != 0)
    {
        WriteLog(LogLevel::Warning, "socketpair failed. ret:%d, errorId:%d", ret, errno);
    }
#endif
    return true;
}
}

