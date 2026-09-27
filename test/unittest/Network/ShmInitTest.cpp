#include <gtest/gtest.h>
#include <Spark/Network/Io/IoBase.h>
#include <Spark/Network/Io/IoFactory.h>
#include <Spark/TemplateLib/Buffer/ShmBuffer.h>

#include <chrono>
#include <format>
#include <memory>
#include <string>

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

bool WriteShmMappingMagic(const std::string& shmObjectName, unsigned magic)
{
#ifdef _WIN32
    HANDLE fileMapping = OpenFileMappingA(FILE_MAP_ALL_ACCESS, FALSE, shmObjectName.c_str());
    if (fileMapping == nullptr)
    {
        return false;
    }
    void* mappingView = MapViewOfFile(fileMapping, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(Spark::SingleShmHeader));
    if (mappingView == nullptr)
    {
        CloseHandle(fileMapping);
        return false;
    }
    Spark::SingleShmHeader* shmHeader = static_cast<Spark::SingleShmHeader*>(mappingView);
    Spark::SingleShmHeader::StoreMappedField(shmHeader->MappingMagic, magic);
    UnmapViewOfFile(mappingView);
    CloseHandle(fileMapping);
    return true;
#else
    const int fileDescriptor = shm_open(shmObjectName.c_str(), O_RDWR, 0666);
    if (fileDescriptor < 0)
    {
        return false;
    }
    void* mappingView = mmap(nullptr, sizeof(Spark::SingleShmHeader), PROT_READ | PROT_WRITE, MAP_SHARED, fileDescriptor, 0);
    close(fileDescriptor);
    if (mappingView == MAP_FAILED)
    {
        return false;
    }
    Spark::SingleShmHeader* shmHeader = static_cast<Spark::SingleShmHeader*>(mappingView);
    Spark::SingleShmHeader::StoreMappedField(shmHeader->MappingMagic, magic);
    munmap(mappingView, sizeof(Spark::SingleShmHeader));
    return true;
#endif
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
