#pragma once
#include <Spark/Network/NetworkExport.h>
#include "Tcp/SocketInit.h"
#include "Tcp/TcpConnect.h"
#include "Tcp/SocketNotify.h"
#include <Spark/Network/Io/IoBase.h>
#include <Spark/TemplateLib/Buffer/LinearBuffer.h>
#include <list>
#include <map>
#include <mutex>
#include <string>

namespace Spark::Network
{
class NETWORK_EXPORTS TcpBase : public IoBase
{
public:
    TcpBase(ServerTypeType serverType, const char* addressName, int milliSeconds);
    virtual ~TcpBase();

    virtual bool Init() override;

    virtual void Send(SessionIdType sessionId, LinearBuffer<BufferSize>* buffer) override;
    virtual bool ConnectToServer(const char* address) override;
    virtual bool ConnectToServer(const char* ip, unsigned short port) { return false; }
    virtual void HandleIoEvent() override;
    virtual bool AddConnect(Connect* connect) override;
    virtual void RemoveConnect(Connect* connect) override;

protected:
    virtual void DoSend(Connect* connect) override;
    virtual void DoRecv(Connect* connect) override;
    virtual void DoAccept();
    virtual void HandleTcpEvent() = 0;
    virtual void CheckConnect() {}

    // Client 断线自动重连:IO 循环内检测无连接且无在途连接时按固定间隔重试
    void TryAutoReconnect();

    addrinfo* addressInfo_;
    SOCKET socket_;
    SocketNotify* socketNotify_;

    // 已发起未落定的连接(首连与重连共用):AddConnect/RemoveConnect 落定时复位
    bool autoConnectPending_;
    std::chrono::steady_clock::time_point lastConnectAttemptTime_;

    std::mutex connectDataMutex_;

    sockaddr_storage remoteAddress_;
#ifdef _WIN32
    int remoteAddressLen_;
#endif
#ifdef __linux__
    unsigned int remoteAddressLen_;
#endif
};
}
