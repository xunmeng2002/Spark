#include "Tcp/TcpIocp/TcpIocpServer.h"
#include "Tcp/TcpIocp/TcpIocpSockApi.h"
#include "Tcp/TcpIocp/TcpIocpCompletePort.h"
#include <Spark/Core/Logger/Logger.h>
#include "Tcp/TcpUtility.h"

#ifdef WINDOWS
using namespace spark::core;

namespace spark::network
{
TcpIocpServer::TcpIocpServer(const char* addressName, int milliSeconds, int backlog)
	:TcpIocpBase(ServerTypeType::Server, addressName, milliSeconds, backlog)
{

}
bool TcpIocpServer::Init()
{
    if (!TcpIocpBase::Init())
        return false;
    if (!TcpUtility::Bind(m_Socket, m_AddressInfo))
    {
        return false;
    }
    if (!TcpUtility::Listen(m_Socket, m_BackLog))
    {
        return false;
    }
    for (auto i = 0; i < m_BackLog; i++)
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
    SOCKET socketID = PrepareAcceptSocket();
    if (socketID == INVALID_SOCKET)
    {
        WriteLog(LogLevel::Error, "PrepareAcceptSocket SOCKET Failed.");
        return false;
    }
    TcpIocpConnect* tcpIocpConnect = TcpIocpConnect::Allocate(GetSessionID(), socketID, "", "");
    MyOverlapped* overlapped = MyOverlapped::Allocate();
    overlapped->SetBuffer(Buffer<BuffSize>::Allocate());
    overlapped->EventID = IocpEvent::EventAccept;
    overlapped->Connect = tcpIocpConnect;

    WriteLog(LogLevel::Info, "PostAccept SessionID:%lld, Socket:%lld", tcpIocpConnect->SessionID, tcpIocpConnect->SocketID);
    DWORD transBytes = 0;
    auto ret = SocketApi::GetInstance().AcceptEx(m_Socket, tcpIocpConnect->SocketID, overlapped->WsaBuffer.buf, 0, (sizeof(SOCKADDR_IN) + 16), (sizeof(SOCKADDR_IN) + 16), &transBytes, overlapped);
    auto lastError = WSAGetLastError();
    if (ret != 0 && lastError != ERROR_IO_PENDING)
    {
        WriteLog(LogLevel::Error, "Call AcceptEx Failed. SessionID:%lld, Socket:%lld, Errno:%d", tcpIocpConnect->SessionID, tcpIocpConnect->SocketID, lastError);
        return false;
    }
    return true;
}
void TcpIocpServer::OnAcceptComplete(MyOverlapped* overlapped)
{
    auto tcpIocpConnect = (TcpIocpConnect*)overlapped->Connect;
    SOCKADDR_IN* remoteAddr = NULL;
    SOCKADDR_IN* localAddr = NULL;
    int remoteLen = sizeof(SOCKADDR_IN), localLen = sizeof(SOCKADDR_IN);
    SocketApi::GetInstance().GetAcceptExSockAddrs(overlapped->WsaBuffer.buf, 0,
        (sizeof(SOCKADDR_IN) + 16), (sizeof(SOCKADDR_IN) + 16), (LPSOCKADDR*)&localAddr, &localLen, (LPSOCKADDR*)&remoteAddr, &remoteLen);
    snprintf(tcpIocpConnect->RemoteAddress, sizeof(tcpIocpConnect->RemoteAddress), inet_ntoa(remoteAddr->sin_addr));
    tcpIocpConnect->RemotePort = ntohs(remoteAddr->sin_port);

    WriteLog(LogLevel::Info, "AcceptComplete: From <%s:%d>, SessionID:%lld, Socket:%lld", tcpIocpConnect->RemoteAddress, tcpIocpConnect->RemotePort, tcpIocpConnect->SessionID, tcpIocpConnect->SocketID);

    AddConnect(tcpIocpConnect);
    PostRecv(overlapped);
    auto overlapped2 = MyOverlapped::Allocate();
    overlapped2->SetBuffer(Buffer<BuffSize>::Allocate());
    overlapped2->Connect = tcpIocpConnect;
    PostRecv(overlapped2);
    PostAccept();
}

SOCKET TcpIocpServer::PrepareAcceptSocket()
{
    SOCKET socketID = WSASocket(m_AddressInfo->ai_family, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
    if (socketID == INVALID_SOCKET)
    {
        WriteLog(LogLevel::Error, "Create SOCKET Failed.");
        return INVALID_SOCKET;
    }
    if (!m_IOCompletePort->AssociateDevice((HANDLE)socketID, socketID))
    {
        WriteLog(LogLevel::Warning, "Associate CompletionPort Failed, Socket:%lld", socketID);
        return INVALID_SOCKET;
    }
    return socketID;
}
}
#endif // WINDOWS
