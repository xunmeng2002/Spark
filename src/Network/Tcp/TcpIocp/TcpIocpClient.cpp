#include "Tcp/TcpIocp/TcpIocpClient.h"
#include "Tcp/TcpIocp/TcpIocpCompletePort.h"
#include "Tcp/TcpIocp/TcpIocpConnect.h"
#include "Tcp/TcpIocp/TcpIocpSockApi.h"
#include <Spark/Network/IO/IOUtility.h>
#include "Tcp/TcpUtility.h"
#include <Spark/Core/Logger/Logger.h>

#ifdef WINDOWS
using namespace spark::core;

namespace spark::network
{
TcpIocpClient::TcpIocpClient(const char* addressName, int milliSeconds, int backlog)
	:TcpIocpBase(ServerTypeType::Client, addressName, milliSeconds, backlog)
{

}
bool TcpIocpClient::Init()
{
    if (!TcpIocpBase::Init())
        return false;
    auto ret = TcpUtility::GetClientAddrinfo(nullptr, "0", m_ClientLocalAddressInfo, m_AddressInfo->ai_family);
    if (ret < 0)
    {
        WriteLog(LogLevel::Info, "GetAddrinfo for m_ClientLocalAddressInfo Failed. ret:%d, Errno:%d", ret, WSAGetLastError());
        return false;
    }
    PostConnect();
    return true;
}
bool TcpIocpClient::ConnectToServer(const char* ip, unsigned short port)
{
    m_Address = ip;
    m_Port = std::to_string(port);
    auto ret = TcpUtility::GetAddrinfo(m_Address.c_str(), m_Port.c_str(), m_AddressInfo);
    if (ret < 0)
    {
        WriteLog(LogLevel::Info, "GetAddrinfo Failed. Address:%s Port:%s ret:%d, Errno:%d", m_Address.c_str(), m_Port.c_str(), ret, WSAGetLastError());
        return false;
    }
    return PostConnect();
}

bool TcpIocpClient::PostConnect()
{
    SOCKET socketID = PrepareConnectSocket();
    if (socketID == INVALID_SOCKET)
    {
        WriteLog(LogLevel::Error, "PrepareConnectSocket SOCKET Failed.");
        return false;
    }
    TcpIocpConnect* tcpIocpConnect = TcpIocpConnect::Allocate(GetSessionID(), socketID, m_Address, m_Port);
    MyOverlapped* overlapped = MyOverlapped::Allocate();
    overlapped->SetBuffer(Buffer<BuffSize>::Allocate());
    overlapped->EventID = IocpEvent::EventConnect;
    overlapped->Connect = tcpIocpConnect;

    WriteLog(LogLevel::Info, "PostConnect For SessionID:%lld, Socket:%lld", tcpIocpConnect->SessionID, tcpIocpConnect->SocketID);
    DWORD transBytes = 0;
    auto ret = SocketApi::GetInstance().ConnectEx(tcpIocpConnect->SocketID, (const sockaddr*)m_AddressInfo->ai_addr, sizeof(SOCKADDR_IN),
        NULL, 0, &transBytes, overlapped);
    if (!ret && WSAGetLastError() != ERROR_IO_PENDING)
    {
        WriteErrorLog(WSAGetLastError(), "Call ConnectEx Failed.");
        return false;
    }
    return true;
}
void TcpIocpClient::OnConnectComplete(MyOverlapped* overlapped)
{
    PostRecv(overlapped);
    auto overlapped2 = MyOverlapped::Allocate();
    overlapped2->SetBuffer(Buffer<BuffSize>::Allocate());
    overlapped2->Connect = overlapped->Connect;
    PostRecv(overlapped2);
    AddConnect(overlapped->Connect);
}
SOCKET TcpIocpClient::PrepareConnectSocket()
{
    WriteLog(LogLevel::Info, "AI_Family for m_ClientLocalAddressInfo:%d, m_AddressInfo:%d", m_ClientLocalAddressInfo->ai_family, m_AddressInfo->ai_family);
    SOCKET socketID = WSASocket(m_ClientLocalAddressInfo->ai_family, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
    if (socketID == INVALID_SOCKET)
    {
        WriteLog(LogLevel::Error, "Create SOCKET Failed.");
        closesocket(socketID);
        return INVALID_SOCKET;
    }
    int on = 1;
    if (setsockopt(socketID, SOL_SOCKET, SO_REUSEADDR, (const char*)&on, sizeof(on)) != 0)
    {
        WriteErrorLog(WSAGetLastError(), "setsockopt Failed. ErrorID:%d, result:%d");
        closesocket(socketID);
        return INVALID_SOCKET;
    }
    if (!TcpUtility::Bind(socketID, m_ClientLocalAddressInfo))
    {
        closesocket(socketID);
        return INVALID_SOCKET;
    }
    if (!m_IOCompletePort->AssociateDevice((HANDLE)socketID, socketID))
    {
        WriteErrorLog(WSAGetLastError(), "AssociateDevice Failed.");
        closesocket(socketID);
        return INVALID_SOCKET;
    }
    return socketID;
}
}

#endif // WINDOWS
