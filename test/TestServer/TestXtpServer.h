#pragma once
#include <Spark/Network/Protocol/Protocol.h>



using namespace Spark::Network;
class XtpServer : public Protocol, public ProtocolSubscriber
{
public:
	XtpServer();
	virtual ~XtpServer();

	virtual void OnProtocolConnect(SessionIdType sessionId, const char* ip, int port) override;
	virtual void OnProtocolDisConnect(SessionIdType sessionId, const char* ip, int port) override;
	virtual void OnMessage(Package* package) override;

	bool connected_;
	SessionIdType sessionId_;
	int recvCount_;
};



void TestXtpServer();


