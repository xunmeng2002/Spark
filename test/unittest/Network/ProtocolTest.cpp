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

    EXPECT_TRUE(recvReturnedToItsCaller);
    EXPECT_EQ(subscriber.MessageCount, 2);
    EXPECT_EQ(subscriber.LastMsgSeqNum, SecondMsgSeqNum);
    EXPECT_EQ(accounting_.CreatedCount, 2);
    EXPECT_EQ(accounting_.DisposedCount, 2);
    EXPECT_EQ(ioProbe_->DisConnectCount, 0);
}

TEST_F(ProtocolRecvTest, APackageParsedWithoutASubscriberIsReturnedToTheFactory)
{
    char frames[MaxPackageSize] = {};
    int frameLen = MakeStepFrame(frames, MaxPackageSize, FirstMsgSeqNum);
    ASSERT_GT(frameLen, 0);

    protocol_->OnRecv(SessionId, frames, static_cast<size_t>(frameLen));

    EXPECT_EQ(accounting_.CreatedCount, 1);
    EXPECT_EQ(accounting_.DisposedCount, 1);
    EXPECT_EQ(ioProbe_->DisConnectCount, 0);
}

TEST_F(ProtocolRecvTest, ARepeatedOnConnectForOneSessionKeepsTheFirstReaderAndReturnsTheSecondToThePool)
{
    PackageReader* recycledReader = PackageReader::Allocate(ProtocolTypeType::Step, &packageFactory_, SessionId, IP);
    recycledReader->Deallocate();

    protocol_->OnConnect(SessionId, IP, 10001);

    PackageReader* nextReader = PackageReader::Allocate(ProtocolTypeType::Step, &packageFactory_, SessionId, IP);
    EXPECT_EQ(nextReader, recycledReader);
    nextReader->Deallocate();
    EXPECT_EQ(protocol_->ReaderOf(SessionId), packageReader_);
}
