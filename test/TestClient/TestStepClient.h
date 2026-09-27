#pragma once
#include "Packages.h"
#include <Spark/Network/Protocol/Protocol.h>
#include <chrono>

class StepClient : public Spark::Network::Protocol, public Spark::Network::ProtocolSubscriber
{
public:
    StepClient();
    virtual ~StepClient();

    virtual void OnProtocolConnect(SessionIdType sessionId, const char* ip, int port) override;
    virtual void OnProtocolDisConnect(SessionIdType sessionId, const char* ip, int port) override;
    virtual void OnMessage(Package* ownedPackage) override;

    void SendReqInsertOrder(int index);

    bool connected_;
    SessionIdType sessionId_;

    std::chrono::steady_clock::time_point startTime_;
    int recvCount_;
};

void TestStepClient();
