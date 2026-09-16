#pragma once
#include "Packages.h"
#include <Spark/Network/Protocol/Protocol.h>
#include <chrono>

class XtpClient : public Spark::Network::Protocol, public Spark::Network::ProtocolSubscriber
{
public:
	XtpClient();
	virtual ~XtpClient();

	virtual void OnProtocolConnect(SessionIdType sessionId, const char* ip, int port) override;
	virtual void OnProtocolDisConnect(SessionIdType sessionId, const char* ip, int port) override;
	virtual void OnMessage(Package* package) override;

	void SendReqInsertOrder(int index);

	bool connected_;
	SessionIdType sessionId_;

	std::chrono::steady_clock::time_point startTime_;
	int recvCount_;
    Spark::Packages::ReqInsertOrderPackage* reqInsertOrder_;
};



void TestXtpClient();


