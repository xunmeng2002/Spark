#include <gtest/gtest.h>

#if defined(_WIN32)

#include "PackageFactory.h"
#include "Packages.h"
#include <Spark/Core/Logger/Logger.h>
#include <Spark/Core/Utility/Utility.h>
#include <Spark/Network/Io/IoBase.h>
#include <Spark/Network/Io/IoFactory.h>
#include <Spark/Network/Io/IoThread.h>
#include <Spark/Network/Protocol/Protocol.h>
#include <Spark/TemplateLib/ObjectPool/ObjectPool.h>

#include <Windows.h>

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <utility>
#include <vector>

using namespace Spark;
using namespace Spark::Core;
using namespace Spark::Network;
using namespace Spark::Packages;

namespace
{
constexpr char IocpLoopbackAddress[] = "tcp://127.0.0.1:20011";
constexpr int RoundTripFrameCount = 200;
constexpr char RejectedConnectAddress[] = "tcp://127.0.0.1:0";
constexpr char AbandonedConnectAddress[] = "tcp://192.0.2.1:9";
constexpr char ThrowawayListenAddress[] = "tcp://127.0.0.1:0";
constexpr int BuildTeardownRoundCount = 20;
constexpr long AllowedHandleGrowth = 2;
constexpr int FirstOrderIndex = 1;
constexpr int PriceBase = 100;
constexpr char ExpectedAccountId[] = "Xunmeng001";
constexpr char ExpectedExchangeId[] = "SHSE";
constexpr char ExpectedInstrumentId[] = "600036";

struct EchoedOrder
{
    int Index;
    double Price;
    long long Volume;
    std::string AccountId;
    std::string ExchangeId;
    std::string InstrumentId;
};

class EchoedOrderRecorder
{
public:
    void Record(EchoedOrder order)
    {
        {
            std::lock_guard<std::mutex> guard(mutex_);
            orders_.push_back(std::move(order));
        }
        changed_.notify_all();
    }
    bool WaitForCount(size_t expectedCount, std::chrono::milliseconds timeout)
    {
        std::unique_lock<std::mutex> lock(mutex_);
        return changed_.wait_for(lock, timeout, [this, expectedCount] { return orders_.size() >= expectedCount; });
    }
    size_t Count() const
    {
        std::lock_guard<std::mutex> guard(mutex_);
        return orders_.size();
    }
    std::vector<EchoedOrder> Snapshot() const
    {
        std::lock_guard<std::mutex> guard(mutex_);
        return orders_;
    }

private:
    mutable std::mutex mutex_;
    std::condition_variable changed_;
    std::vector<EchoedOrder> orders_;
};

bool TryExtractEchoedOrder(const Package* package, EchoedOrder& order)
{
    const auto* insertOrderPackage = dynamic_cast<const ReqInsertOrderPackage*>(package);
    if (insertOrderPackage == nullptr || insertOrderPackage->ReqInsertOrder == nullptr)
    {
        return false;
    }
    const ReqInsertOrderField* field = insertOrderPackage->ReqInsertOrder;
    order.Index = field->ClientOrderId;
    order.Price = field->Price;
    order.Volume = field->Volume;
    order.AccountId = field->AccountId;
    order.ExchangeId = field->ExchangeId;
    order.InstrumentId = field->InstrumentId;
    return true;
}

void ExpectAllOrdersIntact(const std::vector<EchoedOrder>& orders)
{
    ASSERT_EQ(orders.size(), static_cast<size_t>(RoundTripFrameCount));
    for (size_t offset = 0; offset < orders.size(); ++offset)
    {
        const int expectedIndex = FirstOrderIndex + static_cast<int>(offset);
        const EchoedOrder& order = orders[offset];
        EXPECT_EQ(order.Index, expectedIndex);
        EXPECT_EQ(order.Volume, expectedIndex);
        EXPECT_NEAR(order.Price, PriceBase + expectedIndex, 1e-6);
        EXPECT_EQ(order.AccountId, ExpectedAccountId);
        EXPECT_EQ(order.ExchangeId, ExpectedExchangeId);
        EXPECT_EQ(order.InstrumentId, ExpectedInstrumentId);
    }
}

class IocpLoopbackServer : public Protocol, public ProtocolSubscriber
{
public:
    IocpLoopbackServer() : Protocol(ProtocolTypeType::Step, ServerTypeType::Server, IoModelType::Iocp, 0, new PackageFactory())
    {
        Subscribe(this);
        RegisterFront(IocpLoopbackAddress);
    }
    virtual ~IocpLoopbackServer() {}

    virtual void OnProtocolConnect(SessionIdType sessionId, const char* ip, int port) override { ++connectCount_; }
    virtual void OnProtocolDisConnect(SessionIdType sessionId, const char* ip, int port) override { ++disConnectCount_; }
    virtual void OnMessage(Package* ownedPackage) override
    {
        EchoedOrder order;
        if (TryExtractEchoedOrder(ownedPackage, order))
        {
            receivedOrders_.Record(std::move(order));
        }
        else
        {
            ++unexpectedFrameCount_;
        }
        Send(ownedPackage);
        ownedPackage->Deallocate();
    }

    int ConnectCount() const { return connectCount_.load(); }
    int DisConnectCount() const { return disConnectCount_.load(); }
    int UnexpectedFrameCount() const { return unexpectedFrameCount_.load(); }
    size_t ReceivedCount() const { return receivedOrders_.Count(); }
    bool WaitForReceivedCount(size_t expectedCount, std::chrono::milliseconds timeout)
    {
        return receivedOrders_.WaitForCount(expectedCount, timeout);
    }
    std::vector<EchoedOrder> ReceivedOrders() const { return receivedOrders_.Snapshot(); }

private:
    std::atomic<int> connectCount_{0};
    std::atomic<int> disConnectCount_{0};
    std::atomic<int> unexpectedFrameCount_{0};
    EchoedOrderRecorder receivedOrders_;
};

class IocpLoopbackClient : public Protocol, public ProtocolSubscriber
{
public:
    IocpLoopbackClient() : Protocol(ProtocolTypeType::Step, ServerTypeType::Client, IoModelType::Iocp, 0, new PackageFactory())
    {
        Subscribe(this);
        RegisterFront(IocpLoopbackAddress);
    }
    virtual ~IocpLoopbackClient() {}

    virtual void OnProtocolConnect(SessionIdType sessionId, const char* ip, int port) override
    {
        ++connectCount_;
        sessionId_ = sessionId;
        SendDistinctOrders();
    }
    virtual void OnProtocolDisConnect(SessionIdType sessionId, const char* ip, int port) override { ++disConnectCount_; }
    virtual void OnMessage(Package* ownedPackage) override
    {
        EchoedOrder order;
        if (TryExtractEchoedOrder(ownedPackage, order))
        {
            receivedOrders_.Record(std::move(order));
        }
        else
        {
            ++unexpectedFrameCount_;
        }
        ownedPackage->Deallocate();
    }

    int ConnectCount() const { return connectCount_.load(); }
    int DisConnectCount() const { return disConnectCount_.load(); }
    int UnexpectedFrameCount() const { return unexpectedFrameCount_.load(); }
    size_t ReceivedCount() const { return receivedOrders_.Count(); }
    bool WaitForReceivedCount(size_t expectedCount, std::chrono::milliseconds timeout)
    {
        return receivedOrders_.WaitForCount(expectedCount, timeout);
    }
    std::vector<EchoedOrder> ReceivedOrders() const { return receivedOrders_.Snapshot(); }

private:
    void SendDistinctOrders()
    {
        for (int index = FirstOrderIndex; index < FirstOrderIndex + RoundTripFrameCount; ++index)
        {
            SendOneOrder(index);
        }
    }
    void SendOneOrder(int index)
    {
        ReqInsertOrderPackage* reqInsertOrder = ReqInsertOrderPackage::Allocate();
        reqInsertOrder->Prepare(sessionId_, false, index);
        reqInsertOrder->ReqInsertOrder = ObjectPool<ReqInsertOrderField>::GetInstance().Allocate();
        Utility::Strcpy(reqInsertOrder->ReqInsertOrder->AccountId, ExpectedAccountId);
        Utility::Strcpy(reqInsertOrder->ReqInsertOrder->ExchangeId, ExpectedExchangeId);
        Utility::Strcpy(reqInsertOrder->ReqInsertOrder->InstrumentId, ExpectedInstrumentId);
        reqInsertOrder->ReqInsertOrder->Direction = DirectionType::Buy;
        reqInsertOrder->ReqInsertOrder->OffsetFlag = OffsetFlagType::Open;
        reqInsertOrder->ReqInsertOrder->OrderPriceType = OrderPriceTypeType::LimitPrice;
        reqInsertOrder->ReqInsertOrder->Price = PriceBase + index;
        reqInsertOrder->ReqInsertOrder->Volume = index;
        reqInsertOrder->ReqInsertOrder->ClientOrderId = index;
        Send(reqInsertOrder);
        reqInsertOrder->Deallocate();
    }

    std::atomic<int> connectCount_{0};
    std::atomic<int> disConnectCount_{0};
    std::atomic<int> unexpectedFrameCount_{0};
    SessionIdType sessionId_{0};
    EchoedOrderRecorder receivedOrders_;
};

class IocpLoopbackThreadGuard
{
public:
    IocpLoopbackThreadGuard(Protocol& serverProtocol, Protocol& clientProtocol)
        : serverThread_("IocpLoopbackServer"), clientThread_("IocpLoopbackClient")
    {
        serverProtocol.SetIoThread(&serverThread_);
        clientProtocol.SetIoThread(&clientThread_);
    }
    ~IocpLoopbackThreadGuard() { StopAndDetachIo(); }

    bool Start() { return serverThread_.Start() && clientThread_.Start(); }
    void StopAndDetachIo()
    {
        clientThread_.Stop();
        serverThread_.Stop();
        clientThread_.Join();
        serverThread_.Join();
        clientThread_.SetIo(nullptr);
        serverThread_.SetIo(nullptr);
    }

private:
    IoThread serverThread_;
    IoThread clientThread_;
};

int CapturedErrorLogCount = 0;

void CountErrorLog(LogLevel level, const char*, int, const char*, const char*, ...)
{
    if (level >= LogLevel::Error)
    {
        ++CapturedErrorLogCount;
    }
}

unsigned long GetCurrentProcessHandleCount()
{
    DWORD handleCount = 0;
    GetProcessHandleCount(GetCurrentProcess(), &handleCount);
    return static_cast<unsigned long>(handleCount);
}

void RunOneClientBuildAndTeardown(const char* address)
{
    std::unique_ptr<IoBase> io(IoFactory::CreateIo(ServerTypeType::Client, address, IoModelType::Iocp, 0));
    EXPECT_TRUE(io->Init());
    io.reset();
}

void RunOneServerBuildAndTeardown(const char* address)
{
    std::unique_ptr<IoBase> io(IoFactory::CreateIo(ServerTypeType::Server, address, IoModelType::Iocp, 0));
    EXPECT_TRUE(io->Init());
    io.reset();
}

struct BuildTeardownMeasurement
{
    long HandleGrowth;
    int ErrorLogCount;
};

BuildTeardownMeasurement MeasureBuildTeardownRounds(int roundCount, const std::function<void()>& runOneRound)
{
    runOneRound();
    CapturedErrorLogCount = 0;
    const unsigned long handleCountBefore = GetCurrentProcessHandleCount();
    for (int round = 0; round < roundCount; ++round)
    {
        runOneRound();
    }
    const unsigned long handleCountAfter = GetCurrentProcessHandleCount();
    return {static_cast<long>(handleCountAfter) - static_cast<long>(handleCountBefore), CapturedErrorLogCount};
}
}

TEST(TcpIocpTest, LoopbackRoundTripKeepsEveryFrameIntactOnASingleConnection)
{
    IocpLoopbackServer server;
    IocpLoopbackClient client;
    IocpLoopbackThreadGuard threads(server, client);

    ASSERT_TRUE(server.Init());
    ASSERT_TRUE(client.Init());
    ASSERT_TRUE(threads.Start());

    const auto roundTripTimeout = std::chrono::seconds(30);
    const bool clientReceivedAll = client.WaitForReceivedCount(RoundTripFrameCount, roundTripTimeout);
    const bool serverReceivedAll = server.WaitForReceivedCount(RoundTripFrameCount, roundTripTimeout);

    const int serverDisConnectCountDuringRoundTrip = server.DisConnectCount();
    const int clientDisConnectCountDuringRoundTrip = client.DisConnectCount();

    threads.StopAndDetachIo();

    EXPECT_TRUE(clientReceivedAll) << "客户端只收到 " << client.ReceivedCount() << " 帧回显";
    EXPECT_TRUE(serverReceivedAll) << "服务端只收到 " << server.ReceivedCount() << " 帧请求";
    EXPECT_EQ(server.ConnectCount(), 1) << "服务端接受了 " << server.ConnectCount() << " 条连接";
    EXPECT_EQ(client.ConnectCount(), 1) << "客户端建立了 " << client.ConnectCount() << " 条连接";
    EXPECT_EQ(serverDisConnectCountDuringRoundTrip, 0);
    EXPECT_EQ(clientDisConnectCountDuringRoundTrip, 0);
    EXPECT_EQ(server.UnexpectedFrameCount(), 0);
    EXPECT_EQ(client.UnexpectedFrameCount(), 0);

    ExpectAllOrdersIntact(server.ReceivedOrders());
    ExpectAllOrdersIntact(client.ReceivedOrders());
}

TEST(TcpIocpTest, RepeatedlyRejectedConnectAttemptsKeepProcessHandleCountFlat)
{
    WriteLogFunc savedWriteLogFunc = Logger::GetWriteLogFunc();
    Logger::SetExternLogger(CountErrorLog);

    const BuildTeardownMeasurement measurement =
        MeasureBuildTeardownRounds(BuildTeardownRoundCount, [] { RunOneClientBuildAndTeardown(RejectedConnectAddress); });

    Logger::SetExternLogger(savedWriteLogFunc);

    EXPECT_EQ(measurement.ErrorLogCount, BuildTeardownRoundCount)
        << BuildTeardownRoundCount << " 次尝试只记到 " << measurement.ErrorLogCount << " 次同步拒绝：失败模式变了，本用例量的已不是同一条路径";
    EXPECT_LE(measurement.HandleGrowth, AllowedHandleGrowth) << BuildTeardownRoundCount << " 次被拒的连接尝试让句柄涨了 " << measurement.HandleGrowth;
}

TEST(TcpIocpTest, AbandonedPendingConnectKeepsProcessHandleCountFlat)
{
    WriteLogFunc savedWriteLogFunc = Logger::GetWriteLogFunc();
    Logger::SetExternLogger(CountErrorLog);

    const BuildTeardownMeasurement measurement =
        MeasureBuildTeardownRounds(BuildTeardownRoundCount, [] { RunOneClientBuildAndTeardown(AbandonedConnectAddress); });

    Logger::SetExternLogger(savedWriteLogFunc);

    EXPECT_EQ(measurement.ErrorLogCount, 0) << "出现 " << measurement.ErrorLogCount
                                            << " 条 Error 级日志：目标地址已被同步拒绝，本用例量的不再是「悬着的请求」";
    EXPECT_LE(measurement.HandleGrowth, AllowedHandleGrowth)
        << BuildTeardownRoundCount << " 轮建/毁让句柄涨了 " << measurement.HandleGrowth << "：ConnectEx 请求没被收回来";
}

TEST(TcpIocpTest, ServerAbandonedPendingAcceptsKeepProcessHandleCountFlat)
{
    const BuildTeardownMeasurement measurement =
        MeasureBuildTeardownRounds(BuildTeardownRoundCount, [] { RunOneServerBuildAndTeardown(ThrowawayListenAddress); });

    EXPECT_EQ(measurement.ErrorLogCount, 0) << "析构路径记到 " << measurement.ErrorLogCount << " 条 Error 级日志";
    EXPECT_LE(measurement.HandleGrowth, AllowedHandleGrowth)
        << BuildTeardownRoundCount << " 轮建/毁让句柄涨了 " << measurement.HandleGrowth << "：AcceptEx 请求没被收回来";
}

#endif // _WIN32
