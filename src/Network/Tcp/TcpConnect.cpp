#include "Tcp/TcpConnect.h"
#include <Spark/Core/Logger/Logger.h>
#include <Spark/TemplateLib/ObjectPool/ObjectPool.h>
#include <Spark/Core/Utility/TimeUtility.h>
#include "Tcp/TcpUtility.h"

using namespace Spark::Core;
namespace Spark::Network
{
TcpConnect::TcpConnect(SessionIdType sessionId, const SOCKET& socketId, const std::string& remoteIP, const std::string& remotePort)
    : Connect(sessionId, remoteIP.c_str(), atoi(remotePort.c_str()), ConnectStatusType::Connected), SocketId(socketId)
{
}
TcpConnect::~TcpConnect()
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
TcpConnect* TcpConnect::Allocate(SessionIdType sessionId, const SOCKET& socketId, const std::string& remoteIP, const std::string& remotePort)
{
    return ObjectPool<TcpConnect>::GetInstance().Allocate(sessionId, socketId, remoteIP, remotePort);
}
void TcpConnect::Deallocate()
{
    WriteLog(LogLevel::Info, "TcpConnect::Deallocate SessionId:%lld, Socket:%lld", SessionId, SocketId);
    ObjectPool<TcpConnect>::GetInstance().Deallocate(this);
}

void TcpConnect::Set(SessionIdType sessionId, const SOCKET& socketId, const std::string& remoteIP, const std::string& remotePort)
{
    SessionId = sessionId;
    SocketId = socketId;
    snprintf(RemoteAddress, sizeof(RemoteAddress), "%s", remoteIP.c_str());
    RemotePort = atoi(remotePort.c_str());
    LastSendTimePoint = std::chrono::steady_clock::now();
}
void TcpConnect::UpdateLastSendTime()
{
    LastSendTimePoint = std::chrono::steady_clock::now();
}
}
