#include "Tcp/TcpEpoll/TcpEpollClient.h"
#include <Spark/Core/Logger/Logger.h>
#include "Tcp/TcpUtility.h"

using namespace Spark::Core;

namespace Spark::Network
{
TcpEpollClient::TcpEpollClient(const char* addressName, int milliSeconds) : TcpEpollBase(ServerTypeType::Client, addressName, milliSeconds) {}
TcpEpollClient::~TcpEpollClient() {}

bool TcpEpollClient::ConnectToServer(const char* ip, unsigned short port)
{
    if (addressInfo_ != nullptr)
    {
        freeaddrinfo(addressInfo_);
        addressInfo_ = nullptr;
    }
    address_ = ip;
    port_ = std::to_string(port);
    auto ret = TcpUtility::GetAddrinfo(address_.c_str(), port_.c_str(), addressInfo_);
    if (ret < 0 || addressInfo_ == nullptr)
    {
        WriteLog(LogLevel::Info, "GetAddrinfo Failed. Address:%s Port:%s ret:%d, Errno:%d", address_.c_str(), port_.c_str(), ret, errno);
        return false;
    }
    auto socketId = TcpUtility::PrepareSocket(addressInfo_->ai_family);
    ret = connect(socketId, addressInfo_->ai_addr, int(addressInfo_->ai_addrlen));
    if (ret == -1 && errno != EINPROGRESS)
    {
        WriteLog(LogLevel::Warning, "ConnectToServer Failed. IP:%s, Port:%d, errno:%d", ip, port, errno);
        closesocket(socketId);
        return false;
    }
    TcpConnect* tcpConnect = TcpConnect::Allocate(GetSessionId(), socketId, address_, port_);
    if (ret == 0)
    {
        if (!AddConnect(tcpConnect))
        {
            DiscardRefusedConnect(tcpConnect);
            return false;
        }
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
    epoll_ctl(epollFd_, EPOLL_CTL_ADD, connect->SocketId, &epollEvent);
#endif
}
}
