#include "PackageFactory.h"
#include "Packages.h"
#include <Spark/Network/Protocol/PackageReader.h>
#include <Spark/Network/Protocol/Protocol.h>
#include <Spark/TemplateLib/Buffer/LinearBuffer.h>
#include "doctest/doctest.h"

#include <memory>
#include <stdexcept>

using namespace Spark;
using namespace Spark::Network;
using namespace Spark::Test;
using namespace Spark::Test::Packages;

namespace
{
constexpr Spark::SessionIdType SessionId = 42;
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

class OversizedBodyPackage : public Package
{
public:
    void Deallocate() override { delete this; }
    int ToStepStream(char*, int) const override { return static_cast<int>(MaxFrameBodyLen) + 1; }
    bool FromStepStream(char*, int, int) override { return true; }
    int ToXtpStream(char*, int) const override { return 0; }
    bool FromXtpStream(char*, int, int) override { return true; }
    const char* GetDebugString() const override { return "OversizedBodyPackage"; }
};

class PackageAccountingFactory : public PackageFactoryBase
{
public:
    explicit PackageAccountingFactory(PackageAccounting* accounting) : accounting_(accounting) {}
    Package* CreatePackage(Spark::UInt16Type) override
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

    void Send(Spark::SessionIdType, Spark::LinearBuffer<BufferSize>*) override { ++SendCount; }
    void HandleIoEvent() override {}
    void DisConnect(Spark::SessionIdType) override { ++DisConnectCount; }

    int DisConnectCount = 0;
    int SendCount = 0;

protected:
    void DoSend(Connect*) override {}
    void DoRecv(Connect*) override {}
};

class ThrowingMessageProbe : public ProtocolSubscriber
{
public:
    void OnProtocolDisConnect(Spark::SessionIdType, const char*, int) override {}
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
    void AttachReader(Spark::SessionIdType sessionId, PackageReader* packageReader) { sessionPackageReaders_.insert({sessionId, packageReader}); }
    PackageReader* ReaderOf(Spark::SessionIdType sessionId)
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

class ProtocolRecvTest
{
protected:
    ProtocolRecvTest()
    {
        auto ioProbe = std::make_unique<RecvProbeIo>();
        ioProbe_ = ioProbe.get();
        protocol_ = std::make_unique<ProtocolProbe>(&packageFactory_);
        protocol_->AttachIo(ioProbe.release());
        packageReader_ = PackageReader::Allocate(ProtocolTypeType::Step, &packageFactory_, SessionId, IP);
        protocol_->AttachReader(SessionId, packageReader_);
    }

    ~ProtocolRecvTest()
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

TEST_CASE_FIXTURE(ProtocolRecvTest, "ProtocolRecvTest.AThrowingOnMessageNeitherEscapesTheRecvLoopNorStrandsTheRestOfTheBatch")
{
    ThrowingMessageProbe subscriber;
    subscriber.ThrowingMessageIndex = 1;
    protocol_->Subscribe(&subscriber);

    char frames[MaxPackageSize] = {};
    int firstLen = MakeStepFrame(frames, MaxPackageSize, FirstMsgSeqNum);
    REQUIRE_GT(firstLen, 0);
    int secondLen = MakeStepFrame(frames + firstLen, MaxPackageSize - firstLen, SecondMsgSeqNum);
    REQUIRE_GT(secondLen, 0);
    int totalLen = firstLen + secondLen;
    REQUIRE_LT(totalLen, MaxPackageSize);

    bool recvReturnedToItsCaller = false;
    protocol_->OnRecv(SessionId, frames, static_cast<size_t>(totalLen));
    recvReturnedToItsCaller = true;

    CHECK(recvReturnedToItsCaller);
    CHECK_EQ(subscriber.MessageCount, 2);
    CHECK_EQ(subscriber.LastMsgSeqNum, SecondMsgSeqNum);
    CHECK_EQ(accounting_.CreatedCount, 2);
    CHECK_EQ(accounting_.DisposedCount, 2);
    CHECK_EQ(ioProbe_->DisConnectCount, 0);
}

TEST_CASE_FIXTURE(ProtocolRecvTest, "ProtocolRecvTest.APackageParsedWithoutASubscriberIsReturnedToTheFactory")
{
    char frames[MaxPackageSize] = {};
    int frameLen = MakeStepFrame(frames, MaxPackageSize, FirstMsgSeqNum);
    REQUIRE_GT(frameLen, 0);

    protocol_->OnRecv(SessionId, frames, static_cast<size_t>(frameLen));

    CHECK_EQ(accounting_.CreatedCount, 1);
    CHECK_EQ(accounting_.DisposedCount, 1);
    CHECK_EQ(ioProbe_->DisConnectCount, 0);
}

TEST_CASE_FIXTURE(ProtocolRecvTest, "ProtocolRecvTest.AFrameThatCannotBeBuiltReportsFailureAndIsNeverHandedToTheTransport")
{
    OversizedBodyPackage oversized;
    oversized.SessionId = SessionId;

    CHECK_FALSE(protocol_->Send(&oversized));
    CHECK_EQ(ioProbe_->SendCount, 0);
}

TEST_CASE_FIXTURE(ProtocolRecvTest, "ProtocolRecvTest.AFrameThatCanBeBuiltReportsSuccessAndIsHandedToTheTransportOnce")
{
    auto* package = NotifyComponentConnectStatusPackage::Allocate();
    package->Prepare(SessionId, 0, FirstMsgSeqNum);

    CHECK(protocol_->Send(package));
    CHECK_EQ(ioProbe_->SendCount, 1);

    package->Deallocate();
}

TEST_CASE_FIXTURE(ProtocolRecvTest, "ProtocolRecvTest.ARepeatedOnConnectForOneSessionKeepsTheFirstReaderAndReturnsTheSecondToThePool")
{
    PackageReader* recycledReader = PackageReader::Allocate(ProtocolTypeType::Step, &packageFactory_, SessionId, IP);
    recycledReader->Deallocate();

    protocol_->OnConnect(SessionId, IP, 10001);

    PackageReader* nextReader = PackageReader::Allocate(ProtocolTypeType::Step, &packageFactory_, SessionId, IP);
    CHECK_EQ(nextReader, recycledReader);
    nextReader->Deallocate();
    CHECK_EQ(protocol_->ReaderOf(SessionId), packageReader_);
}
