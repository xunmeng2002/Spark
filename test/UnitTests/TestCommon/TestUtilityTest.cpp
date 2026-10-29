#include "TestUtility/TestUtility.h"
#include "doctest/doctest.h"

using namespace Spark;
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

TEST_CASE("TryParseTestProtocolTest.AcceptsEachConfiguredName")
{
    REQUIRE(TryParseTestProtocol("Shm").has_value());
    CHECK_EQ(*TryParseTestProtocol("Shm"), TestProtocolType::Shm);
    CHECK_EQ(*TryParseTestProtocol("Tcp"), TestProtocolType::Tcp);
    CHECK_EQ(*TryParseTestProtocol("Xtp"), TestProtocolType::Xtp);
    CHECK_EQ(*TryParseTestProtocol("Step"), TestProtocolType::Step);
}

TEST_CASE("TryParseTestProtocolTest.RejectsNullName")
{
    CHECK_FALSE(TryParseTestProtocol(nullptr).has_value());
}

TEST_CASE("TryParseTestProtocolTest.RejectsUnconfiguredName")
{
    CHECK_FALSE(TryParseTestProtocol("").has_value());
    CHECK_FALSE(TryParseTestProtocol("Http").has_value());
    CHECK_FALSE(TryParseTestProtocol("Step ").has_value());
}

TEST_CASE("TryParseTestProtocolTest.IsCaseSensitive")
{
    CHECK_FALSE(TryParseTestProtocol("step").has_value());
    CHECK_FALSE(TryParseTestProtocol("STEP").has_value());
}

TEST_CASE("ApplyTestProtocolFromCommandLineTest.LeavesProtocolUnchangedWhenNoArgument")
{
    GlobalValueGuard<TestProtocolType> guard(TestProtocol);
    TestProtocol = TestProtocolType::Step;

    const char* const argv[] = {"TestClient.exe"};
    CHECK_EQ(ApplyTestProtocolFromCommandLine(1, argv), 0);
    CHECK_EQ(TestProtocol, TestProtocolType::Step);
}

TEST_CASE("ApplyTestProtocolFromCommandLineTest.SelectsProtocolFromFirstArgument")
{
    GlobalValueGuard<TestProtocolType> guard(TestProtocol);

    const char* const argv[] = {"TestClient.exe", "Step"};
    CHECK_EQ(ApplyTestProtocolFromCommandLine(2, argv), 0);
    CHECK_EQ(TestProtocol, TestProtocolType::Step);
}

TEST_CASE("ApplyTestProtocolFromCommandLineTest.IgnoresArgumentsAfterTheFirst")
{
    GlobalValueGuard<TestProtocolType> guard(TestProtocol);

    const char* const argv[] = {"TestClient.exe", "Xtp", "Step"};
    CHECK_EQ(ApplyTestProtocolFromCommandLine(3, argv), 0);
    CHECK_EQ(TestProtocol, TestProtocolType::Xtp);
}

TEST_CASE("ApplyTestProtocolFromCommandLineTest.ReportsUsageExitCodeAndKeepsProtocolOnUnknownName")
{
    GlobalValueGuard<TestProtocolType> guard(TestProtocol);
    TestProtocol = TestProtocolType::Shm;

    const char* const argv[] = {"TestClient.exe", "step"};
    CHECK_EQ(ApplyTestProtocolFromCommandLine(2, argv), 2);
    CHECK_EQ(TestProtocol, TestProtocolType::Shm);
}

TEST_CASE("TryParseIoModelTest.AcceptsEachConfiguredName")
{
    REQUIRE(TryParseIoModel("Select").has_value());
    CHECK_EQ(*TryParseIoModel("Select"), IoModelType::Select);
    CHECK_EQ(*TryParseIoModel("Epoll"), IoModelType::Epoll);
    CHECK_EQ(*TryParseIoModel("Iocp"), IoModelType::Iocp);
}

TEST_CASE("TryParseIoModelTest.RejectsNullName")
{
    CHECK_FALSE(TryParseIoModel(nullptr).has_value());
}

TEST_CASE("TryParseIoModelTest.RejectsUnconfiguredName")
{
    CHECK_FALSE(TryParseIoModel("").has_value());
    CHECK_FALSE(TryParseIoModel("Unknown").has_value());
    CHECK_FALSE(TryParseIoModel("Iocp ").has_value());
}

TEST_CASE("TryParseIoModelTest.IsCaseSensitive")
{
    CHECK_FALSE(TryParseIoModel("iocp").has_value());
    CHECK_FALSE(TryParseIoModel("SELECT").has_value());
}

TEST_CASE("ApplyIoModelFromCommandLineTest.LeavesIoModelUnchangedWhenNoSecondArgument")
{
    GlobalValueGuard<IoModelType> guard(IoModel);
    IoModel = IoModelType::Select;

    const char* const argv[] = {"TestClient.exe", "Step"};
    CHECK_EQ(ApplyIoModelFromCommandLine(2, argv), 0);
    CHECK_EQ(IoModel, IoModelType::Select);
}

TEST_CASE("ApplyIoModelFromCommandLineTest.SelectsIoModelFromSecondArgument")
{
    GlobalValueGuard<IoModelType> guard(IoModel);
    IoModel = IoModelType::Select;

    const char* const argv[] = {"TestClient.exe", "Step", "Iocp"};
    CHECK_EQ(ApplyIoModelFromCommandLine(3, argv), 0);
    CHECK_EQ(IoModel, IoModelType::Iocp);
}

TEST_CASE("ApplyIoModelFromCommandLineTest.IgnoresArgumentsAfterTheSecond")
{
    GlobalValueGuard<IoModelType> guard(IoModel);

    const char* const argv[] = {"TestClient.exe", "Step", "Iocp", "Epoll"};
    CHECK_EQ(ApplyIoModelFromCommandLine(4, argv), 0);
    CHECK_EQ(IoModel, IoModelType::Iocp);
}

TEST_CASE("ApplyIoModelFromCommandLineTest.ReportsUsageExitCodeAndKeepsIoModelOnUnknownName")
{
    GlobalValueGuard<IoModelType> guard(IoModel);
    IoModel = IoModelType::Select;

    const char* const argv[] = {"TestClient.exe", "Step", "iocp"};
    CHECK_EQ(ApplyIoModelFromCommandLine(3, argv), 2);
    CHECK_EQ(IoModel, IoModelType::Select);
}
