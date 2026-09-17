#include "Tcp/TcpIocp/TcpIocpConnect.h"
#include <Spark/TemplateLib/ObjectPool/ObjectPool.h>
#include <Spark/Core/Logger/Logger.h>

#ifdef _WIN32
using namespace Spark::Core;

namespace Spark::Network
{
TcpIocpConnect::TcpIocpConnect(SessionIdType sessionId, const SOCKET& socketId, const std::string& remoteIP, const std::string& remotePort)
    :TcpConnect(sessionId, socketId, remoteIP, remotePort)
{
}
TcpIocpConnect::~TcpIocpConnect()
{
#ifdef _WIN32
    shutdown(SocketId, SD_BOTH);
#endif
#ifdef __linux__
    shutdown(SocketId, SHUT_RDWR);
#endif
    closesocket(SocketId);
    SocketId = INVALID_SOCKET;
}
TcpIocpConnect* TcpIocpConnect::Allocate(SessionIdType sessionId, const SOCKET& socketId, const std::string& remoteIP, const std::string& remotePort)
{
    return ObjectPool<TcpIocpConnect>::GetInstance().Allocate(sessionId, socketId, remoteIP, remotePort);
}
void TcpIocpConnect::Deallocate()
{
    WriteLog(LogLevel::Info, "TcpIocpConnect::Close SessionId:%lld, Socket:%lld", SessionId, SocketId);
    ObjectPool<TcpIocpConnect>::GetInstance().Deallocate(this);
}

MyOverlapped::MyOverlapped()
{
    Internal = InternalHigh = 0;
    Offset = OffsetHigh = 0;
    hEvent = nullptr;

    EventId = IocpEvent::EventNone;
    MyBuffer = nullptr;
    WsaBuffer.buf = nullptr;
    WsaBuffer.len = 0;
    Connect = nullptr;
}
MyOverlapped::~MyOverlapped()
{
    if (MyBuffer)
    {
        MyBuffer->Reset();
        MyBuffer->Deallocate();
        MyBuffer = nullptr;
    }
    if (Connect)
    {
        Connect = nullptr;
    }
}
MyOverlapped* MyOverlapped::Allocate()
{
    return ObjectPool<MyOverlapped>::GetInstance().Allocate();
}
void MyOverlapped::Deallocate()
{
    ObjectPool<MyOverlapped>::GetInstance().Deallocate(this);
}
void MyOverlapped::SetBuffer(Buffer<BuffSize>* buffer)
{
    if (MyBuffer != nullptr)
    {
        MyBuffer->Deallocate();
    }
    MyBuffer = buffer;
    WsaBuffer.buf = MyBuffer->GetData();
    WsaBuffer.len = MyBuffer->GetLength();
}
void MyOverlapped::Shift(unsigned int len)
{
    MyBuffer->Shift(len);
    WsaBuffer.buf = MyBuffer->GetData();
    WsaBuffer.len = MyBuffer->GetLength();
}
void MyOverlapped::Reset()
{
    Internal = InternalHigh = 0;
    Offset = OffsetHigh = 0;
    hEvent = nullptr;

    EventId = IocpEvent::EventNone;
    if (MyBuffer)
    {
        MyBuffer->Reset();
        WsaBuffer.buf = MyBuffer->GetData();
        WsaBuffer.len = BuffSize;
    }
    else
    {
        WsaBuffer.buf = nullptr;
        WsaBuffer.len = 0;
    }
    Connect = nullptr;
}
}
#endif // _WIN32
