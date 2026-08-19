#pragma once
#include <Spark/Core/Thread/ThreadBase.h>
#include <Spark/Network/IO/IOBase.h>
#include <Spark/Network/IO/IOThread.h>


using namespace spark;
using namespace spark::network;
class ServerIOSubscriberImpl : public IOSubscriber
{
public:
	ServerIOSubscriberImpl(IOBase* io, IOThread* ioThread);
	~ServerIOSubscriberImpl();

	virtual void OnConnect(SessionIDType sessionID, const char* ip, int port) override;
	virtual void OnDisConnect(SessionIDType sessionID, const char* ip, int port) override;
	virtual void OnRecv(SessionIDType sessionID, spark::Buffer<spark::BuffSize>* buffer) override;

private:
	IOBase* m_IO;
	IOThread* m_IOThread;

	std::map<SessionIDType, int> m_MessageCounts;
};
