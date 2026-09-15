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

	virtual void OnConnect(SessionIDType sessionID, const char* ip, int port) override;
	virtual void OnDisConnect(SessionIDType sessionID, const char* ip, int port) override;
	virtual void OnRecv(SessionIDType sessionID, Spark::Buffer<Spark::BuffSize>* buffer) override;

private:
	IOBase* io_;
	IOThread* ioThread_;

	std::map<SessionIDType, int> m_MessageCounts;
};
