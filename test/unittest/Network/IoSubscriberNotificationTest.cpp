#include <Spark/Network/Io/IoBase.h>
#include <Spark/TemplateLib/Buffer/LinearBuffer.h>
#include "doctest/doctest.h"

#include <algorithm>
#include <atomic>
#include <latch>
#include <mutex>
#include <stdexcept>
#include <thread>
#include <vector>

using namespace Spark;
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
    SessionIdType NextSessionId() { return GetSessionId(); }

protected:
    void DoSend(Connect*) override {}
    void DoRecv(Connect*) override {}
};
}

TEST_CASE("IoSubscriberNotificationTest.AnOnConnectFailureIsContainedAndTheConnectionStaysRegistered")
{
    ProbeIo io;
    SubscriberNotificationProbe subscriber;
    subscriber.ThrowOnConnect = true;
    io.Subscribe(&subscriber);
    ProbeConnect connect;

    io.RegisterConnect(&connect);

    CHECK_EQ(subscriber.ConnectCount, 1);
    CHECK(io.HasRegisteredConnect(ProbeSessionId));
    CHECK_EQ(connect.DeallocateCount, 0);
}

TEST_CASE("IoSubscriberNotificationTest.AnOnDisConnectFailureDoesNotSkipTheTeardown")
{
    ProbeIo io;
    SubscriberNotificationProbe subscriber;
    subscriber.ThrowOnDisConnect = true;
    io.Subscribe(&subscriber);
    ProbeConnect connect;
    io.RegisterConnect(&connect);

    io.UnregisterConnect(&connect);

    CHECK_EQ(subscriber.ConnectCount, 1);
    CHECK_EQ(subscriber.DisConnectCount, 1);
    CHECK_FALSE(io.HasRegisteredConnect(ProbeSessionId));
    CHECK_EQ(connect.DeallocateCount, 1);
}

TEST_CASE("IoSubscriberNotificationTest.AnOnRecvFailureIsContainedAndDoesNotSkipWhatFollows")
{
    ProbeIo io;
    SubscriberNotificationProbe subscriber;
    subscriber.ThrowOnRecv = true;
    io.Subscribe(&subscriber);
    ProbeConnect connect;

    bool deliveredRecvReturnedToItsCaller = false;
    io.DeliverRecv(&connect, "Spark", 5);
    deliveredRecvReturnedToItsCaller = true;

    CHECK_EQ(subscriber.RecvCount, 1);
    CHECK(deliveredRecvReturnedToItsCaller);
}

TEST_CASE("IoSubscriberNotificationTest.AnUnregisteredConnectionIsReturnedWithoutADisConnectNotification")
{
    ProbeIo io;
    SubscriberNotificationProbe subscriber;
    io.Subscribe(&subscriber);
    ProbeConnect connect;

    io.UnregisterConnect(&connect);

    CHECK_EQ(subscriber.ConnectCount, 0);
    CHECK_EQ(subscriber.DisConnectCount, 0);
    CHECK_EQ(connect.DeallocateCount, 1);
}

TEST_CASE("IoSubscriberNotificationTest.ARepeatedSessionIdIsNotRegisteredAndNotNotifiedASecondTime")
{
    ProbeIo io;
    SubscriberNotificationProbe subscriber;
    io.Subscribe(&subscriber);
    ProbeConnect firstConnect;
    ProbeConnect secondConnect;

    io.RegisterConnect(&firstConnect);
    io.RegisterConnect(&secondConnect);

    CHECK_EQ(subscriber.ConnectCount, 1);
    CHECK(io.IsRegisteredConnect(&firstConnect));
    CHECK_FALSE(io.IsRegisteredConnect(&secondConnect));
    CHECK_EQ(firstConnect.DeallocateCount, 0);
    // 登记失败不是「拒绝」：对象仍归调用方处置，本层不归还它。
    CHECK_EQ(secondConnect.DeallocateCount, 0);
}

TEST_CASE("IoSubscriberNotificationTest.ARemoveOfAnotherConnectNeverErasesTheRegisteredEntry")
{
    ProbeIo io;
    SubscriberNotificationProbe subscriber;
    io.Subscribe(&subscriber);
    ProbeConnect registeredConnect;
    ProbeConnect sameSessionIdConnect;
    io.RegisterConnect(&registeredConnect);

    io.UnregisterConnect(&sameSessionIdConnect);

    CHECK(io.IsRegisteredConnect(&registeredConnect));
    CHECK_EQ(subscriber.DisConnectCount, 0);
    CHECK_EQ(registeredConnect.DeallocateCount, 0);
    CHECK_EQ(sameSessionIdConnect.DeallocateCount, 1);
}

TEST_CASE("IoSubscriberNotificationTest.SessionIdAdvancesByExactlyOneEachTime")
{
    ProbeIo io;
    SessionIdType previousId = io.NextSessionId();

    for (int step = 0; step < 1000; ++step)
    {
        SessionIdType currentId = io.NextSessionId();
        CHECK_EQ(currentId, previousId + 1);
        previousId = currentId;
    }
}

TEST_CASE("IoSubscriberNotificationTest.SessionIdsTakenConcurrentlyAreAllDistinct")
{
    constexpr int ThreadCount = 4;
    constexpr int IdsPerThread = 5;
    ProbeIo io;
    std::vector<SessionIdType> takenIds;
    std::mutex takenIdsMutex;
    std::latch startGate(1);

    auto takeSessionIds = [&]()
    {
        startGate.wait();
        std::vector<SessionIdType> threadIds;
        for (int i = 0; i < IdsPerThread; ++i)
        {
            threadIds.push_back(io.NextSessionId());
        }
        std::lock_guard<std::mutex> guard(takenIdsMutex);
        takenIds.insert(takenIds.end(), threadIds.begin(), threadIds.end());
    };

    std::vector<std::thread> threads;
    for (int i = 0; i < ThreadCount; ++i)
    {
        threads.emplace_back(takeSessionIds);
    }
    startGate.count_down();
    for (auto& thread : threads)
    {
        thread.join();
    }

    std::sort(takenIds.begin(), takenIds.end());
    CHECK_EQ(takenIds.size(), static_cast<size_t>(ThreadCount) * IdsPerThread);
    CHECK_EQ(std::adjacent_find(takenIds.begin(), takenIds.end()), takenIds.end());
}
