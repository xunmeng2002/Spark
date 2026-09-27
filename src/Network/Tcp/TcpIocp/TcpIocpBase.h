#pragma once
#ifdef _WIN32
#include "Tcp/TcpBase.h"
#include "Tcp/TcpIocp/TcpIocpConnect.h"
#include "Tcp/TcpIocp/TcpIocpCompletePort.h"
#include <chrono>
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
    // 建立类请求（AcceptEx/ConnectEx）在完成包取回之前不得归还：内核收尾该 IRP 时会写回这块
    // OVERLAPPED 的内存，若池已把它发了出去，等于写坏别人的对象。故提交成功后登记、完成时注销，
    // 析构时把仍登记着的取消并取回。登记表只为析构路径与取消路径服务，不在收发热路径上。
    void RegisterInFlightConnectRequest(MyOverlapped* overlapped);
    void UnregisterInFlightConnectRequest(MyOverlapped* overlapped);
    void ReleaseAllInFlightConnectRequests();

    virtual void OnAcceptComplete(MyOverlapped* overlapped) {}
    virtual void OnConnectComplete(MyOverlapped* overlapped) {}
    virtual void OnDisConnectComplete(MyOverlapped* overlapped);
    virtual void OnSendComplete(MyOverlapped* overlapped, int bytesTransferred);
    virtual void OnRecvComplete(MyOverlapped* overlapped, int bytesTransferred);

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
