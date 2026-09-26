#include <gtest/gtest.h>
#include "TestUtility/TestUtility.h"
// ============================================================
// TryParseTestProtocol / ApplyTestProtocolFromCommandLine 测试
// 名字→协议 的映射是"选中哪条读路径"的唯一入口，故需钉住：
// 四个已配置的名字能选中、大小写敏感、无法识别时返回空，
// 且解析失败不得改动全局 TestProtocol
// ------------------------------------------------------------
// TryParseIoModel / ApplyIoModelFromCommandLine 测试
// 名字→IO 模型 的映射是"选中哪套收发实现"的唯一入口，与上者同为命令行第二个入口；
// 名字取自 GetIoModelString，两处拼写一旦分家这条测试就会红
// ============================================================

namespace
{
// 全局量被解析函数改写后必须复原，否则用例之间的执行顺序会影响结论
template <typename T>
struct GlobalValueGuard
{
    explicit GlobalValueGuard(T& target) : target_(target), saved_(target) {}
    ~GlobalValueGuard() { target_ = saved_; }
    GlobalValueGuard(const GlobalValueGuard&) = delete;
    GlobalValueGuard& operator=(const GlobalValueGuard&) = delete;

private:
    T& target_;
    T saved_;
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
    GlobalValueGuard<TestProtocolType> guard(TestProtocol);
    TestProtocol = TestProtocolType::Step;

    const char* const argv[] = {"TestClient.exe"};
    EXPECT_EQ(ApplyTestProtocolFromCommandLine(1, argv), 0);
    EXPECT_EQ(TestProtocol, TestProtocolType::Step);
}

TEST(ApplyTestProtocolFromCommandLineTest, SelectsProtocolFromFirstArgument)
{
    GlobalValueGuard<TestProtocolType> guard(TestProtocol);

    const char* const argv[] = {"TestClient.exe", "Step"};
    EXPECT_EQ(ApplyTestProtocolFromCommandLine(2, argv), 0);
    EXPECT_EQ(TestProtocol, TestProtocolType::Step);
}

TEST(ApplyTestProtocolFromCommandLineTest, IgnoresArgumentsAfterTheFirst)
{
    GlobalValueGuard<TestProtocolType> guard(TestProtocol);

    const char* const argv[] = {"TestClient.exe", "Xtp", "Step"};
    EXPECT_EQ(ApplyTestProtocolFromCommandLine(3, argv), 0);
    EXPECT_EQ(TestProtocol, TestProtocolType::Xtp);
}

TEST(ApplyTestProtocolFromCommandLineTest, ReportsUsageExitCodeAndKeepsProtocolOnUnknownName)
{
    GlobalValueGuard<TestProtocolType> guard(TestProtocol);
    TestProtocol = TestProtocolType::Shm;

    const char* const argv[] = {"TestClient.exe", "step"};
    EXPECT_EQ(ApplyTestProtocolFromCommandLine(2, argv), 2);
    EXPECT_EQ(TestProtocol, TestProtocolType::Shm);
}

TEST(TryParseIoModelTest, AcceptsEachConfiguredName)
{
    ASSERT_TRUE(TryParseIoModel("Select").has_value());
    EXPECT_EQ(*TryParseIoModel("Select"), IoModelType::Select);
    EXPECT_EQ(*TryParseIoModel("Epoll"), IoModelType::Epoll);
    EXPECT_EQ(*TryParseIoModel("Iocp"), IoModelType::Iocp);
}

TEST(TryParseIoModelTest, RejectsNullName)
{
    EXPECT_FALSE(TryParseIoModel(nullptr).has_value());
}

TEST(TryParseIoModelTest, RejectsUnconfiguredName)
{
    EXPECT_FALSE(TryParseIoModel("").has_value());
    EXPECT_FALSE(TryParseIoModel("Unknown").has_value());
    EXPECT_FALSE(TryParseIoModel("Iocp ").has_value());
}

TEST(TryParseIoModelTest, IsCaseSensitive)
{
    EXPECT_FALSE(TryParseIoModel("iocp").has_value());
    EXPECT_FALSE(TryParseIoModel("SELECT").has_value());
}

TEST(ApplyIoModelFromCommandLineTest, LeavesIoModelUnchangedWhenNoSecondArgument)
{
    GlobalValueGuard<IoModelType> guard(IoModel);
    IoModel = IoModelType::Select;

    const char* const argv[] = {"TestClient.exe", "Step"};
    EXPECT_EQ(ApplyIoModelFromCommandLine(2, argv), 0);
    EXPECT_EQ(IoModel, IoModelType::Select);
}

TEST(ApplyIoModelFromCommandLineTest, SelectsIoModelFromSecondArgument)
{
    GlobalValueGuard<IoModelType> guard(IoModel);
    IoModel = IoModelType::Select;

    const char* const argv[] = {"TestClient.exe", "Step", "Iocp"};
    EXPECT_EQ(ApplyIoModelFromCommandLine(3, argv), 0);
    EXPECT_EQ(IoModel, IoModelType::Iocp);
}

TEST(ApplyIoModelFromCommandLineTest, IgnoresArgumentsAfterTheSecond)
{
    GlobalValueGuard<IoModelType> guard(IoModel);

    const char* const argv[] = {"TestClient.exe", "Step", "Iocp", "Epoll"};
    EXPECT_EQ(ApplyIoModelFromCommandLine(4, argv), 0);
    EXPECT_EQ(IoModel, IoModelType::Iocp);
}

TEST(ApplyIoModelFromCommandLineTest, ReportsUsageExitCodeAndKeepsIoModelOnUnknownName)
{
    GlobalValueGuard<IoModelType> guard(IoModel);
    IoModel = IoModelType::Select;

    const char* const argv[] = {"TestClient.exe", "Step", "iocp"};
    EXPECT_EQ(ApplyIoModelFromCommandLine(3, argv), 2);
    EXPECT_EQ(IoModel, IoModelType::Select);
}
