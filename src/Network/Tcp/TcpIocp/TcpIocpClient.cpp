#include "Tcp/TcpIocp/TcpIocpClient.h"
#include "Tcp/TcpIocp/TcpIocpCompletePort.h"
#include "Tcp/TcpIocp/TcpIocpConnect.h"
#include "Tcp/TcpIocp/TcpIocpSockApi.h"
#include <Spark/Network/Io/IoUtility.h>
#include "Tcp/TcpUtility.h"
#include <Spark/Core/Logger/Logger.h>

#ifdef _WIN32
using namespace Spark::Core;

namespace Spark::Network
{
TcpIocpClient::TcpIocpClient(const char* addressName, int milliSeconds, int backlog)
    : TcpIocpBase(ServerTypeType::Client, addressName, milliSeconds, backlog), clientLocalAddressInfo_(nullptr)
{
}
bool TcpIocpClient::Init()
{
    if (!TcpIocpBase::Init())
        return false;
    auto ret = TcpUtility::GetClientAddrinfo(nullptr, "0", clientLocalAddressInfo_, addressInfo_->ai_family);
    if (ret < 0)
    {
        WriteLog(LogLevel::Info, "GetAddrinfo for clientLocalAddressInfo_ Failed. ret:%d, Errno:%d", ret, WSAGetLastError());
        return false;
    }
    // 首连走 PostConnect 而非 TcpBase::Init 的 ConnectToServer，必须自行复位该标志：
    // 否则 IOCP 的 IO 循环会认为没有在途连接，TryAutoReconnect 再连一次，出现双连接
    autoConnectPending_ = true;
    if (!PostConnect())
    {
        autoConnectPending_ = false;
    }
    return true;
}
bool TcpIocpClient::ConnectToServer(const char* ip, unsigned short port)
{
    address_ = ip;
    port_ = std::to_string(port);
    auto ret = TcpUtility::GetAddrinfo(address_.c_str(), port_.c_str(), addressInfo_);
    if (ret < 0)
    {
        WriteLog(LogLevel::Info, "GetAddrinfo Failed. Address:%s Port:%s ret:%d, Errno:%d", address_.c_str(), port_.c_str(), ret, WSAGetLastError());
        return false;
    }
    return PostConnect();
}

bool TcpIocpClient::PostConnect()
{
    SOCKET socketId = PrepareConnectSocket();
    if (socketId == INVALID_SOCKET)
    {
        WriteLog(LogLevel::Error, "PrepareConnectSocket SOCKET Failed.");
        return false;
    }
    TcpIocpConnect* tcpIocpConnect = TcpIocpConnect::Allocate(GetSessionId(), socketId, address_, port_);
    MyOverlapped* overlapped = MyOverlapped::Allocate();
    overlapped->SetBuffer(LinearBuffer<BufferSize>::Allocate());
    overlapped->EventId = IocpEvent::EventConnect;
    overlapped->Connect = tcpIocpConnect;

    WriteLog(LogLevel::Info, "PostConnect For SessionId:%lld, Socket:%lld", tcpIocpConnect->SessionId, tcpIocpConnect->SocketId);
    DWORD transBytes = 0;
    auto ret = SocketApi::GetInstance().ConnectEx(tcpIocpConnect->SocketId, static_cast<const sockaddr*>(addressInfo_->ai_addr), sizeof(SOCKADDR_IN),
                                                  NULL, 0, &transBytes, overlapped);
    if (!ret && WSAGetLastError() != ERROR_IO_PENDING)
    {
        WriteErrorLog(WSAGetLastError(), "Call ConnectEx Failed.");
        ReleaseUnsubmittedIoRequest(tcpIocpConnect, overlapped);
        return false;
    }
    return true;
}
void TcpIocpClient::OnConnectComplete(MyOverlapped* overlapped)
{
    PostRecv(overlapped);
    auto overlapped2 = MyOverlapped::Allocate();
    overlapped2->SetBuffer(LinearBuffer<BufferSize>::Allocate());
    overlapped2->Connect = overlapped->Connect;
    PostRecv(overlapped2);
    AddConnect(overlapped->Connect);
}
SOCKET TcpIocpClient::PrepareConnectSocket()
{
    WriteLog(LogLevel::Info, "AI_Family for clientLocalAddressInfo_:%d, addressInfo_:%d", clientLocalAddressInfo_->ai_family,
             addressInfo_->ai_family);
    SOCKET socketId = WSASocket(clientLocalAddressInfo_->ai_family, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
    if (socketId == INVALID_SOCKET)
    {
        WriteLog(LogLevel::Error, "Create SOCKET Failed.");
        closesocket(socketId);
        return INVALID_SOCKET;
    }
    int on = 1;
    if (setsockopt(socketId, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&on), sizeof(on)) != 0)
    {
        WriteErrorLog(WSAGetLastError(), "setsockopt Failed. ErrorId:%d, result:%d");
        closesocket(socketId);
        return INVALID_SOCKET;
    }
    if (!TcpUtility::Bind(socketId, clientLocalAddressInfo_))
    {
        closesocket(socketId);
        return INVALID_SOCKET;
    }
    if (!ioCompletePort_->AssociateDevice((HANDLE)socketId, socketId))
    {
        WriteErrorLog(WSAGetLastError(), "AssociateDevice Failed.");
        closesocket(socketId);
        return INVALID_SOCKET;
    }
    return socketId;
}
}

#endif // _WIN32
