#pragma once
#include <Spark/Network/IO/IOBase.h>
#include <Spark/Network/IO/IOThread.h>
#include <Spark/TemplateLib/Buffer/Buffer.h>

using namespace Spark;
using namespace Spark::Network;
class ShmSubscriberImpl : public IOSubscriber
{
public:
	ShmSubscriberImpl(IOBase* io, ServerTypeType serverType);

	virtual void OnConnect(SessionIDType sessionID, const char* ip, int port) override;
	virtual void OnDisConnect(SessionIDType sessionID, const char* ip, int port) override;
	virtual void OnRecv(SessionIDType sessionID, Buffer<BuffSize>* buffer) override;


public:
	bool connected_;
	SessionIDType sessionId_;
private:
	IOBase* io_;
	ServerTypeType serverType_;

	char* buff_;
	int length_;
};


