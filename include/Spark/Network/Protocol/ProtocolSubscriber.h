#pragma once
#include <Spark/Network/NetworkExport.h>
#include <Spark/Network/Protocol/Package.h>

namespace Spark::Network
{
class ProtocolSubscriber
{
public:
    virtual void OnProtocolConnect(SessionIdType sessionId, const char* ip, int port) {}
    virtual void OnProtocolDisConnect(SessionIdType sessionId, const char* ip, int port) = 0;
    virtual void OnMessage(Package* package) = 0;
};
}
