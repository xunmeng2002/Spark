#pragma once
#include <Spark/Network/Protocol/Protocol.h>



using namespace Spark::Network;
class XtpServer : public Protocol, public ProtocolSubscriber
{
public:
	XtpServer();
	virtual ~XtpServer();

	virtual void OnProtocolConnect(SessionIDType sessionID, const char* ip, int port) override;
	virtual void OnProtocolDisConnect(SessionIDType sessionID, const char* ip, int port) override;
	virtual void OnMessage(Package* package) override;

	bool connected_;
	SessionIDType sessionId_;
	int m_RecvCount;
};



void TestXtpServer();


