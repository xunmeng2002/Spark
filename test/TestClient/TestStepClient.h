#pragma once
#include "Packages.h"
#include <Spark/Network/Protocol/Protocol.h>
#include <chrono>


class StepClient : public Spark::Network::Protocol, public Spark::Network::ProtocolSubscriber
{
public:
	StepClient();
	virtual ~StepClient();

	virtual void OnProtocolConnect(SessionIDType sessionID, const char* ip, int port) override;
	virtual void OnProtocolDisConnect(SessionIDType sessionID, const char* ip, int port) override;
	virtual void OnMessage(Package* package) override;

	void SendReqInsertOrder(int index);

	bool connected_;
	SessionIDType sessionId_;

	std::chrono::steady_clock::time_point m_StartTime;
	int m_RecvCount;
	Spark::Packages::ReqInsertOrderPackage* m_ReqInsertOrder;
};



void TestStepClient();


