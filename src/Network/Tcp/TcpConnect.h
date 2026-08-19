#pragma once
#include <Spark/Network/NetworkExport.h>
#include <Spark/Types.h>
#include "Tcp/SocketInit.h"
#include <Spark/Network/IO/Connect.h>
#include <mutex>
#include <string>
#include <chrono>
#include <atomic>

namespace spark::network
{
class NETWORK_EXPORTS TcpConnect : public Connect
{
public:
	TcpConnect(SessionIDType sessionID, const SOCKET& socketID, const std::string& remoteIP, const std::string& remotePort);
	virtual ~TcpConnect();

	static TcpConnect* Allocate(SessionIDType sessionID, const SOCKET& socketID, const std::string& remoteIP, const std::string& remotePort);
	virtual void Deallocate() override;

	virtual void Set(SessionIDType sessionID, const SOCKET& socketID, const std::string& remoteIP, const std::string& remotePort);
	void UpdateLastSendTime();


	SOCKET SocketID;
	std::chrono::steady_clock::time_point LastSendTimePoint;
};
}
