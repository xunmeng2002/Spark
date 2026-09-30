#include "PackageFactory.h"
#include "Packages.h"
#include <Spark/Network/Protocol/PackageReader.h>
#include <Spark/Network/Protocol/Protocol.h>
#include <Spark/TemplateLib/Buffer/LinearBuffer.h>
#include <gtest/gtest.h>

#include <memory>
#include <stdexcept>

using namespace Spark;
using namespace Spark::Network;
using namespace Spark::Packages;

// ============================================================
// Protocol 取包循环：取出的包必须有个去处，订阅者抛出不许打断这一批
// 交出去的归订阅者（含抛出时），没交出去的（订阅者为空）必须由 Protocol 自己归还
// 契约见 docs/io-subscriber-notification-contract.md：IO 层的兜底只保证异常不穿出 IO 层，
// 而 Protocol 自身就是 IoSubscriber，它在这一层的兜底由本文件钉住
// ============================================================

namespace
{
constexpr SessionIdType SessionId = 42;
constexpr const char* IP = "192.168.1.100";
constexpr int FirstMsgSeqNum = 1001;
constexpr int SecondMsgSeqNum = 1002;

class PackageAccounting
{
public:
    int CreatedCount = 0;
    int DisposedCount = 0;
};

// 记名的包：工厂交出去几个、Deallocate 收回来几个，两数一比就能看出取包循环漏掉了哪些
class LedgeredPackage : public Package
{
public:
    explicit LedgeredPackage(PackageAccounting* accounting) : accounting_(accounting) {}
    void Deallocate() override
    {
        ++accounting_->DisposedCount;
        delete this;
    }
    int ToStepStream(char*, int) const override { return 0; }
    bool FromStepStream(char*, int, int) override { return true; }
    int ToXtpStream(char*, int) const override { return 0; }
    bool FromXtpStream(char*, int, int) override { return true; }
    const char* GetDebugString() const override { return "LedgeredPackage"; }

private:
    PackageAccounting* accounting_;
};

class PackageAccountingFactory : public PackageFactoryBase
{
public:
    explicit PackageAccountingFactory(PackageAccounting* accounting) : accounting_(accounting) {}
    Package* CreatePackage(UInt16Type) override
    {
        ++accounting_->CreatedCount;
        return new LedgeredPackage(accounting_);
    }

private:
    PackageAccounting* accounting_;
};

// 只用来当 Protocol 的 ioBase_：取包循环回不到这里，除了 ParsePackage 报错时的断链
class RecvProbeIo : public IoBase
{
public:
    RecvProbeIo() : IoBase(ServerTypeType::Server, "tcp://127.0.0.1:10001", 1000) {}

    void Send(SessionIdType, Spark::LinearBuffer<BufferSize>*) override {}
    void HandleIoEvent() override {}
    void DisConnect(SessionIdType) override { ++DisConnectCount; }

    int DisConnectCount = 0;

protected:
    void DoSend(Connect*) override {}
    void DoRecv(Connect*) override {}
};

class ThrowingMessageProbe : public ProtocolSubscriber
{
public:
    void OnProtocolDisConnect(SessionIdType, const char*, int) override {}
    void OnMessage(Package* ownedPackage) override
    {
        ++MessageCount;
        LastMsgSeqNum = ownedPackage->Head.MsgSeqNum;
        // 形参即所有权：先归还再抛，与只管本包、不管后事的真实订阅者一致
        ownedPackage->Deallocate();
        if (MessageCount == ThrowingMessageIndex)
        {
            throw std::runtime_error("Probe OnMessage Failure.");
        }
    }

    int ThrowingMessageIndex = 0;
    int MessageCount = 0;
    int LastMsgSeqNum = 0;
};

// 把 Protocol 的受保护面揭给用例：ioBase_ 由用例装配、会话的 PackageReader 由用例插入，
// 取包循环因此可以在不挂共享内存、不依赖时序的情况下被逐帧驱动
class ProtocolProbe : public Protocol
{
public:
    explicit ProtocolProbe(PackageFactoryBase* packageFactory)
        : Protocol(ProtocolTypeType::Step, ServerTypeType::Server, IoModelType::Select, 1000, packageFactory)
    {
    }
    void AttachIo(IoBase* ioBase) { ioBase_ = ioBase; }
    void AttachReader(SessionIdType sessionId, PackageReader* packageReader) { sessionPackageReaders_.insert({sessionId, packageReader}); }
    PackageReader* ReaderOf(SessionIdType sessionId)
    {
        auto it = sessionPackageReaders_.find(sessionId);
        return it == sessionPackageReaders_.end() ? nullptr : it->second;
    }
};

// 一条完整 Step 报文的字节数；buff 的容量由调用方给足
int MakeStepFrame(char* buff, int capacity, int msgSeqNum)
{
    auto* package = NotifyComponentConnectStatusPackage::Allocate();
    package->Prepare(SessionId, 0, msgSeqNum);
    int totalLen = package->MakePackage(ProtocolTypeType::Step, buff, capacity);
    package->Deallocate();
    return totalLen;
}

class ProtocolRecvTest : public testing::Test
{
protected:
    void SetUp() override
    {
        auto ioProbe = std::make_unique<RecvProbeIo>();
        ioProbe_ = ioProbe.get();
        protocol_ = std::make_unique<ProtocolProbe>(&packageFactory_);
        protocol_->AttachIo(ioProbe.release());
        packageReader_ = PackageReader::Allocate(ProtocolTypeType::Step, &packageFactory_, SessionId, IP);
        protocol_->AttachReader(SessionId, packageReader_);
    }

    void TearDown() override
    {
        packageReader_->Deallocate();
        // ~Protocol 会 delete ioBase_，ioProbe_ 的计数须在此之前的断言里取完
        protocol_.reset();
    }

    PackageAccounting accounting_;
    PackageAccountingFactory packageFactory_{&accounting_};
    RecvProbeIo* ioProbe_ = nullptr;
    std::unique_ptr<ProtocolProbe> protocol_;
    PackageReader* packageReader_ = nullptr;
};
}

TEST_F(ProtocolRecvTest, AThrowingOnMessageNeitherEscapesTheRecvLoopNorStrandsTheRestOfTheBatch)
{
    ThrowingMessageProbe subscriber;
    subscriber.ThrowingMessageIndex = 1;
    protocol_->Subscribe(&subscriber);

    char frames[MaxPackageSize] = {};
    int firstLen = MakeStepFrame(frames, MaxPackageSize, FirstMsgSeqNum);
    ASSERT_GT(firstLen, 0);
    int secondLen = MakeStepFrame(frames + firstLen, MaxPackageSize - firstLen, SecondMsgSeqNum);
    ASSERT_GT(secondLen, 0);
    int totalLen = firstLen + secondLen;
    ASSERT_LT(totalLen, MaxPackageSize);

    bool recvReturnedToItsCaller = false;
    protocol_->OnRecv(SessionId, frames, static_cast<size_t>(totalLen));
    recvReturnedToItsCaller = true;

    // 第一帧抛出后，同一段字节里的第二帧仍要在这一次 OnRecv 里派发出去，不能滞留在 reader 里等下一批流量
    EXPECT_TRUE(recvReturnedToItsCaller);
    EXPECT_EQ(subscriber.MessageCount, 2);
    EXPECT_EQ(subscriber.LastMsgSeqNum, SecondMsgSeqNum);
    EXPECT_EQ(accounting_.CreatedCount, 2);
    // 抛出时 Protocol 不得替订阅者归还：形参是所有权移交，再由它归还就是把同一指针二次入池
    EXPECT_EQ(accounting_.DisposedCount, 2);
    EXPECT_EQ(ioProbe_->DisConnectCount, 0);
}

TEST_F(ProtocolRecvTest, APackageParsedWithoutASubscriberIsReturnedToTheFactory)
{
    char frames[MaxPackageSize] = {};
    int frameLen = MakeStepFrame(frames, MaxPackageSize, FirstMsgSeqNum);
    ASSERT_GT(frameLen, 0);

    protocol_->OnRecv(SessionId, frames, static_cast<size_t>(frameLen));

    // 没交出去的必须归还：订阅者为空时这一帧谁也没拿到
    EXPECT_EQ(accounting_.CreatedCount, 1);
    EXPECT_EQ(accounting_.DisposedCount, 1);
    EXPECT_EQ(ioProbe_->DisConnectCount, 0);
}

TEST_F(ProtocolRecvTest, ARepeatedOnConnectForOneSessionKeepsTheFirstReaderAndReturnsTheSecondToThePool)
{
    PackageReader* recycledReader = PackageReader::Allocate(ProtocolTypeType::Step, &packageFactory_, SessionId, IP);
    recycledReader->Deallocate();

    protocol_->OnConnect(SessionId, IP, 10001);

    // 还回去了才会再取到同一个槽：没还回去，这一次取到的就是池里的下一个槽
    PackageReader* nextReader = PackageReader::Allocate(ProtocolTypeType::Step, &packageFactory_, SessionId, IP);
    EXPECT_EQ(nextReader, recycledReader);
    nextReader->Deallocate();
    // 在册的仍是先来的那个 reader：重复的会话号不得把表项换掉
    EXPECT_EQ(protocol_->ReaderOf(SessionId), packageReader_);
}
