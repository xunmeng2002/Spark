#pragma once
#include <Spark/Core/Thread/ThreadBase.h>
#include <Spark/Network/Io/IoBase.h>
#include <Spark/Network/Io/IoThread.h>


using namespace Spark;
using namespace Spark::Network;
class ServerIoSubscriberImpl : public IoSubscriber
{
public:
    ServerIoSubscriberImpl(IoBase* io, IoThread* ioThread);
    ~ServerIoSubscriberImpl();

    virtual void OnConnect(SessionIdType sessionId, const char* ip, int port) override;
    virtual void OnDisConnect(SessionIdType sessionId, const char* ip, int port) override;
    virtual void OnRecv(SessionIdType sessionId, Spark::Buffer<Spark::BuffSize>* buffer) override;

private:
    IoBase* io_;
    IoThread* ioThread_;

    std::map<SessionIdType, int> messageCounts_;
};
