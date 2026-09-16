#pragma once
#include <Spark/Network/NetworkExport.h>
#include <Spark/Types.h>
#include "Tcp/SocketInit.h"
#include <Spark/Network/IO/Connect.h>
#include <mutex>
#include <string>
#include <chrono>
#include <atomic>

namespace Spark::Network
{
class NETWORK_EXPORTS TcpConnect : public Connect
{
public:
	TcpConnect(SessionIdType sessionId, const SOCKET& socketId, const std::string& remoteIP, const std::string& remotePort);
	virtual ~TcpConnect();

	static TcpConnect* Allocate(SessionIdType sessionId, const SOCKET& socketId, const std::string& remoteIP, const std::string& remotePort);
	virtual void Deallocate() override;

	virtual void Set(SessionIdType sessionId, const SOCKET& socketId, const std::string& remoteIP, const std::string& remotePort);
	void UpdateLastSendTime();


	SOCKET SocketId;
	std::chrono::steady_clock::time_point LastSendTimePoint;
};
}
