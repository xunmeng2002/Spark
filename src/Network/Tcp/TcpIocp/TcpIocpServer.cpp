#include "Tcp/TcpIocp/TcpIocpServer.h"
#include "Tcp/TcpIocp/TcpIocpSockApi.h"
#include "Tcp/TcpIocp/TcpIocpCompletePort.h"
#include <Spark/Core/Logger/Logger.h>
#include "Tcp/TcpUtility.h"

#ifdef _WIN32
using namespace Spark::Core;

namespace Spark::Network
{
TcpIocpServer::TcpIocpServer(const char* addressName, int milliSeconds, int backlog)
    : TcpIocpBase(ServerTypeType::Server, addressName, milliSeconds, backlog)
{
}
bool TcpIocpServer::Init()
{
    if (!TcpIocpBase::Init())
        return false;
    if (!TcpUtility::Bind(socket_, addressInfo_))
    {
        return false;
    }
    if (!TcpUtility::Listen(socket_, backLog_))
    {
        return false;
    }
    for (auto i = 0; i < backLog_; i++)
    {
        if (!PostAccept())
        {
            WriteLog(LogLevel::Error, "PostAccept Failed.");
            return false;
        }
    }
    return true;
}
bool TcpIocpServer::PostAccept()
{
    SOCKET socketId = PrepareAcceptSocket();
    if (socketId == INVALID_SOCKET)
    {
        WriteLog(LogLevel::Error, "PrepareAcceptSocket SOCKET Failed.");
        return false;
    }
    TcpIocpConnect* tcpIocpConnect = TcpIocpConnect::Allocate(GetSessionId(), socketId, "", "");
    MyOverlapped* overlapped = MyOverlapped::Allocate();
    overlapped->SetBuffer(LinearBuffer<BufferSize>::Allocate());
    overlapped->EventId = IocpEvent::EventAccept;
    overlapped->Connect = tcpIocpConnect;

    WriteLog(LogLevel::Info, "PostAccept SessionId:%lld, Socket:%lld", tcpIocpConnect->SessionId, tcpIocpConnect->SocketId);
    DWORD transBytes = 0;
    auto ret = SocketApi::GetInstance().AcceptEx(socket_, tcpIocpConnect->SocketId, overlapped->WsaBuffer.buf, 0, (sizeof(SOCKADDR_IN) + 16),
                                                 (sizeof(SOCKADDR_IN) + 16), &transBytes, overlapped);
    auto lastError = WSAGetLastError();
    if (!ret && lastError != ERROR_IO_PENDING)
    {
        WriteLog(LogLevel::Error, "Call AcceptEx Failed. SessionId:%lld, Socket:%lld, Errno:%d", tcpIocpConnect->SessionId, tcpIocpConnect->SocketId,
                 lastError);
        ReleaseUnsubmittedIoRequest(tcpIocpConnect, overlapped);
        return false;
    }
    RegisterInFlightConnectRequest(overlapped);
    return true;
}
void TcpIocpServer::OnAcceptComplete(MyOverlapped* overlapped)
{
    auto tcpIocpConnect = static_cast<TcpIocpConnect*>(overlapped->Connect);
    UnregisterInFlightConnectRequest(overlapped);
    SOCKADDR_IN* remoteAddr = NULL;
    SOCKADDR_IN* localAddr = NULL;
    int remoteLen = sizeof(SOCKADDR_IN), localLen = sizeof(SOCKADDR_IN);
    SocketApi::GetInstance().GetAcceptExSockAddrs(overlapped->WsaBuffer.buf, 0, (sizeof(SOCKADDR_IN) + 16), (sizeof(SOCKADDR_IN) + 16),
                                                  reinterpret_cast<LPSOCKADDR*>(&localAddr), &localLen, reinterpret_cast<LPSOCKADDR*>(&remoteAddr),
                                                  &remoteLen);
    inet_ntop(AF_INET, &remoteAddr->sin_addr, tcpIocpConnect->RemoteAddress, sizeof(tcpIocpConnect->RemoteAddress));
    tcpIocpConnect->RemotePort = ntohs(remoteAddr->sin_port);

    WriteLog(LogLevel::Info, "AcceptComplete: From <%s:%d>, SessionId:%lld, Socket:%lld", tcpIocpConnect->RemoteAddress, tcpIocpConnect->RemotePort,
             tcpIocpConnect->SessionId, tcpIocpConnect->SocketId);

    AddConnect(tcpIocpConnect);
    PostRecv(overlapped);
    auto overlapped2 = MyOverlapped::Allocate();
    overlapped2->SetBuffer(LinearBuffer<BufferSize>::Allocate());
    overlapped2->Connect = tcpIocpConnect;
    PostRecv(overlapped2);
    PostAccept();
}

SOCKET TcpIocpServer::PrepareAcceptSocket()
{
    SOCKET socketId = WSASocket(addressInfo_->ai_family, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
    if (socketId == INVALID_SOCKET)
    {
        WriteLog(LogLevel::Error, "Create SOCKET Failed.");
        return INVALID_SOCKET;
    }
    if (!ioCompletePort_->AssociateDevice(reinterpret_cast<HANDLE>(socketId), socketId))
    {
        WriteLog(LogLevel::Warning, "Associate CompletionPort Failed, Socket:%lld", socketId);
        closesocket(socketId);
        return INVALID_SOCKET;
    }
    return socketId;
}
}
#endif // _WIN32
