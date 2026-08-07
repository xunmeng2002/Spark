#pragma once
#include <Spark/Network/IO/IOBase.h>
#include <Spark/Network/IO/IOThread.h>
#include <Spark/TemplateLib/Buffer/Buffer.h>

class ShmSubscriberImpl : public IOSubscriber
{
public:
	ShmSubscriberImpl(IOBase* io, ServerTypeType serverType);

	virtual void OnConnect(SessionIDType sessionID, const char* ip, int port) override;
	virtual void OnDisConnect(SessionIDType sessionID, const char* ip, int port) override;
	virtual void OnRecv(SessionIDType sessionID, Buffer<BuffSize>* buffer) override;


public:
	bool m_Connected;
	SessionIDType m_SessionID;
private:
	IOBase* m_IO;
	ServerTypeType m_ServerType;

	char* m_Buff;
	int m_Length;
};


