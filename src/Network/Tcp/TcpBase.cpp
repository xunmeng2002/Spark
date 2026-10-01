#include "Tcp/TcpBase.h"
#include <Spark/Core/Platform/Platform.h>
#include <Spark/Core/Logger/Logger.h>
#include "Tcp/TcpUtility.h"
#include <Spark/Core/Utility/TimeUtility.h>
#include <cstring>
#include <assert.h>
#include <chrono>

using namespace std;
using namespace Spark::Core;

namespace Spark::Network
{
namespace
{
// Client 断线自动重连的固定重试间隔
constexpr int AutoReconnectIntervalMs = 3000;
}

TcpBase::TcpBase(ServerTypeType serverType, const char* addressName, int milliSeconds)
    : IoBase(serverType, addressName, milliSeconds), addressInfo_(nullptr), socket_(INVALID_SOCKET), socketNotify_(nullptr),
      autoConnectPending_(false), lastConnectAttemptTime_{}, remoteAddressLen_(sizeof(remoteAddress_))
{
    SocketInit::GetInstance().Init();
    memset(&remoteAddress_, 0, sizeof(remoteAddress_));
}
TcpBase::~TcpBase()
{
    if (socket_ != INVALID_SOCKET)
    {
        closesocket(socket_);
        socket_ = INVALID_SOCKET;
    }
    if (addressInfo_ != nullptr)
    {
        freeaddrinfo(addressInfo_);
        addressInfo_ = nullptr;
    }
}

bool TcpBase::Init()
{
    SocketInit::GetInstance().Init();
    socketNotify_ = new SocketNotify();
    if (!socketNotify_->Init())
    {
        WriteLog(LogLevel::Error, "SocketNotify Init Failed.");
        return false;
    }
    if (serverType_ == ServerTypeType::Client)
    {
        // 首连同步失败时复位,交由 IO 循环的自动重连兜底(修复首连失败后永久失联)
        autoConnectPending_ = true;
        if (!ConnectToServer(address_.c_str(), atoi(port_.c_str())))
        {
            autoConnectPending_ = false;
        }
    }
    else if (serverType_ == ServerTypeType::Server)
    {
        auto ret = TcpUtility::GetAddrinfo(address_.c_str(), port_.c_str(), addressInfo_);
        if (ret < 0)
        {
            WriteLog(LogLevel::Info, "GetAddrinfo Failed. Address:%s Port:%s ret:%d, Errno:%d", address_.c_str(), port_.c_str(), ret,
                     WSAGetLastError());
            return false;
        }
        socket_ = TcpUtility::PrepareSocket(addressInfo_->ai_family);
        if (socket_ == INVALID_SOCKET)
        {
            return false;
        }
        if (!TcpUtility::Bind(socket_, addressInfo_))
        {
            return false;
        }
        return TcpUtility::Listen(socket_);
    }
    return true;
}
void TcpBase::Send(SessionIdType sessionId, LinearBuffer<BufferSize>* buffer)
{
    auto connect = static_cast<TcpConnect*>(GetConnect(sessionId));
    if (connect == nullptr)
    {
        WriteLog(LogLevel::Warning, "Send Connect Not Exist, Drop Buffer. SessionId:%lld, Len:%zu", sessionId, buffer->GetLength());
        buffer->Deallocate();
        return;
    }
    connect->PushBack(buffer);
    socketNotify_->Notify();
}
bool TcpBase::ConnectToServer(const char* address)
{
    ParseAddress(address, address_, port_);
    return ConnectToServer(address_.c_str(), atoi(port_.c_str()));
}
void TcpBase::HandleIoEvent()
{
    if (serverType_ == ServerTypeType::Client)
    {
        CheckConnect();
        TryAutoReconnect();
    }
    DoDisConnect();
    HandleTcpEvent();
}
void TcpBase::AddConnect(Connect* connect)
{
    autoConnectPending_ = false;
    IoBase::AddConnect(connect);
}
void TcpBase::RemoveConnect(Connect* connect)
{
    autoConnectPending_ = false;
    IoBase::RemoveConnect(connect);
}
void TcpBase::TryAutoReconnect()
{
    if (autoConnectPending_ || !connects_.empty())
    {
        return;
    }
    auto now = std::chrono::steady_clock::now();
    if (now - lastConnectAttemptTime_ < std::chrono::milliseconds(AutoReconnectIntervalMs))
    {
        return;
    }
    lastConnectAttemptTime_ = now;
    WriteLog(LogLevel::Info, "TcpAutoReconnect: Attempt. Address:%s, Port:%s", address_.c_str(), port_.c_str());
    if (ConnectToServer(address_.c_str(), atoi(port_.c_str())))
    {
        autoConnectPending_ = true;
    }
}
void TcpBase::DoSend(Connect* connect)
{
    auto buffer = connect->GetNextBuffer();
    while (buffer != nullptr)
    {
        int len = send((static_cast<TcpConnect*>(connect))->SocketId, buffer->GetData(), static_cast<int>(buffer->GetLength()), 0);
        if (len > 0)
        {
            buffer->Shift(len);
            if (buffer->GetLength() > 0)
            {
                connect->PushFront(buffer);
                break;
            }
            else
            {
                buffer->Deallocate();
                buffer = connect->GetNextBuffer();
            }
        }
        else
        {
            auto errorId = WSAGetLastError();
#ifdef _WIN32
            if (errorId == WSAEWOULDBLOCK || errorId == WSAENOBUFS)
#endif
#ifdef __linux__
                if (errorId == EWOULDBLOCK || errorId == ENOBUFS || errorId == ENOMEM)
#endif
                {
                    connect->PushFront(buffer);
                    break;
                }
                else
                {
                    WriteLog(LogLevel::Warning, "Tcp send Failed. SessionId:%lld, len:%d, errorId:%d", connect->SessionId, len, errorId);
                    buffer->Deallocate();
                    DisConnect(connect->SessionId);
                }
        }
    }
}
void TcpBase::DoRecv(Connect* connect)
{
    auto tcpConnect = static_cast<TcpConnect*>(connect);
    LinearBuffer<BufferSize>* buffer = LinearBuffer<BufferSize>::Allocate();
    auto data = buffer->GetData();
    int len = recv(tcpConnect->SocketId, data, BufferSize - 1, 0);
    if (len <= 0)
    {
        WriteLog(LogLevel::Info, "DisConnect For Recv. SessionId:%lld, Socket:%lld, ErrorId:%d", tcpConnect->SessionId, tcpConnect->SocketId, len);
        buffer->Deallocate();
        DisConnect(tcpConnect->SessionId);
    }
    else if (ioSubscriber_)
    {
        WriteLog(LogLevel::Ignore, "OnRecv: SessionId:%lld, Socket:%lld, RecvLen:%d", tcpConnect->SessionId, tcpConnect->SocketId, len);
        NotifySubscriberRecvSafely(tcpConnect, data, static_cast<size_t>(len));
        buffer->Deallocate();
    }
    else
    {
        buffer->Deallocate();
    }
}
void TcpBase::DoAccept()
{
    for (int i = 0; i < 5; i++)
    {
        SOCKET socketId = accept(socket_, reinterpret_cast<sockaddr*>(&remoteAddress_), &remoteAddressLen_);
        if (socketId == INVALID_SOCKET)
        {
            break;
        }
        std::string ip, port;
        auto ret = TcpUtility::GetNameinfo(reinterpret_cast<sockaddr*>(&remoteAddress_), remoteAddressLen_, ip, port);
        TcpUtility::SetSockNodelay(socketId);
        auto connect = TcpConnect::Allocate(GetSessionId(), socketId, ip, port);
        AddConnect(connect);
    }
}
}
