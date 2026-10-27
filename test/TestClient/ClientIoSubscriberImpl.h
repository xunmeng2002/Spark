#pragma once
#include <Spark/Core/Thread/ThreadBase.h>
#include <Spark/Network/Io/IoBase.h>
#include <Spark/Network/Io/IoThread.h>
#include <Spark/Core/Utility/TimeUtility.h>
#include <map>
#include <chrono>

using namespace Spark::Network;
class ClientIoSubscriberImpl : public IoSubscriber
{
public:
    ClientIoSubscriberImpl(IoBase* io, IoThread* ioThread);
    ~ClientIoSubscriberImpl();

    virtual void OnConnect(Spark::SessionIdType sessionId, const char* ip, int port) override;
    virtual void OnDisConnect(Spark::SessionIdType sessionId, const char* ip, int port) override;
    virtual void OnRecv(Spark::SessionIdType sessionId, const char* data, size_t length) override;

    void Send(Spark::SessionIdType sessionId);
    void SendCommand(Spark::SessionIdType sessionId, const char* cmd);

private:
    IoBase* io_;
    IoThread* ioThread_;
    std::map<Spark::SessionIdType, int> messageCounts_;
    std::chrono::steady_clock::time_point startSendTime_;
};
