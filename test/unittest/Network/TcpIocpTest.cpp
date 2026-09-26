#include <gtest/gtest.h>

#if defined(_WIN32)

#include "PackageFactory.h"
#include "Packages.h"
#include <Spark/Core/Utility/Utility.h>
#include <Spark/Network/Io/IoThread.h>
#include <Spark/Network/Protocol/Protocol.h>
#include <Spark/TemplateLib/ObjectPool/ObjectPool.h>

#include <atomic>
#include <chrono>
#include <condition_variable>
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
// IOCP 的连接建立与收包缓冲回收都独立于其它后端：首连走 TcpIocpClient::PostConnect 而非常规的
// TcpBase::ConnectToServer，收包缓冲是每连接常驻、由 PostRecv 反复复用。故「两端各只建一条连接 /
// 回显逐帧字段完好且顺序不乱 / Stop 后线程能退出」这三件事必须靠真实回环连接单独钉住——
// 缓冲被提前归还或跨线程覆写时，现象是字段串、条数缺，不会抛异常。
constexpr char kIocpLoopbackAddress[] = "tcp://127.0.0.1:20011";
constexpr int kRoundTripFrameCount = 200;
constexpr int kFirstOrderIndex = 1;
constexpr int kPriceBase = 100;
constexpr char kAccountId[] = "Xunmeng001";
constexpr char kExchangeId[] = "SHSE";
constexpr char kInstrumentId[] = "600036";

struct EchoedOrder
{
    int Index;
    double Price;
    long long Volume;
    std::string AccountId;
    std::string ExchangeId;
    std::string InstrumentId;
};

// 两端回调都跑在各自的 IO 线程上，记录表按锁归属；等待用条件变量，避免固定 sleep 拖长用例
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

// 只有字段完好且已填充的 ReqInsertOrderPackage 才可能被读出，取不到即解析路径出问题
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
    ASSERT_EQ(orders.size(), static_cast<size_t>(kRoundTripFrameCount));
    for (size_t offset = 0; offset < orders.size(); ++offset)
    {
        const int expectedIndex = kFirstOrderIndex + static_cast<int>(offset);
        const EchoedOrder& order = orders[offset];
        EXPECT_EQ(order.Index, expectedIndex);
        EXPECT_EQ(order.Volume, expectedIndex);
        EXPECT_NEAR(order.Price, kPriceBase + expectedIndex, 1e-6);
        EXPECT_EQ(order.AccountId, kAccountId);
        EXPECT_EQ(order.ExchangeId, kExchangeId);
        EXPECT_EQ(order.InstrumentId, kInstrumentId);
    }
}

class IocpLoopbackServer : public Protocol, public ProtocolSubscriber
{
public:
    IocpLoopbackServer() : Protocol(ProtocolTypeType::Step, ServerTypeType::Server, IoModelType::Iocp, 0, new PackageFactory())
    {
        Subscribe(this);
        RegisterFront(kIocpLoopbackAddress);
    }
    virtual ~IocpLoopbackServer() {}

    virtual void OnProtocolConnect(SessionIdType sessionId, const char* ip, int port) override
    {
        ++connectCount_;
    }
    virtual void OnProtocolDisConnect(SessionIdType sessionId, const char* ip, int port) override
    {
        ++disConnectCount_;
    }
    // 逐帧原样回显：客户端据此收到自己发出的每一帧，从而证明缓冲在往返途中未被覆写
    virtual void OnMessage(Package* package) override
    {
        EchoedOrder order;
        if (TryExtractEchoedOrder(package, order))
        {
            receivedOrders_.Record(std::move(order));
        }
        else
        {
            ++unexpectedFrameCount_;
        }
        Send(package);
        package->Deallocate();
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
        RegisterFront(kIocpLoopbackAddress);
    }
    virtual ~IocpLoopbackClient() {}

    virtual void OnProtocolConnect(SessionIdType sessionId, const char* ip, int port) override
    {
        ++connectCount_;
        sessionId_ = sessionId;
        SendDistinctOrders();
    }
    virtual void OnProtocolDisConnect(SessionIdType sessionId, const char* ip, int port) override
    {
        ++disConnectCount_;
    }
    virtual void OnMessage(Package* package) override
    {
        EchoedOrder order;
        if (TryExtractEchoedOrder(package, order))
        {
            receivedOrders_.Record(std::move(order));
        }
        else
        {
            ++unexpectedFrameCount_;
        }
        package->Deallocate();
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
        for (int index = kFirstOrderIndex; index < kFirstOrderIndex + kRoundTripFrameCount; ++index)
        {
            SendOneOrder(index);
        }
    }
    // 包与字段都按次领取/归还：常驻成员在归还后再用会把同一指针重复推入空闲链
    void SendOneOrder(int index)
    {
        ReqInsertOrderPackage* reqInsertOrder = ReqInsertOrderPackage::Allocate();
        reqInsertOrder->Prepare(sessionId_, false, index);
        reqInsertOrder->ReqInsertOrder = ObjectPool<ReqInsertOrderField>::GetInstance().Allocate();
        Utility::Strcpy(reqInsertOrder->ReqInsertOrder->AccountId, kAccountId);
        Utility::Strcpy(reqInsertOrder->ReqInsertOrder->ExchangeId, kExchangeId);
        Utility::Strcpy(reqInsertOrder->ReqInsertOrder->InstrumentId, kInstrumentId);
        reqInsertOrder->ReqInsertOrder->Direction = DirectionType::Buy;
        reqInsertOrder->ReqInsertOrder->OffsetFlag = OffsetFlagType::Open;
        reqInsertOrder->ReqInsertOrder->OrderPriceType = OrderPriceTypeType::LimitPrice;
        reqInsertOrder->ReqInsertOrder->Price = kPriceBase + index;
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

// IoThread 与 Protocol 都会删同一个 IoBase：本用例里两者同生命周期，故由本守卫统一收尾——
// 先停并 join 线程（ThreadExit 里还要用 io_），再摘掉线程手里的指针，最后才轮到 Protocol 析构删除
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
    // 收尾顺序不可颠倒：线程退出路径要用 io_，故先停并 join，再摘指针；重复调用无副作用
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
}

// ============================================================
// IOCP 回环收发测试
// 服务端逐帧回显，客户端发 kRoundTripFrameCount 帧互不相同的委托单
// 断言：两端各只建一条连接、回显逐帧字段完好且顺序不乱、收尾能干净退出
// ============================================================

TEST(TcpIocpTest, LoopbackRoundTripKeepsEveryFrameIntactOnASingleConnection)
{
    IocpLoopbackServer server;
    IocpLoopbackClient client;
    IocpLoopbackThreadGuard threads(server, client);

    ASSERT_TRUE(server.Init());
    ASSERT_TRUE(client.Init());
    ASSERT_TRUE(threads.Start());

    const auto roundTripTimeout = std::chrono::seconds(30);
    const bool clientReceivedAll = client.WaitForReceivedCount(kRoundTripFrameCount, roundTripTimeout);
    const bool serverReceivedAll = server.WaitForReceivedCount(kRoundTripFrameCount, roundTripTimeout);

    // 停线程时 ThreadExit → DisConnectAll 必然各产生一次断连，故断连只统计到这一刻为止：
    // 往返全程一次断连都没有，才说明连接没被打断过
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

#endif // _WIN32
