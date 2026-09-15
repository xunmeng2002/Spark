#pragma once
#include <Spark/Core/Thread/ThreadBase.h>
#include <Spark/Network/IO/IOBase.h>
#include <Spark/Network/IO/IOThread.h>
#include <Spark/Core/Utility/TimeUtility.h>
#include <map>
#include <chrono>

using namespace Spark::Network;
class ClientIOSubscriberImpl : public IOSubscriber
{
public:
	ClientIOSubscriberImpl(IOBase* io, IOThread* ioThread);
	~ClientIOSubscriberImpl();



	virtual void OnConnect(SessionIDType sessionID, const char* ip, int port) override;
	virtual void OnDisConnect(SessionIDType sessionID, const char* ip, int port) override;
	virtual void OnRecv(SessionIDType sessionID, Spark::Buffer<Spark::BuffSize>* buffer) override;

	void Send(SessionIDType sessionID);
	void SendCommand(SessionIDType sessionID, const char* cmd);
private:
	IOBase* io_;
	IOThread* ioThread_;
	std::map<SessionIDType, int> m_MessageCounts;
	std::chrono::steady_clock::time_point m_StartSendTime;
};
