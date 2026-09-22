#pragma once
#include <Spark/Network/Io/IoBase.h>
#include <Spark/Network/Io/IoThread.h>
#include <Spark/TemplateLib/Buffer/Buffer.h>

#include <cstddef>

using namespace Spark;
using namespace Spark::Network;
class ShmSubscriberImpl : public IoSubscriber
{
public:
    ShmSubscriberImpl(IoBase* io, ServerTypeType serverType);

    virtual void OnConnect(SessionIdType sessionId, const char* ip, int port) override;
    virtual void OnDisConnect(SessionIdType sessionId, const char* ip, int port) override;
    virtual void OnRecv(SessionIdType sessionId, Buffer<BufferSize>* buffer) override;

    bool connected_;
    SessionIdType sessionId_;

private:
    IoBase* io_;
    ServerTypeType serverType_;

    char* buff_;
    size_t length_;
};
