#pragma once
#include <Spark/Network/Protocol/Protocol.h>

using namespace Spark::Network;
class XtpServer : public Protocol, public ProtocolSubscriber
{
public:
    XtpServer();
    virtual ~XtpServer();

    virtual void OnProtocolConnect(Spark::SessionIdType sessionId, const char* ip, int port) override;
    virtual void OnProtocolDisConnect(Spark::SessionIdType sessionId, const char* ip, int port) override;
    virtual void OnMessage(Package* ownedPackage) override;

    bool connected_;
    Spark::SessionIdType sessionId_;
    int recvCount_;
};

void TestXtpServer();
