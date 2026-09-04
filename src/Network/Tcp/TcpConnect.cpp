#include "Tcp/TcpConnect.h"
#include <Spark/Core/Logger/Logger.h>
#include <Spark/TemplateLib/ObjectPool/ObjectPool.h>
#include <Spark/Core/Utility/TimeUtility.h>
#include "Tcp/TcpUtility.h"

using namespace spark::core;
namespace spark::network
{
TcpConnect::TcpConnect(SessionIDType sessionID, const SOCKET& socketID, const std::string& remoteIP, const std::string& remotePort)
	:Connect(sessionID, remoteIP.c_str(), atoi(remotePort.c_str()), ConnectStatusType::Connected), SocketID(socketID)
{

}
TcpConnect::~TcpConnect()
{
#ifdef _WIN32
	shutdown(SocketID, SD_BOTH);
#endif
#ifdef __linux__
	shutdown(SocketID, SHUT_RDWR);
#endif
	closesocket(SocketID);
	SocketID = INVALID_SOCKET;
}
TcpConnect* TcpConnect::Allocate(SessionIDType sessionID, const SOCKET& socketID, const std::string& remoteIP, const std::string& remotePort)
{
	return ObjectPool<TcpConnect>::GetInstance().Allocate(sessionID, socketID, remoteIP, remotePort);
}
void TcpConnect::Deallocate()
{
	WriteLog(LogLevel::Info, "TcpConnect::Deallocate SessionID:%lld, Socket:%lld", SessionID, SocketID);
	ObjectPool<TcpConnect>::GetInstance().Deallocate(this);
}

void TcpConnect::Set(SessionIDType sessionID, const SOCKET& socketID, const std::string& remoteIP, const std::string& remotePort)
{
	SessionID = sessionID;
	SocketID = socketID;
	snprintf(RemoteAddress, sizeof(RemoteAddress), "%s", remoteIP.c_str());
	RemotePort = atoi(remotePort.c_str());
	LastSendTimePoint = std::chrono::steady_clock::now();
}
void TcpConnect::UpdateLastSendTime()
{
	LastSendTimePoint = std::chrono::steady_clock::now();
}
}
