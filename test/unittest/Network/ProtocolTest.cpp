#include "PackageFactory.h"
#include "Packages.h"
#include <Spark/Network/Protocol/PackageReader.h>
#include <Spark/Network/Protocol/Protocol.h>
#include <Spark/TemplateLib/Buffer/LinearBuffer.h>
#include <gtest/gtest.h>

#include <memory>

using namespace Spark;
using namespace Spark::Network;
using namespace Spark::Packages;

// ============================================================
// Protocol 取包循环：取出的包必须有个去处
// 交出去的归订阅者，没交出去的（订阅者为空）必须由 Protocol 自己归还，不能随指针出作用域丢掉
// ============================================================

namespace
{
constexpr SessionIdType SessionId = 42;
constexpr const char* IP = "192.168.1.100";
constexpr int FirstMsgSeqNum = 1001;

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
