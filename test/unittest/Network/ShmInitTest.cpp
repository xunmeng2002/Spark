#include <gtest/gtest.h>
#include <Spark/Network/Io/IoBase.h>
#include <Spark/Network/Io/IoFactory.h>
#include <Spark/TemplateLib/Buffer/ShmBuffer.h>

#include <chrono>
#include <format>
#include <memory>
#include <string>
#include <thread>

#ifdef _WIN32
#include <Windows.h>
#else
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#endif
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
    const int fileDescriptor = shm_open(shmObjectName.c_str(), O_RDWR, 0666);
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

class ConnectEventProbe : public IoSubscriber
{
public:
    void OnConnect(SessionIdType, const char*, int) override { ++ConnectCount; }
    void OnDisConnect(SessionIdType, const char*, int) override { ++DisConnectCount; }
    void OnRecv(SessionIdType, const char*, size_t) override {}

    int ConnectCount = 0;
    int DisConnectCount = 0;
};

template <typename StopCondition>
void DriveIoEventsUntil(IoBase& io, StopCondition&& stopCondition, std::chrono::milliseconds limit)
{
    const auto deadline = std::chrono::steady_clock::now() + limit;
    while (!stopCondition() && std::chrono::steady_clock::now() < deadline)
    {
        io.HandleIoEvent();
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}
}

// ============================================================
// Shm 后端 Init 门禁测试
// 地址末段即连接数（ShmBase 用 StepUtility::ParseInteger 解析），非法值须在 Init 内被拒
// ============================================================

TEST(ShmInitTest, Init_RejectsNonNumericConnectSize)
{
    const auto io = CreateShmServer(MakeUniqueShmAddress("SparkShmUnitTestRejectText", "abc"));

    ASSERT_NE(io, nullptr);
    EXPECT_FALSE(io->Init());
}

TEST(ShmInitTest, Init_RejectsZeroConnectSize)
{
    const auto io = CreateShmServer(MakeUniqueShmAddress("SparkShmUnitTestRejectZero", "0"));

    ASSERT_NE(io, nullptr);
    EXPECT_FALSE(io->Init());
}

TEST(ShmInitTest, Init_AcceptsSmallestPositiveConnectSize)
{
    const auto io = CreateShmServer(MakeUniqueShmAddress("SparkShmUnitTestAccept", "1"));

    ASSERT_NE(io, nullptr);
    EXPECT_TRUE(io->Init());
}

TEST(ShmInitTest, Init_RejectsConnectSizeWhoseMappingWouldWrap)
{
    const auto io = CreateShmServer(MakeUniqueShmAddress("SparkShmUnitTestWrap", "2049"));

    ASSERT_NE(io, nullptr);
    EXPECT_FALSE(io->Init());
}

TEST(ShmInitTest, Init_ReusesShmObjectLeftByAPreviousRun)
{
    const auto shmAddress = MakeUniqueShmAddress("SparkShmUnitTestReuse", "1");
    const auto leftoverOwner = CreateShmServer(shmAddress);
    ASSERT_NE(leftoverOwner, nullptr);
    ASSERT_TRUE(leftoverOwner->Init());

    const auto secondOwner = CreateShmServer(shmAddress);
    ASSERT_NE(secondOwner, nullptr);
    EXPECT_TRUE(secondOwner->Init());
}

TEST(ShmInitTest, Init_RejectsReusedShmObjectSmallerThanNeeded)
{
    const auto shmName = MakeUniqueShmName("SparkShmUnitTestReuseSmall");
    const auto smallerOwner = CreateShmServer(shmName + ":1");
    ASSERT_NE(smallerOwner, nullptr);
    ASSERT_TRUE(smallerOwner->Init());

    const auto largerOwner = CreateShmServer(shmName + ":2");
    ASSERT_NE(largerOwner, nullptr);
    EXPECT_FALSE(largerOwner->Init());
}

TEST(ShmInitTest, Init_RejectsReusedShmObjectWithForeignMappingMagic)
{
    const auto shmObjectName = MakeUniqueShmObjectName("SparkShmUnitTestForeignMagic");
    const auto shmAddress = ToShmAddress(shmObjectName, "1");
    const auto leftoverOwner = CreateShmServer(shmAddress);
    ASSERT_NE(leftoverOwner, nullptr);
    ASSERT_TRUE(leftoverOwner->Init());
    ASSERT_TRUE(WriteShmMappingMagic(shmObjectName, 0xDEADBEEF));

    const auto secondOwner = CreateShmServer(shmAddress);
    ASSERT_NE(secondOwner, nullptr);
    EXPECT_FALSE(secondOwner->Init());
}

TEST(ShmInitTest, Init_RejectsReusedShmObjectWithForeignMappingLayoutVersion)
{
    const auto shmObjectName = MakeUniqueShmObjectName("SparkShmUnitTestForeignLayoutVersion");
    const auto shmAddress = ToShmAddress(shmObjectName, "1");
    const auto leftoverOwner = CreateShmServer(shmAddress);
    ASSERT_NE(leftoverOwner, nullptr);
    ASSERT_TRUE(leftoverOwner->Init());
    ASSERT_TRUE(WriteShmMappingLayoutVersion(shmObjectName, Spark::ShmMappingLayoutVersion + 1));

    const auto secondOwner = CreateShmServer(shmAddress);
    ASSERT_NE(secondOwner, nullptr);
    EXPECT_FALSE(secondOwner->Init());
}

TEST(ShmInitTest, Init_AcceptsStampedShmObjectForAClient)
{
    const auto shmAddress = ToShmAddress(MakeUniqueShmObjectName("SparkShmUnitTestClientStamp"), "1");
    const auto shmOwner = CreateShmServer(shmAddress);
    ASSERT_NE(shmOwner, nullptr);
    ASSERT_TRUE(shmOwner->Init());

    const auto client = CreateShmClient(shmAddress);
    ASSERT_NE(client, nullptr);
    EXPECT_TRUE(client->Init());
}

TEST(ShmInitTest, Init_RejectsUnstampedShmObjectForAClient)
{
    const auto shmObjectName = MakeUniqueShmObjectName("SparkShmUnitTestUnstamped");
    const auto shmAddress = ToShmAddress(shmObjectName, "1");
    const auto shmOwner = CreateShmServer(shmAddress);
    ASSERT_NE(shmOwner, nullptr);
    ASSERT_TRUE(shmOwner->Init());
    ASSERT_TRUE(WriteShmMappingMagic(shmObjectName, 0));

    const auto client = CreateShmClient(shmAddress);
    ASSERT_NE(client, nullptr);
    EXPECT_FALSE(client->Init());
}

// ============================================================
// 服务端侧连接回收：一次不被对端确认的受理必须被收回
// 对端由测试通过映射视图直接扮演（写控制头状态），服务端只经 HandleIoEvent 驱动
// ============================================================

constexpr unsigned ControlHeaderIndex = 0;
constexpr unsigned FirstChannelIndex = 1;
constexpr auto ReclaimDriveLimit = std::chrono::milliseconds(8000);

TEST(ShmConnectLifecycleTest, ReclaimsConnectWhosePeerNeverAttached)
{
    const auto shmObjectName = MakeUniqueShmObjectName("SparkShmUnitTestReclaimRejected");
    const auto server = CreateShmServer(ToShmAddress(shmObjectName, "3"));
    ASSERT_NE(server, nullptr);
    ASSERT_TRUE(server->Init());
    ConnectEventProbe probe;
    server->Subscribe(&probe);

    ASSERT_TRUE(WriteShmHeaderStatus(shmObjectName, ControlHeaderIndex, ConnectStatusType::Connecting));
    server->HandleIoEvent();
    EXPECT_EQ(probe.ConnectCount, 1);
    EXPECT_EQ(ReadShmHeaderStatus(shmObjectName, FirstChannelIndex), ConnectStatusType::Accepted);

    ASSERT_TRUE(WriteShmHeaderStatus(shmObjectName, ControlHeaderIndex, ConnectStatusType::UnConnected));
    server->HandleIoEvent();
    EXPECT_EQ(probe.DisConnectCount, 0);
    EXPECT_EQ(ReadShmHeaderStatus(shmObjectName, FirstChannelIndex), ConnectStatusType::Accepted);

    DriveIoEventsUntil(*server, [&probe] { return probe.DisConnectCount > 0; }, ReclaimDriveLimit);
    EXPECT_EQ(probe.DisConnectCount, 1);
    EXPECT_EQ(ReadShmHeaderStatus(shmObjectName, FirstChannelIndex), ConnectStatusType::UnConnected);

    ASSERT_TRUE(WriteShmHeaderStatus(shmObjectName, ControlHeaderIndex, ConnectStatusType::Connecting));
    server->HandleIoEvent();
    EXPECT_EQ(probe.ConnectCount, 2);
    EXPECT_EQ(ReadShmHeaderStatus(shmObjectName, FirstChannelIndex), ConnectStatusType::Accepted);
}

TEST(ShmConnectLifecycleTest, ReclaimsConnectWhosePeerNeverConfirmedAndResetsTheControlHeader)
{
    const auto shmObjectName = MakeUniqueShmObjectName("SparkShmUnitTestReclaimSilent");
    const auto server = CreateShmServer(ToShmAddress(shmObjectName, "3"));
    ASSERT_NE(server, nullptr);
    ASSERT_TRUE(server->Init());
    ConnectEventProbe probe;
    server->Subscribe(&probe);

    ASSERT_TRUE(WriteShmHeaderStatus(shmObjectName, ControlHeaderIndex, ConnectStatusType::Connecting));
    server->HandleIoEvent();
    EXPECT_EQ(probe.ConnectCount, 1);
    EXPECT_EQ(ReadShmHeaderStatus(shmObjectName, ControlHeaderIndex), ConnectStatusType::Accepted);

    DriveIoEventsUntil(
        *server, [&probe, &shmObjectName]
        { return probe.DisConnectCount > 0 && ReadShmHeaderStatus(shmObjectName, ControlHeaderIndex) == ConnectStatusType::UnConnected; },
        ReclaimDriveLimit);
    EXPECT_EQ(probe.DisConnectCount, 1);
    EXPECT_EQ(ReadShmHeaderStatus(shmObjectName, ControlHeaderIndex), ConnectStatusType::UnConnected);
    EXPECT_EQ(ReadShmHeaderStatus(shmObjectName, FirstChannelIndex), ConnectStatusType::UnConnected);
}
