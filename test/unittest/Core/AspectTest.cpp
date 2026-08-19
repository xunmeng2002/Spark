#include <gtest/gtest.h>
#include <Spark/Core/Aspect/Aspect.h>
#include <Spark/Core/Aspect/AspectBefore.h>
#include <Spark/Core/Aspect/AspectAfter.h>
#include <Spark/Core/Aspect/AspectLogger.h>
#include <Spark/Core/Aspect/AspectPerformance.h>

#include <string>
#include <vector>
#include <chrono>
#include <thread>
using namespace spark::core;
// ============================================================
// Aspect 测试
// AOP 切面模板：Before/After 调用顺序、多切面链式调用
// ============================================================

namespace
{
    // 测试辅助切面：记录 Before/After 调用轨迹
    struct CallTracer
    {
        std::vector<std::string> calls;

        void Before(const char* funcName)
        {
            calls.push_back(std::string(funcName) + "_Before");
        }

        void After(const char* funcName)
        {
            calls.push_back(std::string(funcName) + "_After");
        }
    };

    int g_aspect_value = 0;
    std::string g_aspect_trace;

    void SetAspectValue(int x)
    {
        g_aspect_value = x;
    }

    int DoubleAspectValue(int x)
    {
        return x * 2;
    }

    void AppendTrace(const std::string& tag)
    {
        g_aspect_trace += tag;
    }
}

// ============================================================
// Aspect（通用模板：Before + After）
// ============================================================

TEST(AspectTest, SingleAspect_CallOrder)
{
    g_aspect_value = 0;
    CallTracer tracer;

    auto wrapper = [](int x) { SetAspectValue(x); };
    Aspect<decltype(wrapper), int> aspect(std::move(wrapper), "SetValue");
    aspect.Invoke(42, tracer);

    EXPECT_EQ(g_aspect_value, 42);
    ASSERT_EQ(tracer.calls.size(), 2);
    EXPECT_EQ(tracer.calls[0], "SetValue_Before");
    EXPECT_EQ(tracer.calls[1], "SetValue_After");
}

TEST(AspectTest, MultipleAspects_NestingOrder)
{
    g_aspect_trace.clear();
    CallTracer outer;
    CallTracer inner;

    // 传递顺序：outer 先入，inner 后入
    // 调用顺序：outer.Before → inner.Before → Func → inner.After → outer.After
    auto wrapper = [](const std::string& tag) { AppendTrace(tag); };
    Aspect<decltype(wrapper), const std::string&> aspect(std::move(wrapper), "Multi");
    aspect.Invoke("func", outer, inner);

    ASSERT_EQ(outer.calls.size(), 2);
    EXPECT_EQ(outer.calls[0], "Multi_Before");
    EXPECT_EQ(outer.calls[1], "Multi_After");

    ASSERT_EQ(inner.calls.size(), 2);
    EXPECT_EQ(inner.calls[0], "Multi_Before");
    EXPECT_EQ(inner.calls[1], "Multi_After");

    EXPECT_EQ(g_aspect_trace, "func");
}

TEST(AspectTest, InvokeFreeFunction_SingleAspect)
{
    g_aspect_value = 0;

    auto wrapper = [](int x) { SetAspectValue(x); };
    Invoke<CallTracer>(wrapper, "FreeInvoke", 99);

    EXPECT_EQ(g_aspect_value, 99);
}

TEST(AspectTest, InvokeFreeFunction_MultipleAspects)
{
    g_aspect_trace.clear();

    auto wrapper = [](const std::string& tag) { AppendTrace(tag); };
    Invoke<CallTracer, CallTracer>(wrapper, "FreeMulti", "trace");

    EXPECT_EQ(g_aspect_trace, "trace");
}

TEST(AspectTest, FunctionWithReturnValue)
{
    auto wrapper = [](int x) -> int { return DoubleAspectValue(x); };
    Aspect<decltype(wrapper), int> aspect(std::move(wrapper), "Double");
    CallTracer tracer;

    // 无方面版本直接调用
    int result = 0;
    auto wrapper2 = [&](int x) { result = DoubleAspectValue(x); };
    Aspect<decltype(wrapper2), int> aspect2(std::move(wrapper2), "Double");
    aspect2.Invoke(21, tracer);

    EXPECT_EQ(result, 42);
}

// ============================================================
// AspectBefore（仅 Before 切面）
// ============================================================

TEST(AspectTest, AspectBefore_Single)
{
    g_aspect_value = 0;
    CallTracer tracer;

    auto wrapper = [](int x) { SetAspectValue(x); };
    AspectBefore<decltype(wrapper), int> aspect(wrapper, "BeforeOnly");
    aspect.InvokeBefore(77, tracer);

    EXPECT_EQ(g_aspect_value, 77);
    ASSERT_EQ(tracer.calls.size(), 1);
    EXPECT_EQ(tracer.calls[0], "BeforeOnly_Before");
}

TEST(AspectTest, AspectBefore_Multiple)
{
    g_aspect_trace.clear();

    // AP... 推演为非引用类型，自由函数避免了传左值引用导致 AP() 构造失败的问题
    auto wrapper = [](const std::string& tag) { AppendTrace(tag); };
    InvokeBefore<CallTracer, CallTracer>(wrapper, "MultiBefore", "ok");

    EXPECT_EQ(g_aspect_trace, "ok");
}

TEST(AspectTest, InvokeBeforeFreeFunction)
{
    g_aspect_value = 0;

    auto wrapper = [](int x) { SetAspectValue(x); };
    InvokeBefore<CallTracer>(wrapper, "InvokeBefore", 55);

    EXPECT_EQ(g_aspect_value, 55);
}

// ============================================================
// AspectAfter（仅 After 切面）
// ============================================================

TEST(AspectTest, AspectAfter_Single)
{
    g_aspect_value = 0;
    CallTracer tracer;

    auto wrapper = [](int x) { SetAspectValue(x); };
    AspectAfter<decltype(wrapper), int> aspect(wrapper, "AfterOnly");
    aspect.InvokeAfter(33, tracer);

    EXPECT_EQ(g_aspect_value, 33);
    ASSERT_EQ(tracer.calls.size(), 1);
    EXPECT_EQ(tracer.calls[0], "AfterOnly_After");
}

TEST(AspectTest, AspectAfter_Multiple)
{
    g_aspect_trace.clear();

    // 自由函数创建右值切面实例
    auto wrapper = [](const std::string& tag) { AppendTrace(tag); };
    InvokeAfter<CallTracer, CallTracer>(wrapper, "MultiAfter", "done");

    EXPECT_EQ(g_aspect_trace, "done");
}

TEST(AspectTest, InvokeAfterFreeFunction)
{
    g_aspect_value = 0;

    auto wrapper = [](int x) { SetAspectValue(x); };
    InvokeAfter<CallTracer>(wrapper, "InvokeAfter", 11);

    EXPECT_EQ(g_aspect_value, 11);
}

// ============================================================
// AspectLogger（真实日志切面）
// ============================================================

TEST(AspectTest, AspectLogger_BeforeAfter)
{
    // AspectLogger::Before/After 内部调用 WriteLog
    // WriteLog 在 GetWriteLogFunc() 为空时直接跳过
    // 验证不崩溃即可
    AspectLogger logger;

    EXPECT_NO_THROW(logger.Before("TestFunc"));
    EXPECT_NO_THROW(logger.After("TestFunc"));
}

// ============================================================
// AspectPerformance（性能计时切面）
// ============================================================

TEST(AspectTest, AspectPerformance_RecordsTime)
{
    // 验证 Constructor/Before/After 不崩溃
    AspectPerformance perf;

    EXPECT_NO_THROW(perf.Before("TimedFunc"));
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    EXPECT_NO_THROW(perf.After("TimedFunc"));
}

TEST(AspectTest, AspectPerformance_ResetTimeOnBefore)
{
    // Before 会重置开始时间，连续调用不应崩溃
    AspectPerformance perf;

    perf.Before("First");
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    perf.After("First");

    perf.Before("Second");
    perf.After("Second");
}

// ============================================================
// 综合测试：AspectLogger + AspectPerformance 组合
// ============================================================

TEST(AspectTest, CombinedLoggerAndPerformance)
{
    g_aspect_value = 0;

    auto wrapper = [](int x) { SetAspectValue(x); };
    Invoke<AspectLogger, AspectPerformance>(wrapper, "Combined", 123);

    EXPECT_EQ(g_aspect_value, 123);
}
