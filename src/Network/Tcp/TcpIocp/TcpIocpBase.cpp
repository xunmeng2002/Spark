#include "Tcp/TcpIocp/TcpIocpBase.h"
#include "Tcp/TcpIocp/TcpIocpConnect.h"
#include "Tcp/TcpIocp/TcpIocpSockApi.h"
#include "Tcp/TcpIocp/TcpIocpCompletePort.h"
#include "Tcp/TcpUtility.h"
#include <Spark/Core/Logger/Logger.h>

#ifdef _WIN32

using namespace std;
using namespace Spark::Core;

namespace Spark::Network
{
TcpIocpBase::TcpIocpBase(ServerTypeType serverType, const char* addressName, int milliSeconds, int backlog)
    : TcpBase(serverType, addressName, milliSeconds), backLog_(backlog)
{
    ioCompletePort_ = new TcpIocpCompletePort();
}
TcpIocpBase::~TcpIocpBase()
{
    ioCompletePort_->PostStatus(0, 0, NULL);
    delete ioCompletePort_;
}

bool TcpIocpBase::Init()
{
    auto ret = TcpUtility::GetAddrinfo(address_.c_str(), port_.c_str(), addressInfo_);
    if (ret != 0)
    {
        WriteLog(LogLevel::Info, "GetAddrinfo Failed. Address:%s, Port%s, ret:%d", address_.c_str(), port_.c_str(), ret);
        return false;
    }
    socket_ = WSASocket(addressInfo_->ai_family, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
    if (socket_ == INVALID_SOCKET)
    {
        WriteLog(LogLevel::Error, "Create SOCKET Failed.");
        return false;
    }
    int on = 1;
    if (setsockopt(socket_, SOL_SOCKET, SO_REUSEADDR, (const char*)&on, sizeof(on)) != 0)
    {
        WriteErrorLog(WSAGetLastError(), "setsockopt Failed. ErrorId:%d, result:%d");
        return false;
    }
    if (!SocketApi::GetInstance().Init(socket_))
    {
        return false;
    }
    if (!ioCompletePort_->Create())
    {
        WriteErrorLog(LogLevel::Error, "Create TcpIocpCompletePort Failed.");
        return false;
    }
    if (!ioCompletePort_->AssociateDevice((HANDLE)socket_, socket_))
    {
        WriteErrorLog(WSAGetLastError(), "AssociateDevice Failed.");
        return false;
    }

    return true;
}
void TcpIocpBase::Send(SessionIdType sessionId, LinearBuffer<BufferSize>* buffer)
{
    if (buffer->GetLength() == 0)
    {
        WriteLog(LogLevel::Error, "Send BufferLen is 0");
        buffer->Deallocate();
        return;
    }
    auto connect = (TcpIocpConnect*)GetConnect(sessionId);
    if (connect == nullptr)
    {
        WriteLog(LogLevel::Warning, "Send Connect Not Exist, Drop Buffer. SessionId:%lld, Len:%zu", sessionId, buffer->GetLength());
        buffer->Deallocate();
        return;
    }
    {
        lock_guard<mutex> guard(connect->BuffersMutex);
        if (connect->HasPendingSend || !connect->Buffers.empty())
        {
            connect->Buffers.push_back(buffer);
            return;
        }
    }
    connect->HasPendingSend = true;
    MyOverlapped* overlapped = MyOverlapped::Allocate();
    overlapped->SetBuffer(buffer);
    overlapped->Connect = connect;
    PostSend(overlapped);
}
void TcpIocpBase::HandleTcpEvent()
{
    DWORD len;
    ULONG_PTR competionKey;
    MyOverlapped* overlapped;

    auto bOK = ioCompletePort_->GetStatus(&len, &competionKey, (LPOVERLAPPED*)&overlapped, (DWORD)timeOut_.count());
    WriteLog(LogLevel::Debug, "CompletionKey:%d, Len:%d, Ret:%d.", competionKey, len, bOK);
    if (!bOK)
    {
        auto errorId = WSAGetLastError();
        if (errorId == WAIT_TIMEOUT)
        {
            return;
        }
        else if (errorId == ERROR_OPERATION_ABORTED)
        {
            WriteLog(LogLevel::Info, "GetStatus Failed For CancelIoEx. errorId:%d, SessionId:%lld, CompetionKey:%d.", errorId,
                     overlapped->Connect->SessionId, competionKey);
            overlapped->Deallocate();
            return;
        }
        else
        {
            if (overlapped != nullptr)
            {
                WriteLog(LogLevel::Info, "GetStatus Failed. ErrorId:%d, SessionId:%lld, CompetionKey:%d.", errorId, overlapped->Connect->SessionId,
                         competionKey);
                PostDisConnect(overlapped);
                return;
            }
            else
            {
                WriteLog(LogLevel::Warning, "GetStatus Failed. Overlapped is NULL. Errno:%d", errorId);
                return;
            }
        }
    }
    if (overlapped == nullptr)
    {
        WriteLog(LogLevel::Error, "CompetionKey:%d, OVERLAPPED is null.", competionKey);
        return;
    }
    auto tcpConnect = overlapped->Connect;

    if (len == 0 && (overlapped->EventId == IocpEvent::EventSend || overlapped->EventId == IocpEvent::EventRecv))
    {
        WriteLog(LogLevel::Warning, "CompetionKey:%d, Len is 0, EventId:%d overlapped:%p, overlapped->MyBuffer:%p, BufferLen:%zu", competionKey,
                 overlapped->EventId, overlapped, overlapped->MyBuffer, overlapped->MyBuffer->GetLength());
        PostDisConnect(overlapped);
        return;
    }

    switch (overlapped->EventId)
    {
    case IocpEvent::EventAccept:
        OnAcceptComplete(overlapped);
        break;
    case IocpEvent::EventConnect:
        OnConnectComplete(overlapped);
        break;
    case IocpEvent::EventDisConnect:
        OnDisConnectComplete(overlapped);
        break;
    case IocpEvent::EventSend:
        OnSendComplete(overlapped, len);
        break;
    case IocpEvent::EventRecv:
        OnRecvComplete(overlapped, len);
        break;
    default:
        WriteLog(LogLevel::Error, "INVALID EventId:%d, SessionId:%lld, Socket:%lld.", overlapped->EventId, tcpConnect->SessionId,
                 tcpConnect->SocketId);
        break;
    }
}
void TcpIocpBase::DoDisConnect()
{
    lock_guard<mutex> guard(disConnectSessionIdsMutex_);
    for (auto sessionId : disConnectSessionIds_)
    {
        auto connect = (TcpConnect*)connects_[sessionId];
        if (connect != nullptr)
            PostDisConnect(connect);
    }
    disConnectSessionIds_.clear();
}

bool TcpIocpBase::PostDisConnect(Connect* connect)
{
    auto tcpIocpConnect = (TcpIocpConnect*)connect;
    MyOverlapped* overlapped = MyOverlapped::Allocate();
    overlapped->SetBuffer(LinearBuffer<BufferSize>::Allocate());
    overlapped->EventId = IocpEvent::EventDisConnect;
    overlapped->Connect = tcpIocpConnect;

    return PostDisConnect(overlapped);
}
bool TcpIocpBase::PostDisConnect(MyOverlapped* overlapped)
{
    overlapped->EventId = IocpEvent::EventDisConnect;

    WriteLog(LogLevel::Info, "PostDisConnect SessionId:%lld, Socket:%lld", overlapped->Connect->SessionId, overlapped->Connect->SocketId);
    DWORD transBytes = 0, flag = 0;
    CancelIoEx((HANDLE)overlapped->Connect->SocketId, NULL);
    auto ret = SocketApi::GetInstance().DisconnectEx(overlapped->Connect->SocketId, overlapped, TF_REUSE_SOCKET, 0);
    auto lastError = WSAGetLastError();
    if (ret != 0 && lastError != ERROR_IO_PENDING)
    {
        WriteLog(LogLevel::Error, "Call DisConnectEx Failed. SessionId:%lld, Socket:%lld, Errno:%d", overlapped->Connect->SessionId,
                 overlapped->Connect->SocketId, lastError);
        OnDisConnectComplete(overlapped);
        return false;
    }
    return true;
}
bool TcpIocpBase::PostSend(MyOverlapped* overlapped)
{
    overlapped->EventId = IocpEvent::EventSend;

    WriteLog(LogLevel::Debug, "PostSend SessionId:%lld, Socket:%lld, overlapped:%p, overlapped->MyBuffer:%p, BufferLen:%zu",
             overlapped->Connect->SessionId, overlapped->Connect->SocketId, overlapped, overlapped->MyBuffer, overlapped->MyBuffer->GetLength());
    if (overlapped->MyBuffer->GetLength() == 0)
    {
        WriteLog(LogLevel::Warning, "PostSend BufferLen is 0. SessionId:%lld, Socket:%lld, overlapped:%p, overlapped->MyBuffer:%p, BufferLen:%zu",
                 overlapped->Connect->SessionId, overlapped->Connect->SocketId, overlapped, overlapped->MyBuffer, overlapped->MyBuffer->GetLength());
    }
    DWORD transBytes = 0, flag = 0;
    auto ret = WSASend(overlapped->Connect->SocketId, &overlapped->WsaBuffer, 1, &transBytes, flag, overlapped, NULL);
    if (ret == SOCKET_ERROR)
    {
        auto lastError = WSAGetLastError();
        if (lastError != ERROR_IO_PENDING)
        {
            WriteLog(LogLevel::Error, "PostSend: WSASend failed. SessionId:%lld, Socket:%lld, Errno:%d", overlapped->Connect->SessionId,
                     overlapped->Connect->SocketId, lastError);
            PostDisConnect(overlapped);
            return false;
        }
    }
    return true;
}
bool TcpIocpBase::PostRecv(MyOverlapped* overlapped)
{
    auto tcpIocpConnect = (TcpIocpConnect*)overlapped->Connect;
    overlapped->Reset();
    overlapped->EventId = IocpEvent::EventRecv;
    overlapped->Connect = tcpIocpConnect;

    WriteLog(LogLevel::Debug, "PostRecv SessionId:%lld, Socket:%lld, overlapped:%p, overlapped->MyBuffer:%p, BufferLen:%zu", tcpIocpConnect->SessionId,
             tcpIocpConnect->SocketId, overlapped, overlapped->MyBuffer, overlapped->MyBuffer->GetLength());
    DWORD transBytes = 0, flag = 0;
    auto ret = WSARecv(tcpIocpConnect->SocketId, &overlapped->WsaBuffer, 1, nullptr, &flag, overlapped, NULL);
    if (ret == SOCKET_ERROR)
    {
        auto lastError = WSAGetLastError();
        if (lastError != ERROR_IO_PENDING)
        {
            WriteLog(LogLevel::Error, "PostRecv: WSARecv failed. SessionId:%lld, Socket:%lld, Errno:%d", tcpIocpConnect->SessionId,
                     tcpIocpConnect->SocketId, lastError);
            PostDisConnect(overlapped);
            return false;
        }
    }
    return true;
}

void TcpIocpBase::OnDisConnectComplete(MyOverlapped* overlapped)
{
    WriteLog(LogLevel::Info, "OnDisConnectComplete SessionId:%lld, Socket:%lld", overlapped->Connect->SessionId, overlapped->Connect->SocketId);
    RemoveConnect(overlapped->Connect);
    overlapped->Deallocate();
}
void TcpIocpBase::OnSendComplete(MyOverlapped* overlapped, int bytesTransferred)
{
    WriteLog(LogLevel::Debug, "OnSendComplete SessionId:%lld, Socket:%lld, bytesTransferred:%d, overlapped:%p, overlapped->MyBuffer:%p, BufferLen:%zu",
             overlapped->Connect->SessionId, overlapped->Connect->SocketId, bytesTransferred, overlapped, overlapped->MyBuffer,
             overlapped->MyBuffer->GetLength());
    if (static_cast<size_t>(bytesTransferred) < overlapped->MyBuffer->GetLength())
    {
        WriteLog(LogLevel::Warning,
                 "OnSendComplete PartSended. PostSend Again. BufferLen:%zu, bytesTransferred:%d, overlapped:%p, overlapped->MyBuffer:%p",
                 overlapped->MyBuffer->GetLength(), bytesTransferred, overlapped, overlapped->MyBuffer);
        overlapped->Shift(bytesTransferred);
        PostSend(overlapped);
    }
    else
    {
        LinearBuffer<BufferSize>* buffer;
        {
            lock_guard<mutex> guard(overlapped->Connect->BuffersMutex);
            if (overlapped->Connect->Buffers.empty())
            {
                overlapped->Connect->HasPendingSend = false;
                overlapped->Deallocate();
                return;
            }
            else
            {
                buffer = overlapped->Connect->Buffers.front();
                overlapped->Connect->Buffers.pop_front();
            }
        }
        overlapped->SetBuffer(buffer);
        PostSend(overlapped);
    }
}
void TcpIocpBase::OnRecvComplete(MyOverlapped* overlapped, int bytesTransferred)
{
    WriteLog(LogLevel::Debug, "OnRecvComplete SessionId:%lld, Socket:%lld, bytesTransferred:%d, overlapped:%p, overlapped->MyBuffer:%p, BufferLen:%zu",
             overlapped->Connect->SessionId, overlapped->Connect->SocketId, bytesTransferred, overlapped, overlapped->MyBuffer,
             overlapped->MyBuffer->GetLength());
    overlapped->MyBuffer->SetLength(static_cast<size_t>(bytesTransferred));
    auto tcpConnect = (TcpConnect*)overlapped->Connect;
    if (ioSubscriber_)
    {
        ioSubscriber_->OnRecv(tcpConnect->SessionId, overlapped->MyBuffer);
    }
    PostRecv(overlapped);
}
}
#endif // _WIN32
