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

    virtual void OnConnect(SessionIdType sessionId, const char* ip, int port) override;
    virtual void OnDisConnect(SessionIdType sessionId, const char* ip, int port) override;
    virtual void OnRecv(SessionIdType sessionId, const char* data, size_t length) override;

    void Send(SessionIdType sessionId);
    void SendCommand(SessionIdType sessionId, const char* cmd);

private:
    IoBase* io_;
    IoThread* ioThread_;
    std::map<SessionIdType, int> messageCounts_;
    std::chrono::steady_clock::time_point startSendTime_;
};
