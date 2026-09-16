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



	virtual void OnConnect(SessionIdType sessionId, const char* ip, int port) override;
	virtual void OnDisConnect(SessionIdType sessionId, const char* ip, int port) override;
	virtual void OnRecv(SessionIdType sessionId, Spark::Buffer<Spark::BuffSize>* buffer) override;

	void Send(SessionIdType sessionId);
	void SendCommand(SessionIdType sessionId, const char* cmd);
private:
	IOBase* io_;
	IOThread* ioThread_;
	std::map<SessionIdType, int> m_MessageCounts;
	std::chrono::steady_clock::time_point m_StartSendTime;
};
