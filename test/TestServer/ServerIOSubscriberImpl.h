#pragma once
#include <Spark/Core/Thread/ThreadBase.h>
#include <Spark/Network/IO/IOBase.h>
#include <Spark/Network/IO/IOThread.h>


using namespace Spark;
using namespace Spark::Network;
class ServerIOSubscriberImpl : public IOSubscriber
{
public:
	ServerIOSubscriberImpl(IOBase* io, IOThread* ioThread);
	~ServerIOSubscriberImpl();

	virtual void OnConnect(SessionIdType sessionId, const char* ip, int port) override;
	virtual void OnDisConnect(SessionIdType sessionId, const char* ip, int port) override;
	virtual void OnRecv(SessionIdType sessionId, Spark::Buffer<Spark::BuffSize>* buffer) override;

private:
	IOBase* io_;
	IOThread* ioThread_;

	std::map<SessionIdType, int> messageCounts_;
};
