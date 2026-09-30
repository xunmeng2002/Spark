#include <gtest/gtest.h>
#include <Spark/Network/Io/IoBase.h>
#include <Spark/TemplateLib/Buffer/LinearBuffer.h>

#include <stdexcept>

using namespace Spark::Network;

namespace
{
constexpr SessionIdType ProbeSessionId = 20260930001LL;

class ProbeConnect : public Connect
{
public:
    ProbeConnect() : Connect(ProbeSessionId, "127.0.0.1", 10001, ConnectStatusType::Connected) {}

    void Deallocate() override { ++DeallocateCount; }

    int DeallocateCount = 0;
};

class SubscriberNotificationProbe : public IoSubscriber
{
public:
    void OnConnect(SessionIdType, const char*, int) override
    {
        ++ConnectCount;
        if (ThrowOnConnect)
        {
            throw std::runtime_error("Probe OnConnect Failure.");
        }
    }
    void OnDisConnect(SessionIdType, const char*, int) override
    {
        ++DisConnectCount;
        if (ThrowOnDisConnect)
        {
            throw std::runtime_error("Probe OnDisConnect Failure.");
        }
    }
    void OnRecv(SessionIdType, const char*, size_t) override
    {
        ++RecvCount;
        if (ThrowOnRecv)
        {
            throw std::runtime_error("Probe OnRecv Failure.");
        }
    }

    bool ThrowOnConnect = false;
    bool ThrowOnDisConnect = false;
    bool ThrowOnRecv = false;
    int ConnectCount = 0;
    int DisConnectCount = 0;
    int RecvCount = 0;
};

class ProbeIo : public IoBase
{
public:
    ProbeIo() : IoBase(ServerTypeType::Server, "tcp://127.0.0.1:10001", 1000) {}

    void Send(SessionIdType, Spark::LinearBuffer<BufferSize>*) override {}
    void HandleIoEvent() override {}

    void RegisterConnect(Connect* connect) { AddConnect(connect); }
    void UnregisterConnect(Connect* connect) { RemoveConnect(connect); }
    bool HasRegisteredConnect(SessionIdType sessionId) { return GetConnect(sessionId) != nullptr; }
    bool IsRegisteredConnect(Connect* connect) { return GetConnect(connect->SessionId) == connect; }
    void DeliverRecv(Connect* connect, const char* data, size_t length) { NotifySubscriberRecvSafely(connect, data, length); }

protected:
    void DoSend(Connect*) override {}
    void DoRecv(Connect*) override {}
};
}

TEST(IoSubscriberNotificationTest, AnOnConnectFailureIsContainedAndTheConnectionStaysRegistered)
{
    ProbeIo io;
    SubscriberNotificationProbe subscriber;
    subscriber.ThrowOnConnect = true;
    io.Subscribe(&subscriber);
    ProbeConnect connect;

    io.RegisterConnect(&connect);

    EXPECT_EQ(subscriber.ConnectCount, 1);
    EXPECT_TRUE(io.HasRegisteredConnect(ProbeSessionId));
    EXPECT_EQ(connect.DeallocateCount, 0);
}

TEST(IoSubscriberNotificationTest, AnOnDisConnectFailureDoesNotSkipTheTeardown)
{
    ProbeIo io;
    SubscriberNotificationProbe subscriber;
    subscriber.ThrowOnDisConnect = true;
    io.Subscribe(&subscriber);
    ProbeConnect connect;
    io.RegisterConnect(&connect);

    io.UnregisterConnect(&connect);

    EXPECT_EQ(subscriber.ConnectCount, 1);
    EXPECT_EQ(subscriber.DisConnectCount, 1);
    EXPECT_FALSE(io.HasRegisteredConnect(ProbeSessionId));
    EXPECT_EQ(connect.DeallocateCount, 1);
}

TEST(IoSubscriberNotificationTest, AnOnRecvFailureIsContainedAndDoesNotSkipWhatFollows)
{
    ProbeIo io;
    SubscriberNotificationProbe subscriber;
    subscriber.ThrowOnRecv = true;
    io.Subscribe(&subscriber);
    ProbeConnect connect;

    bool deliveredRecvReturnedToItsCaller = false;
    io.DeliverRecv(&connect, "Spark", 5);
    deliveredRecvReturnedToItsCaller = true;

    EXPECT_EQ(subscriber.RecvCount, 1);
    EXPECT_TRUE(deliveredRecvReturnedToItsCaller);
}

TEST(IoSubscriberNotificationTest, AnUnregisteredConnectionIsReturnedWithoutADisConnectNotification)
{
    ProbeIo io;
    SubscriberNotificationProbe subscriber;
    io.Subscribe(&subscriber);
    ProbeConnect connect;

    io.UnregisterConnect(&connect);

    EXPECT_EQ(subscriber.ConnectCount, 0);
    EXPECT_EQ(subscriber.DisConnectCount, 0);
    EXPECT_EQ(connect.DeallocateCount, 1);
}

TEST(IoSubscriberNotificationTest, ARepeatedSessionIdIsRefusedWithoutASecondConnectNotification)
{
    ProbeIo io;
    SubscriberNotificationProbe subscriber;
    io.Subscribe(&subscriber);
    ProbeConnect firstConnect;
    ProbeConnect secondConnect;

    io.RegisterConnect(&firstConnect);
    io.RegisterConnect(&secondConnect);

    EXPECT_EQ(subscriber.ConnectCount, 1);
    EXPECT_TRUE(io.IsRegisteredConnect(&firstConnect));
    EXPECT_FALSE(io.IsRegisteredConnect(&secondConnect));
    EXPECT_EQ(firstConnect.DeallocateCount, 0);
    EXPECT_EQ(secondConnect.DeallocateCount, 0);
}
