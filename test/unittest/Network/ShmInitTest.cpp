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
// shm 名在 Server 侧按「独占创建」打开（Windows CREATE_NEW / Linux O_CREAT|O_EXCL），
// 上一次运行若异常退出留下同名对象，后续每次 Init 都会被这层残留挡下——故按运行取唯一后缀。
std::string MakeUniqueShmAddress(const char* namePrefix, const char* connectSizeText)
{
    const auto ticks = std::chrono::steady_clock::now().time_since_epoch().count();
    return std::format("shm://{}{}:{}", namePrefix, ticks, connectSizeText);
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
