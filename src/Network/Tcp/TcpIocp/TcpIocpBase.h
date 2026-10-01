#pragma once
#ifdef _WIN32
#include "Tcp/TcpBase.h"
#include "Tcp/TcpIocp/TcpIocpConnect.h"
#include "Tcp/TcpIocp/TcpIocpCompletePort.h"
#include <chrono>
#include <cstddef>
#include <list>
#include <mutex>

namespace Spark::Network
{
class TcpIocpBase : public TcpBase
{
public:
    TcpIocpBase(ServerTypeType serverType, const char* addressName, int milliSeconds, int backlog = 5);
    ~TcpIocpBase();

    virtual bool Init() override;
    virtual void Send(SessionIdType sessionId, LinearBuffer<BufferSize>* buffer) override;

protected:
    virtual void HandleTcpEvent() override;
    virtual void DoDisConnect() override;

    virtual bool PostAccept() { return false; }
    virtual bool PostConnect() { return false; }
    virtual bool PostDisConnect(Connect* connect);
    virtual bool PostDisConnect(MyOverlapped* overlapped);
    virtual bool PostSend(MyOverlapped* overlapped);
    virtual bool PostRecv(MyOverlapped* overlapped);
    void ReleaseUnsubmittedIoRequest(TcpIocpConnect* tcpIocpConnect, MyOverlapped* overlapped);
    void RegisterInFlightConnectRequest(MyOverlapped* overlapped);
    void UnregisterInFlightConnectRequest(MyOverlapped* overlapped);
    void ReleaseAllInFlightConnectRequests();

    virtual void OnAcceptComplete(MyOverlapped* overlapped) {}
    virtual void OnConnectComplete(MyOverlapped* overlapped) {}
    virtual void OnDisConnectComplete(MyOverlapped* overlapped);
    virtual void OnSendComplete(MyOverlapped* overlapped, size_t bytesTransferred);
    virtual void OnRecvComplete(MyOverlapped* overlapped, size_t bytesTransferred);

    int backLog_;
    TcpIocpCompletePort* ioCompletePort_;

private:
    MyOverlapped* TakeOneInFlightConnectRequest();
    bool ReapCancelledConnectRequest(MyOverlapped* overlapped, std::chrono::steady_clock::time_point reapDeadline);

    std::list<MyOverlapped*> inFlightConnectRequestList_;
    std::mutex inFlightConnectRequestMutex_;
};
}
#endif // _WIN32
