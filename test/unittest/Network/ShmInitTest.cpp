#include <gtest/gtest.h>
#include <Spark/Network/Io/IoBase.h>
#include <Spark/Network/Io/IoFactory.h>

#include <chrono>
#include <format>
#include <memory>
#include <string>
using namespace Spark::Network;

namespace
{
std::string MakeUniqueShmName(const char* namePrefix)
{
    const auto ticks = std::chrono::steady_clock::now().time_since_epoch().count();
    return std::format("shm://{}{}", namePrefix, ticks);
}

std::string MakeUniqueShmAddress(const char* namePrefix, const char* connectSizeText)
{
    return MakeUniqueShmName(namePrefix) + ":" + connectSizeText;
}

std::unique_ptr<IoBase> CreateShmServer(const std::string& address)
{
    return std::unique_ptr<IoBase>(IoFactory::CreateIo(ServerTypeType::Server, address.c_str()));
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
