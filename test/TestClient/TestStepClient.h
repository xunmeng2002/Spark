#pragma once
#include "Packages.h"
#include <Spark/Network/Protocol/Protocol.h>
#include <chrono>


class StepClient : public spark::network::Protocol, public spark::network::ProtocolSubscriber
{
public:
	StepClient();
	virtual ~StepClient();

	virtual void OnProtocolConnect(SessionIDType sessionID, const char* ip, int port) override;
	virtual void OnProtocolDisConnect(SessionIDType sessionID, const char* ip, int port) override;
	virtual void OnMessage(Package* package) override;

	void SendReqInsertOrder(int index);

	bool m_Connected;
	SessionIDType m_SessionID;

	std::chrono::steady_clock::time_point m_StartTime;
	int m_RecvCount;
	spark::packages::ReqInsertOrderPackage* m_ReqInsertOrder;
};



void TestStepClient();


