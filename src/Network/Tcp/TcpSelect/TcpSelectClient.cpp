#include "Tcp/TcpSelect/TcpSelectClient.h"
#include "Tcp/TcpUtility.h"
#include <Spark/Core/Logger/Logger.h>
#include <cstring>

using namespace std;
using namespace Spark::Core;

namespace Spark::Network
{
TcpSelectClient::TcpSelectClient(const char* addressName, int milliSeconds) : TcpSelectBase(ServerTypeType::Client, addressName, milliSeconds) {}

bool TcpSelectClient::ConnectToServer(const char* ip, unsigned short port)
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
#ifdef _WIN32
    auto error = WSAGetLastError();
    if (ret == -1 && error != WSAEWOULDBLOCK && error != WSAEINPROGRESS)
    {
        WriteLog(LogLevel::Warning, "ConnectToServer Failed. ip:%s, port:%d, errno:%d", ip, port, errno);
        closesocket(socketId);
        return false;
    }
#endif
#ifdef __linux__
    if (ret == -1 && errno != EINPROGRESS)
    {
        WriteLog(LogLevel::Warning, "ConnectToServer Failed. ip:%s, port:%d, errno:%d", ip, port, errno);
        closesocket(socketId);
        return false;
    }
#endif
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
        connectings_.insert(make_pair(tcpConnect->SessionId, tcpConnect));
    }
    return true;
}
void TcpSelectClient::CheckConnect()
{
    if (connectings_.empty())
        return;
    FD_ZERO(&writeFds_);
    FD_ZERO(&errorFds_);
    maxId_ = 0;
    for (auto& it : connectings_)
    {
        auto connect = it.second;
        FD_SET(connect->SocketId, &writeFds_);
        FD_SET(connect->SocketId, &errorFds_);
        if (connect->SocketId > maxId_)
        {
            maxId_ = connect->SocketId;
        }
    }
    ++maxId_;

    memcpy(&selectSocketTimeOutTemp_, &selectSocketTimeOut_, sizeof(timeval));
    ::select(static_cast<int>(maxId_), nullptr, &writeFds_, &errorFds_, &selectSocketTimeOutTemp_);
    for (auto& it : connectings_)
    {
        auto connect = static_cast<TcpConnect*>(it.second);
        if (FD_ISSET(connect->SocketId, &writeFds_))
        {
            if (!AddConnect(connect))
            {
                // 会话号先取出再归还：归还即析构，之后不得再读 connect 的成员。
                SessionIdType refusedSessionId = connect->SessionId;
                DiscardRefusedConnect(connect);
                connectFailedSessions_.push_back(refusedSessionId);
                // 必须跳过本轮的 error 分支：它会读 connect->SocketId，而对象已归还。
                continue;
            }
            connectSuccessedSessions_.push_back(connect->SessionId);
        }
        if (FD_ISSET(connect->SocketId, &errorFds_))
        {
            RemoveConnect(connect);
            connectFailedSessions_.push_back(connect->SessionId);
        }
    }
    for (auto& sessionId : connectSuccessedSessions_)
    {
        connectings_.erase(sessionId);
    }
    connectSuccessedSessions_.clear();
    for (auto& sessionId : connectFailedSessions_)
    {
        connectings_.erase(sessionId);
    }
    connectFailedSessions_.clear();
}
}
