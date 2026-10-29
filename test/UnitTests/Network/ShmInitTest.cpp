#include <Spark/Core/Logger/Logger.h>
#include <Spark/Network/Io/IoBase.h>
#include <Spark/Network/Io/IoFactory.h>
#include <Spark/TemplateLib/Buffer/ShmBuffer.h>
#include "doctest/doctest.h"

#include <atomic>
#include <chrono>
#include <cstring>
#include <format>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <thread>

#ifdef _WIN32
#include <Windows.h>
#else
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#endif
using namespace Spark;
using namespace Spark::Network;

namespace
{
std::string MakeUniqueShmObjectName(const char* namePrefix)
{
    const auto ticks = std::chrono::steady_clock::now().time_since_epoch().count();
    return std::format("{}{}", namePrefix, ticks);
}

std::string MakeUniqueShmName(const char* namePrefix)
{
    return "shm://" + MakeUniqueShmObjectName(namePrefix);
}

std::string ToShmAddress(const std::string& shmObjectName, const char* connectSizeText)
{
    return std::format("shm://{}:{}", shmObjectName, connectSizeText);
}

std::string MakeUniqueShmAddress(const char* namePrefix, const char* connectSizeText)
{
    return MakeUniqueShmName(namePrefix) + ":" + connectSizeText;
}

std::unique_ptr<IoBase> CreateShmServer(const std::string& address)
{
    return std::unique_ptr<IoBase>(IoFactory::CreateIo(ServerTypeType::Server, address.c_str()));
}

std::unique_ptr<IoBase> CreateShmClient(const std::string& address)
{
    return std::unique_ptr<IoBase>(IoFactory::CreateIo(ServerTypeType::Client, address.c_str()));
}

#ifndef _WIN32
std::string BuildPosixShmObjectName(const std::string& shmObjectName)
{
    return "/" + shmObjectName;
}
#endif

template <typename Visitor>
bool VisitShmMapping(const std::string& shmObjectName, size_t viewSize, Visitor&& visitor)
{
#ifdef _WIN32
    HANDLE fileMapping = OpenFileMappingA(FILE_MAP_ALL_ACCESS, FALSE, shmObjectName.c_str());
    if (fileMapping == nullptr)
    {
        return false;
    }
    void* mappingView = MapViewOfFile(fileMapping, FILE_MAP_ALL_ACCESS, 0, 0, viewSize);
    if (mappingView == nullptr)
    {
        CloseHandle(fileMapping);
        return false;
    }
    visitor(static_cast<Spark::SingleShmHeader*>(mappingView));
    UnmapViewOfFile(mappingView);
    CloseHandle(fileMapping);
    return true;
#else
    const int fileDescriptor = shm_open(BuildPosixShmObjectName(shmObjectName).c_str(), O_RDWR, 0666);
    if (fileDescriptor < 0)
    {
        return false;
    }
    void* mappingView = mmap(nullptr, viewSize, PROT_READ | PROT_WRITE, MAP_SHARED, fileDescriptor, 0);
    close(fileDescriptor);
    if (mappingView == MAP_FAILED)
    {
        return false;
    }
    visitor(static_cast<Spark::SingleShmHeader*>(mappingView));
    munmap(mappingView, viewSize);
    return true;
#endif
}

bool WriteShmMappingMagic(const std::string& shmObjectName, size_t magic)
{
    return VisitShmMapping(shmObjectName, sizeof(Spark::SingleShmHeader),
                           [magic](Spark::SingleShmHeader* shmHeader) { Spark::SingleShmHeader::StoreMappedField(shmHeader->MappingMagic, magic); });
}

bool WriteShmMappingLayoutVersion(const std::string& shmObjectName, size_t layoutVersion)
{
    return VisitShmMapping(shmObjectName, sizeof(Spark::SingleShmHeader), [layoutVersion](Spark::SingleShmHeader* shmHeader)
                           { Spark::SingleShmHeader::StoreMappedField(shmHeader->MappingLayoutVersion, layoutVersion); });
}

bool WriteShmHeaderStatus(const std::string& shmObjectName, unsigned connectionIndex, ConnectStatusType status)
{
    return VisitShmMapping(shmObjectName, sizeof(Spark::SingleShmHeader) * (connectionIndex + 1),
                           [connectionIndex, status](Spark::SingleShmHeader* shmHeader)
                           { Spark::SingleShmHeader::StoreStatus(shmHeader + connectionIndex, status); });
}

ConnectStatusType ReadShmHeaderStatus(const std::string& shmObjectName, unsigned connectionIndex)
{
    ConnectStatusType status = ConnectStatusType::UnConnected;
    VisitShmMapping(shmObjectName, sizeof(Spark::SingleShmHeader) * (connectionIndex + 1),
                    [connectionIndex, &status](Spark::SingleShmHeader* shmHeader)
                    { status = Spark::SingleShmHeader::LoadStatus(shmHeader + connectionIndex); });
    return status;
}

struct ShmChannelDownCounters
{
    size_t WriteCount = 0;
    size_t ReadCount = 0;
};

bool ReadShmChannelDownCounters(const std::string& shmObjectName, unsigned connectionIndex, ShmChannelDownCounters& counters)
{
    return VisitShmMapping(shmObjectName, sizeof(Spark::SingleShmHeader) * (connectionIndex + 1),
                           [connectionIndex, &counters](Spark::SingleShmHeader* shmHeader)
                           {
                               Spark::SingleShmHeader* channelHeader = shmHeader + connectionIndex;
                               counters.WriteCount = Spark::SingleShmHeader::LoadMappedField(channelHeader->DownWriteCount);
                               counters.ReadCount = Spark::SingleShmHeader::LoadMappedField(channelHeader->DownReadCount);
                           });
}

bool ShmObjectExists(const std::string& shmObjectName)
{
    return VisitShmMapping(shmObjectName, sizeof(Spark::SingleShmHeader), [](Spark::SingleShmHeader*) {});
}

class ConnectEventProbe : public IoSubscriber
{
public:
    void OnConnect(SessionIdType sessionId, const char*, int) override
    {
        ++ConnectCount;
        PeerSessionId = sessionId;
    }
    void OnDisConnect(SessionIdType, const char*, int) override { ++DisConnectCount; }
    void OnRecv(SessionIdType, const char*, size_t) override {}

    int ConnectCount = 0;
    int DisConnectCount = 0;
    SessionIdType PeerSessionId = 0;
};

constexpr auto WaitPollInterval = std::chrono::milliseconds(50);

template <typename StepOnce, typename StopCondition>
[[nodiscard]] bool WaitUntil(StepOnce&& stepOnce, StopCondition&& stopCondition, std::chrono::milliseconds limit)
{
    const auto deadline = std::chrono::steady_clock::now() + limit;
    while (!stopCondition() && std::chrono::steady_clock::now() < deadline)
    {
        stepOnce();
        std::this_thread::sleep_for(WaitPollInterval);
    }
    return stopCondition();
}

template <typename StopCondition>
[[nodiscard]] bool DriveIoEventsUntil(IoBase& io, StopCondition&& stopCondition, std::chrono::milliseconds limit)
{
    return WaitUntil([&io] { io.HandleIoEvent(); }, stopCondition, limit);
}

class ThrowingRecvProbe : public ConnectEventProbe
{
public:
    void OnRecv(SessionIdType, const char*, size_t length) override
    {
        ++RecvCount;
        LastRecvLength = length;
        if (RecvCount <= ThrowingRecvCount)
        {
            throw std::runtime_error("Probe OnRecv Failure.");
        }
    }

    int ThrowingRecvCount = 0;
    int RecvCount = 0;
    size_t LastRecvLength = 0;
};

class PayloadCollectingProbe : public ConnectEventProbe
{
public:
    void OnRecv(SessionIdType, const char* data, size_t length) override { ReceivedPayload.append(data, length); }

    std::string ReceivedPayload;
};

constexpr const char* DriveBudgetExhaustedMessage = "驱动预算耗尽：到点停条件仍不成立，与断言的状态不符不是一回事";

class ExternLogCapture
{
public:
    explicit ExternLogCapture(const char* logSignature) : logSignature_(logSignature)
    {
        ActiveCapture = this;
        savedWriteLogFunc_ = Spark::Core::Logger::GetWriteLogFunc();
        Spark::Core::Logger::SetExternLogger(CaptureMatchingLog);
    }
    ~ExternLogCapture()
    {
        Spark::Core::Logger::SetExternLogger(savedWriteLogFunc_);
        ActiveCapture = nullptr;
    }
    ExternLogCapture(const ExternLogCapture&) = delete;
    ExternLogCapture& operator=(const ExternLogCapture&) = delete;

    int MatchCount() const { return matchCount_.load(); }

private:
    static void CaptureMatchingLog(Spark::Core::LogLevel, const char*, int, const char*, const char* formatStr, ...)
    {
        if (ActiveCapture != nullptr && std::strstr(formatStr, ActiveCapture->logSignature_) != nullptr)
        {
            ++ActiveCapture->matchCount_;
        }
    }

    static ExternLogCapture* ActiveCapture;

    const char* logSignature_;
    Spark::Core::WriteLogFunc savedWriteLogFunc_;
    std::atomic<int> matchCount_{0};
};

ExternLogCapture* ExternLogCapture::ActiveCapture = nullptr;
}

// ============================================================
// Shm 后端 Init 门禁测试
// 地址末段即连接数（ShmBase 用 StepUtility::ParseInteger 解析），非法值须在 Init 内被拒
// ============================================================

TEST_CASE("ShmInitTest.Init_RejectsNonNumericConnectSize")
{
    const auto io = CreateShmServer(MakeUniqueShmAddress("SparkShmUnitTestRejectText", "abc"));

    REQUIRE_NE(io, nullptr);
    CHECK_FALSE(io->Init());
}

TEST_CASE("ShmInitTest.Init_RejectsZeroConnectSize")
{
    const auto io = CreateShmServer(MakeUniqueShmAddress("SparkShmUnitTestRejectZero", "0"));

    REQUIRE_NE(io, nullptr);
    CHECK_FALSE(io->Init());
}

TEST_CASE("ShmInitTest.Init_AcceptsSmallestPositiveConnectSize")
{
    const auto io = CreateShmServer(MakeUniqueShmAddress("SparkShmUnitTestAccept", "1"));

    REQUIRE_NE(io, nullptr);
    CHECK(io->Init());
}

TEST_CASE("ShmInitTest.Init_RejectsConnectSizeWhoseMappingWouldWrap")
{
    const auto io = CreateShmServer(MakeUniqueShmAddress("SparkShmUnitTestWrap", "2049"));

    REQUIRE_NE(io, nullptr);
    CHECK_FALSE(io->Init());
}

TEST_CASE("ShmInitTest.Init_ReusesShmObjectLeftByAPreviousRun")
{
    const auto shmAddress = MakeUniqueShmAddress("SparkShmUnitTestReuse", "1");
    const auto leftoverOwner = CreateShmServer(shmAddress);
    REQUIRE_NE(leftoverOwner, nullptr);
    REQUIRE(leftoverOwner->Init());

    const auto secondOwner = CreateShmServer(shmAddress);
    REQUIRE_NE(secondOwner, nullptr);
    CHECK(secondOwner->Init());
}

TEST_CASE("ShmInitTest.Init_RejectsReusedShmObjectSmallerThanNeeded")
{
    const auto shmName = MakeUniqueShmName("SparkShmUnitTestReuseSmall");
    const auto smallerOwner = CreateShmServer(shmName + ":1");
    REQUIRE_NE(smallerOwner, nullptr);
    REQUIRE(smallerOwner->Init());

    const auto largerOwner = CreateShmServer(shmName + ":2");
    REQUIRE_NE(largerOwner, nullptr);
    CHECK_FALSE(largerOwner->Init());
}

TEST_CASE("ShmInitTest.Init_RejectsReusedShmObjectWithForeignMappingMagic")
{
    const auto shmObjectName = MakeUniqueShmObjectName("SparkShmUnitTestForeignMagic");
    const auto shmAddress = ToShmAddress(shmObjectName, "1");
    const auto leftoverOwner = CreateShmServer(shmAddress);
    REQUIRE_NE(leftoverOwner, nullptr);
    REQUIRE(leftoverOwner->Init());
    REQUIRE(WriteShmMappingMagic(shmObjectName, 0xDEADBEEF));

    const auto secondOwner = CreateShmServer(shmAddress);
    REQUIRE_NE(secondOwner, nullptr);
    CHECK_FALSE(secondOwner->Init());
}

TEST_CASE("ShmInitTest.Init_RejectsReusedShmObjectWithForeignMappingLayoutVersion")
{
    const auto shmObjectName = MakeUniqueShmObjectName("SparkShmUnitTestForeignLayoutVersion");
    const auto shmAddress = ToShmAddress(shmObjectName, "1");
    const auto leftoverOwner = CreateShmServer(shmAddress);
    REQUIRE_NE(leftoverOwner, nullptr);
    REQUIRE(leftoverOwner->Init());
    REQUIRE(WriteShmMappingLayoutVersion(shmObjectName, Spark::ShmMappingLayoutVersion + 1));

    {
        const auto secondOwner = CreateShmServer(shmAddress);
        REQUIRE_NE(secondOwner, nullptr);
        CHECK_FALSE(secondOwner->Init());
    }
    CHECK(ShmObjectExists(shmObjectName));
}

TEST_CASE("ShmInitTest.Init_LeavesTheObjectAloneWhenTheConnectSizeIsRejected")
{
    const auto shmObjectName = MakeUniqueShmObjectName("SparkShmUnitTestGateRejectedOwner");
    const auto owner = CreateShmServer(ToShmAddress(shmObjectName, "1"));
    REQUIRE_NE(owner, nullptr);
    REQUIRE(owner->Init());

    {
        const auto rejectedOwner = CreateShmServer(ToShmAddress(shmObjectName, "abc"));
        REQUIRE_NE(rejectedOwner, nullptr);
        CHECK_FALSE(rejectedOwner->Init());
    }
    CHECK(ShmObjectExists(shmObjectName));
}

TEST_CASE("ShmInitTest.Init_AcceptsStampedShmObjectForAClient")
{
    const auto shmAddress = ToShmAddress(MakeUniqueShmObjectName("SparkShmUnitTestClientStamp"), "1");
    const auto shmOwner = CreateShmServer(shmAddress);
    REQUIRE_NE(shmOwner, nullptr);
    REQUIRE(shmOwner->Init());

    const auto client = CreateShmClient(shmAddress);
    REQUIRE_NE(client, nullptr);
    CHECK(client->Init());
}

TEST_CASE("ShmInitTest.Init_RejectsUnstampedShmObjectForAClient")
{
    const auto shmObjectName = MakeUniqueShmObjectName("SparkShmUnitTestUnstamped");
    const auto shmAddress = ToShmAddress(shmObjectName, "1");
    const auto shmOwner = CreateShmServer(shmAddress);
    REQUIRE_NE(shmOwner, nullptr);
    REQUIRE(shmOwner->Init());
    REQUIRE(WriteShmMappingMagic(shmObjectName, 0));

    const auto client = CreateShmClient(shmAddress);
    REQUIRE_NE(client, nullptr);
    CHECK_FALSE(client->Init());
}

// ============================================================
// 服务端侧连接回收：一次不被对端确认的受理必须被收回
// 对端由测试通过映射视图直接扮演（写控制头状态），服务端只经 HandleIoEvent 驱动
// ============================================================

constexpr unsigned ControlHeaderIndex = 0;
constexpr unsigned FirstChannelIndex = 1;
constexpr auto ReclaimDriveLimit = std::chrono::milliseconds(8000);

TEST_CASE("ShmConnectLifecycleTest.ReclaimsConnectWhosePeerNeverAttached")
{
    const auto shmObjectName = MakeUniqueShmObjectName("SparkShmUnitTestReclaimRejected");
    const auto server = CreateShmServer(ToShmAddress(shmObjectName, "3"));
    REQUIRE_NE(server, nullptr);
    REQUIRE(server->Init());
    ConnectEventProbe probe;
    server->Subscribe(&probe);

    REQUIRE(WriteShmHeaderStatus(shmObjectName, ControlHeaderIndex, ConnectStatusType::Connecting));
    server->HandleIoEvent();
    CHECK_EQ(probe.ConnectCount, 1);
    CHECK_EQ(ReadShmHeaderStatus(shmObjectName, FirstChannelIndex), ConnectStatusType::Accepted);

    REQUIRE(WriteShmHeaderStatus(shmObjectName, ControlHeaderIndex, ConnectStatusType::UnConnected));
    server->HandleIoEvent();
    CHECK_EQ(probe.DisConnectCount, 0);
    CHECK_EQ(ReadShmHeaderStatus(shmObjectName, FirstChannelIndex), ConnectStatusType::Accepted);

    INFO(DriveBudgetExhaustedMessage);
    REQUIRE(DriveIoEventsUntil(*server, [&probe] { return probe.DisConnectCount > 0; }, ReclaimDriveLimit));
    CHECK_EQ(probe.DisConnectCount, 1);
    CHECK_EQ(ReadShmHeaderStatus(shmObjectName, FirstChannelIndex), ConnectStatusType::UnConnected);

    REQUIRE(WriteShmHeaderStatus(shmObjectName, ControlHeaderIndex, ConnectStatusType::Connecting));
    server->HandleIoEvent();
    CHECK_EQ(probe.ConnectCount, 2);
    CHECK_EQ(ReadShmHeaderStatus(shmObjectName, FirstChannelIndex), ConnectStatusType::Accepted);
}

TEST_CASE("ShmConnectLifecycleTest.ReclaimsConnectWhosePeerNeverConfirmedAndResetsTheControlHeader")
{
    const auto shmObjectName = MakeUniqueShmObjectName("SparkShmUnitTestReclaimSilent");
    const auto server = CreateShmServer(ToShmAddress(shmObjectName, "3"));
    REQUIRE_NE(server, nullptr);
    REQUIRE(server->Init());
    ConnectEventProbe probe;
    server->Subscribe(&probe);

    REQUIRE(WriteShmHeaderStatus(shmObjectName, ControlHeaderIndex, ConnectStatusType::Connecting));
    server->HandleIoEvent();
    CHECK_EQ(probe.ConnectCount, 1);
    CHECK_EQ(ReadShmHeaderStatus(shmObjectName, ControlHeaderIndex), ConnectStatusType::Accepted);

    INFO(DriveBudgetExhaustedMessage);
    REQUIRE(DriveIoEventsUntil(
        *server, [&probe, &shmObjectName]
        { return probe.DisConnectCount > 0 && ReadShmHeaderStatus(shmObjectName, ControlHeaderIndex) == ConnectStatusType::UnConnected; },
        ReclaimDriveLimit));
    CHECK_EQ(probe.DisConnectCount, 1);
    CHECK_EQ(ReadShmHeaderStatus(shmObjectName, ControlHeaderIndex), ConnectStatusType::UnConnected);
    CHECK_EQ(ReadShmHeaderStatus(shmObjectName, FirstChannelIndex), ConnectStatusType::UnConnected);
}

TEST_CASE("ShmClientConfirmTest.ConfirmsAcceptAndPublishesConnected")
{
    const auto shmObjectName = MakeUniqueShmObjectName("SparkShmUnitTestClientConfirm");
    const auto shmAddress = ToShmAddress(shmObjectName, "3");
    const auto server = CreateShmServer(shmAddress);
    REQUIRE_NE(server, nullptr);
    REQUIRE(server->Init());

    const auto client = CreateShmClient(shmAddress);
    REQUIRE_NE(client, nullptr);
    REQUIRE(client->Init());
    ConnectEventProbe clientProbe;
    client->Subscribe(&clientProbe);

    client->HandleIoEvent();
    CHECK_EQ(ReadShmHeaderStatus(shmObjectName, ControlHeaderIndex), ConnectStatusType::Connecting);

    server->HandleIoEvent();
    CHECK_EQ(ReadShmHeaderStatus(shmObjectName, FirstChannelIndex), ConnectStatusType::Accepted);

    client->HandleIoEvent();
    CHECK_EQ(ReadShmHeaderStatus(shmObjectName, FirstChannelIndex), ConnectStatusType::Connected);
    CHECK_EQ(clientProbe.ConnectCount, 1);
    CHECK_EQ(ReadShmHeaderStatus(shmObjectName, ControlHeaderIndex), ConnectStatusType::UnConnected);
}

TEST_CASE("ShmClientConfirmTest.LeavesRevokedAcceptUnconfirmedAndRetries")
{
    const auto shmObjectName = MakeUniqueShmObjectName("SparkShmUnitTestClientConfirmRevoked");
    const auto shmAddress = ToShmAddress(shmObjectName, "3");
    const auto server = CreateShmServer(shmAddress);
    REQUIRE_NE(server, nullptr);
    REQUIRE(server->Init());
    ConnectEventProbe serverProbe;
    server->Subscribe(&serverProbe);

    const auto client = CreateShmClient(shmAddress);
    REQUIRE_NE(client, nullptr);
    REQUIRE(client->Init());
    ConnectEventProbe clientProbe;
    client->Subscribe(&clientProbe);

    client->HandleIoEvent();
    server->HandleIoEvent();
    REQUIRE_EQ(ReadShmHeaderStatus(shmObjectName, FirstChannelIndex), ConnectStatusType::Accepted);
    REQUIRE(WriteShmHeaderStatus(shmObjectName, FirstChannelIndex, ConnectStatusType::DisConnected));

    client->HandleIoEvent();
    CHECK_EQ(ReadShmHeaderStatus(shmObjectName, FirstChannelIndex), ConnectStatusType::DisConnected);
    CHECK_EQ(clientProbe.ConnectCount, 0);

    client->HandleIoEvent();
    CHECK_EQ(ReadShmHeaderStatus(shmObjectName, ControlHeaderIndex), ConnectStatusType::Connecting);

    INFO(DriveBudgetExhaustedMessage);
    REQUIRE(DriveIoEventsUntil(*server, [&serverProbe] { return serverProbe.DisConnectCount > 0; }, ReclaimDriveLimit));
    CHECK_EQ(serverProbe.DisConnectCount, 1);
    CHECK_EQ(ReadShmHeaderStatus(shmObjectName, FirstChannelIndex), ConnectStatusType::UnConnected);
}

// ============================================================
// 订阅者收包回调的兜底：回调抛出不得穿出 IO 层，也不得吃掉这一轮之后的数据
// 收包缓冲由 IO 层持有、仅在本次回调期间有效，故服务端无论回调成败都必须把它还给池
// ============================================================

constexpr int ThrowingRecvPayloadCount = 100;
constexpr size_t ThrowingRecvPayloadLength = 5;

TEST_CASE("ShmSubscriberRecvFailureTest.AThrowingOnRecvNeitherEscapesTheIoLoopNorStopsLaterPayloads")
{
    const auto shmObjectName = MakeUniqueShmObjectName("SparkShmUnitTestThrowingRecv");
    const auto shmAddress = ToShmAddress(shmObjectName, "3");
    const auto server = CreateShmServer(shmAddress);
    REQUIRE_NE(server, nullptr);
    REQUIRE(server->Init());
    ThrowingRecvProbe serverProbe;
    serverProbe.ThrowingRecvCount = 1;
    server->Subscribe(&serverProbe);

    const auto client = CreateShmClient(shmAddress);
    REQUIRE_NE(client, nullptr);
    REQUIRE(client->Init());
    ThrowingRecvProbe clientProbe;
    client->Subscribe(&clientProbe);

    client->HandleIoEvent();
    server->HandleIoEvent();
    client->HandleIoEvent();
    REQUIRE_EQ(clientProbe.ConnectCount, 1);

    for (int index = 0; index < ThrowingRecvPayloadCount; ++index)
    {
        Spark::LinearBuffer<BufferSize>* buffer = client->AllocateSendBuffer();
        REQUIRE_NE(buffer, nullptr);
        REQUIRE_EQ(buffer->Append("Spark", ThrowingRecvPayloadLength), ThrowingRecvPayloadLength);
        client->Send(clientProbe.PeerSessionId, buffer);
        server->HandleIoEvent();
    }

    CHECK_EQ(serverProbe.RecvCount, ThrowingRecvPayloadCount);
    CHECK_EQ(serverProbe.LastRecvLength, ThrowingRecvPayloadLength);
    CHECK_EQ(serverProbe.DisConnectCount, 0);
}

// ============================================================
// 写不出时的收口：对端已断连则丢弃剩余字节并记一条 Warning，而不是自旋等待
// 断连由测试经映射视图改写 1 号通道头的状态扮演，两侧连接本身都是真的
// ============================================================

constexpr std::string_view DeliveredPayloadText = "Delivered";
constexpr std::string_view DroppedPayloadText = "DroppedBytes";
constexpr const char* DroppedBufferLogSignature = "Send Peer DisConnected, Drop Buffer.";

TEST_CASE("ShmSendTest.DropsTheRemainingBytesWhenThePeerIsDisConnected")
{
    const auto shmObjectName = MakeUniqueShmObjectName("SparkShmUnitTestSendDrop");
    const auto shmAddress = ToShmAddress(shmObjectName, "3");
    const auto server = CreateShmServer(shmAddress);
    REQUIRE_NE(server, nullptr);
    REQUIRE(server->Init());
    ConnectEventProbe serverProbe;
    server->Subscribe(&serverProbe);

    const auto client = CreateShmClient(shmAddress);
    REQUIRE_NE(client, nullptr);
    REQUIRE(client->Init());

    client->HandleIoEvent();
    server->HandleIoEvent();
    client->HandleIoEvent();
    REQUIRE_EQ(serverProbe.ConnectCount, 1);
    REQUIRE_EQ(ReadShmHeaderStatus(shmObjectName, FirstChannelIndex), ConnectStatusType::Connected);

    Spark::LinearBuffer<BufferSize>* deliveredBuffer = server->AllocateSendBuffer();
    REQUIRE_NE(deliveredBuffer, nullptr);
    REQUIRE_EQ(deliveredBuffer->Append(DeliveredPayloadText.data(), DeliveredPayloadText.size()), DeliveredPayloadText.size());
    server->Send(serverProbe.PeerSessionId, deliveredBuffer);
    client->HandleIoEvent();

    ShmChannelDownCounters deliveredCounters;
    REQUIRE(ReadShmChannelDownCounters(shmObjectName, FirstChannelIndex, deliveredCounters));
    INFO("断连前那一发没写进通道：本用例的前提不成立");
    REQUIRE_EQ(deliveredCounters.WriteCount, DeliveredPayloadText.size());
    INFO("对端没把那一发读走：本用例的前提不成立");
    REQUIRE_EQ(deliveredCounters.ReadCount, DeliveredPayloadText.size());

    REQUIRE(WriteShmHeaderStatus(shmObjectName, FirstChannelIndex, ConnectStatusType::DisConnected));

    Spark::LinearBuffer<BufferSize>* droppedBuffer = server->AllocateSendBuffer();
    REQUIRE_NE(droppedBuffer, nullptr);
    REQUIRE_EQ(droppedBuffer->Append(DroppedPayloadText.data(), DroppedPayloadText.size()), DroppedPayloadText.size());

    int droppedBufferLogCount = 0;
    {
        ExternLogCapture droppedBufferCapture(DroppedBufferLogSignature);
        server->Send(serverProbe.PeerSessionId, droppedBuffer);
        droppedBufferLogCount = droppedBufferCapture.MatchCount();
    }

    INFO("对端断连时没有记下丢弃那条 Warning：丢弃分支可能被改成了静默 break");
    CHECK_EQ(droppedBufferLogCount, 1);

    ShmChannelDownCounters droppedCounters;
    REQUIRE(ReadShmChannelDownCounters(shmObjectName, FirstChannelIndex, droppedCounters));
    INFO("被丢弃的字节进了通道");
    CHECK_EQ(droppedCounters.WriteCount, DeliveredPayloadText.size());
    INFO("丢弃的字节对对端可见");
    CHECK_EQ(droppedCounters.ReadCount, DeliveredPayloadText.size());
}

// ============================================================
// 写不出但未断连时的收口：一个字节都不丢，等到对端开读再写下去；等满阈值记一条 Warning
// 通道真的被填满（整帧 64 KiB × 32 帧 > 1 MiB 窗口），发送挂在后台线程，主线程驱动对端消费
// ============================================================

constexpr const char* BlockedSendLogSignature = "Send Blocked, Peer Not Reading.";
constexpr size_t SendFrameLength = BufferSize;
constexpr int SendFrameCount = 32;
constexpr char SendFrameByteBase = 'A';
constexpr auto BlockedSendWarningBudget = std::chrono::milliseconds(5000);
constexpr auto SendDeliveryBudget = std::chrono::milliseconds(5000);

TEST_CASE("ShmSendTest.KeepsWaitingInsteadOfDroppingWhenThePeerIsSlowToRead")
{
    const auto shmObjectName = MakeUniqueShmObjectName("SparkShmUnitTestSendBlocked");
    const auto shmAddress = ToShmAddress(shmObjectName, "3");
    const auto server = CreateShmServer(shmAddress);
    REQUIRE_NE(server, nullptr);
    REQUIRE(server->Init());
    ConnectEventProbe serverProbe;
    server->Subscribe(&serverProbe);

    auto client = CreateShmClient(shmAddress);
    REQUIRE_NE(client, nullptr);
    REQUIRE(client->Init());
    PayloadCollectingProbe clientProbe;
    client->Subscribe(&clientProbe);

    client->HandleIoEvent();
    server->HandleIoEvent();
    client->HandleIoEvent();
    REQUIRE_EQ(serverProbe.ConnectCount, 1);
    REQUIRE_EQ(ReadShmHeaderStatus(shmObjectName, FirstChannelIndex), ConnectStatusType::Connected);

    ExternLogCapture blockedSendCapture(BlockedSendLogSignature);
    std::atomic<int> completedFrameCount{0};
    std::thread sender(
        [&]
        {
            for (int frameIndex = 0; frameIndex < SendFrameCount; ++frameIndex)
            {
                Spark::LinearBuffer<BufferSize>* buffer = server->AllocateSendBuffer();
                if (buffer == nullptr)
                {
                    return;
                }
                const std::string framePayload(SendFrameLength, static_cast<char>(SendFrameByteBase + frameIndex % 26));
                REQUIRE_EQ(buffer->Append(framePayload.data(), framePayload.size()), framePayload.size());
                server->Send(serverProbe.PeerSessionId, buffer);
                completedFrameCount.store(frameIndex + 1);
            }
        });

    const bool warningAppeared = WaitUntil([] {}, [&] { return blockedSendCapture.MatchCount() > 0; }, BlockedSendWarningBudget);

    const size_t expectedPayloadLength = static_cast<size_t>(SendFrameCount) * SendFrameLength;
    const bool everythingDelivered =
        WaitUntil([&] { client->HandleIoEvent(); }, [&] { return clientProbe.ReceivedPayload.size() >= expectedPayloadLength; }, SendDeliveryBudget);
    if (!everythingDelivered)
    {
        client.reset();
    }
    sender.join();

    INFO("通道写满且对端仍 Connected 时，等满阈值没有记下那条 Warning");
    CHECK(warningAppeared);
    INFO("发送线程没有把 32 帧全部写出去");
    CHECK_EQ(completedFrameCount.load(), SendFrameCount);
    INFO("字节没有完整送达：写不出的部分被丢了");
    CHECK_EQ(clientProbe.ReceivedPayload.size(), expectedPayloadLength);
    INFO("阻塞 Warning 应只在等满阈值时记一条");
    CHECK_EQ(blockedSendCapture.MatchCount(), 1);

    size_t firstMismatchIndex = clientProbe.ReceivedPayload.size();
    for (size_t index = 0; index < clientProbe.ReceivedPayload.size(); ++index)
    {
        const char expectedByte = static_cast<char>(SendFrameByteBase + ((index / SendFrameLength) % 26));
        if (clientProbe.ReceivedPayload[index] != expectedByte)
        {
            firstMismatchIndex = index;
            break;
        }
    }
    INFO("第 " << firstMismatchIndex << " 字节起与发送内容不一致：字节被丢或被错序");
    CHECK_EQ(firstMismatchIndex, expectedPayloadLength);
}
