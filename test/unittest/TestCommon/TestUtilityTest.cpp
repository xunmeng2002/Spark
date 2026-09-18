#include <gtest/gtest.h>
#include "TestUtility/TestUtility.h"
// ============================================================
// TryParseTestProtocol / ApplyTestProtocolFromCommandLine 测试
// 名字→协议 的映射是"选中哪条读路径"的唯一入口，故需钉住：
// 四个已配置的名字能选中、大小写敏感、无法识别时返回空，
// 且解析失败不得改动全局 TestProtocol
// ============================================================

namespace
{
struct TestProtocolGuard
{
    TestProtocolGuard() : saved_(TestProtocol) {}
    ~TestProtocolGuard() { TestProtocol = saved_; }
    TestProtocolGuard(const TestProtocolGuard&) = delete;
    TestProtocolGuard& operator=(const TestProtocolGuard&) = delete;

private:
    TestProtocolType saved_;
};
}

TEST(TryParseTestProtocolTest, AcceptsEachConfiguredName)
{
    ASSERT_TRUE(TryParseTestProtocol("Shm").has_value());
    EXPECT_EQ(*TryParseTestProtocol("Shm"), TestProtocolType::Shm);
    EXPECT_EQ(*TryParseTestProtocol("Tcp"), TestProtocolType::Tcp);
    EXPECT_EQ(*TryParseTestProtocol("Xtp"), TestProtocolType::Xtp);
    EXPECT_EQ(*TryParseTestProtocol("Step"), TestProtocolType::Step);
}

TEST(TryParseTestProtocolTest, RejectsNullName)
{
    EXPECT_FALSE(TryParseTestProtocol(nullptr).has_value());
}

TEST(TryParseTestProtocolTest, RejectsUnconfiguredName)
{
    EXPECT_FALSE(TryParseTestProtocol("").has_value());
    EXPECT_FALSE(TryParseTestProtocol("Http").has_value());
    EXPECT_FALSE(TryParseTestProtocol("Step ").has_value());
}

TEST(TryParseTestProtocolTest, IsCaseSensitive)
{
    EXPECT_FALSE(TryParseTestProtocol("step").has_value());
    EXPECT_FALSE(TryParseTestProtocol("STEP").has_value());
}

TEST(ApplyTestProtocolFromCommandLineTest, LeavesProtocolUnchangedWhenNoArgument)
{
    TestProtocolGuard guard;
    TestProtocol = TestProtocolType::Step;

    const char* const argv[] = {"TestClient.exe"};
    EXPECT_EQ(ApplyTestProtocolFromCommandLine(1, argv), 0);
    EXPECT_EQ(TestProtocol, TestProtocolType::Step);
}

TEST(ApplyTestProtocolFromCommandLineTest, SelectsProtocolFromFirstArgument)
{
    TestProtocolGuard guard;

    const char* const argv[] = {"TestClient.exe", "Step"};
    EXPECT_EQ(ApplyTestProtocolFromCommandLine(2, argv), 0);
    EXPECT_EQ(TestProtocol, TestProtocolType::Step);
}

TEST(ApplyTestProtocolFromCommandLineTest, IgnoresArgumentsAfterTheFirst)
{
    TestProtocolGuard guard;

    const char* const argv[] = {"TestClient.exe", "Xtp", "Step"};
    EXPECT_EQ(ApplyTestProtocolFromCommandLine(3, argv), 0);
    EXPECT_EQ(TestProtocol, TestProtocolType::Xtp);
}

TEST(ApplyTestProtocolFromCommandLineTest, ReportsUsageExitCodeAndKeepsProtocolOnUnknownName)
{
    TestProtocolGuard guard;
    TestProtocol = TestProtocolType::Shm;

    const char* const argv[] = {"TestClient.exe", "step"};
    EXPECT_EQ(ApplyTestProtocolFromCommandLine(2, argv), 2);
    EXPECT_EQ(TestProtocol, TestProtocolType::Shm);
}
