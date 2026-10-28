#include <Spark/Core/Aspect/Aspect.h>
#include <Spark/Core/Aspect/AspectBefore.h>
#include <Spark/Core/Aspect/AspectAfter.h>
#include <Spark/Core/Aspect/AspectLogger.h>
#include <Spark/Core/Aspect/AspectPerformance.h>
#include "doctest/doctest.h"

#include <string>
#include <vector>
#include <chrono>
#include <thread>
using namespace Spark::Core;
// ============================================================
// Aspect 测试
// AOP 切面模板：Before/After 调用顺序、多切面链式调用
// ============================================================

namespace
{
// 测试辅助切面：记录 Before/After 调用轨迹
struct CallTracer
{
    void Before(const char* funcName) { calls.push_back(std::string(funcName) + "_Before"); }

    void After(const char* funcName) { calls.push_back(std::string(funcName) + "_After"); }
    std::vector<std::string> calls;
};

int AspectValue = 0;
std::string AspectTrace;

void SetAspectValue(int x)
{
    AspectValue = x;
}

int DoubleAspectValue(int x)
{
    return x * 2;
}

void AppendTrace(const std::string& tag)
{
    AspectTrace += tag;
}
}

// ============================================================
// Aspect（通用模板：Before + After）
// ============================================================

TEST_CASE("AspectTest.SingleAspect_CallOrder")
{
    AspectValue = 0;
    CallTracer tracer;

    auto wrapper = [](int x) { SetAspectValue(x); };
    Aspect<decltype(wrapper), int> aspect(std::move(wrapper), "SetValue");
    aspect.Invoke(42, tracer);

    CHECK_EQ(AspectValue, 42);
    REQUIRE_EQ(tracer.calls.size(), 2);
    CHECK_EQ(tracer.calls[0], "SetValue_Before");
    CHECK_EQ(tracer.calls[1], "SetValue_After");
}

TEST_CASE("AspectTest.MultipleAspects_NestingOrder")
{
    AspectTrace.clear();
    CallTracer outer;
    CallTracer inner;

    // 传递顺序：outer 先入，inner 后入
    // 调用顺序：outer.Before → inner.Before → Func → inner.After → outer.After
    auto wrapper = [](const std::string& tag) { AppendTrace(tag); };
    Aspect<decltype(wrapper), const std::string&> aspect(std::move(wrapper), "Multi");
    aspect.Invoke("func", outer, inner);

    REQUIRE_EQ(outer.calls.size(), 2);
    CHECK_EQ(outer.calls[0], "Multi_Before");
    CHECK_EQ(outer.calls[1], "Multi_After");

    REQUIRE_EQ(inner.calls.size(), 2);
    CHECK_EQ(inner.calls[0], "Multi_Before");
    CHECK_EQ(inner.calls[1], "Multi_After");

    CHECK_EQ(AspectTrace, "func");
}

TEST_CASE("AspectTest.InvokeFreeFunction_SingleAspect")
{
    AspectValue = 0;

    auto wrapper = [](int x) { SetAspectValue(x); };
    Invoke<CallTracer>(wrapper, "FreeInvoke", 99);

    CHECK_EQ(AspectValue, 99);
}

TEST_CASE("AspectTest.InvokeFreeFunction_MultipleAspects")
{
    AspectTrace.clear();

    auto wrapper = [](const std::string& tag) { AppendTrace(tag); };
    Invoke<CallTracer, CallTracer>(wrapper, "FreeMulti", "trace");

    CHECK_EQ(AspectTrace, "trace");
}

TEST_CASE("AspectTest.FunctionWithReturnValue")
{
    auto wrapper = [](int x) -> int { return DoubleAspectValue(x); };
    Aspect<decltype(wrapper), int> aspect(std::move(wrapper), "Double");
    CallTracer tracer;

    // 无方面版本直接调用
    int result = 0;
    auto wrapper2 = [&](int x) { result = DoubleAspectValue(x); };
    Aspect<decltype(wrapper2), int> aspect2(std::move(wrapper2), "Double");
    aspect2.Invoke(21, tracer);

    CHECK_EQ(result, 42);
}

// ============================================================
// AspectBefore（仅 Before 切面）
// ============================================================

TEST_CASE("AspectTest.AspectBefore_Single")
{
    AspectValue = 0;
    CallTracer tracer;

    auto wrapper = [](int x) { SetAspectValue(x); };
    AspectBefore<decltype(wrapper), int> aspect(wrapper, "BeforeOnly");
    aspect.InvokeBefore(77, tracer);

    CHECK_EQ(AspectValue, 77);
    REQUIRE_EQ(tracer.calls.size(), 1);
    CHECK_EQ(tracer.calls[0], "BeforeOnly_Before");
}

TEST_CASE("AspectTest.AspectBefore_Multiple")
{
    AspectTrace.clear();

    // AP... 推演为非引用类型，自由函数避免了传左值引用导致 AP() 构造失败的问题
    auto wrapper = [](const std::string& tag) { AppendTrace(tag); };
    InvokeBefore<CallTracer, CallTracer>(wrapper, "MultiBefore", "ok");

    CHECK_EQ(AspectTrace, "ok");
}

TEST_CASE("AspectTest.InvokeBeforeFreeFunction")
{
    AspectValue = 0;

    auto wrapper = [](int x) { SetAspectValue(x); };
    InvokeBefore<CallTracer>(wrapper, "InvokeBefore", 55);

    CHECK_EQ(AspectValue, 55);
}

// ============================================================
// AspectAfter（仅 After 切面）
// ============================================================

TEST_CASE("AspectTest.AspectAfter_Single")
{
    AspectValue = 0;
    CallTracer tracer;

    auto wrapper = [](int x) { SetAspectValue(x); };
    AspectAfter<decltype(wrapper), int> aspect(wrapper, "AfterOnly");
    aspect.InvokeAfter(33, tracer);

    CHECK_EQ(AspectValue, 33);
    REQUIRE_EQ(tracer.calls.size(), 1);
    CHECK_EQ(tracer.calls[0], "AfterOnly_After");
}

TEST_CASE("AspectTest.AspectAfter_Multiple")
{
    AspectTrace.clear();

    // 自由函数创建右值切面实例
    auto wrapper = [](const std::string& tag) { AppendTrace(tag); };
    InvokeAfter<CallTracer, CallTracer>(wrapper, "MultiAfter", "done");

    CHECK_EQ(AspectTrace, "done");
}

TEST_CASE("AspectTest.InvokeAfterFreeFunction")
{
    AspectValue = 0;

    auto wrapper = [](int x) { SetAspectValue(x); };
    InvokeAfter<CallTracer>(wrapper, "InvokeAfter", 11);

    CHECK_EQ(AspectValue, 11);
}

// ============================================================
// AspectLogger（真实日志切面）
// ============================================================

TEST_CASE("AspectTest.AspectLogger_BeforeAfter")
{
    // AspectLogger::Before/After 内部调用 WriteLog
    // WriteLog 在 GetWriteLogFunc() 为空时直接跳过
    // 验证不崩溃即可
    AspectLogger logger;

    CHECK_NOTHROW(logger.Before("TestFunc"));
    CHECK_NOTHROW(logger.After("TestFunc"));
}

// ============================================================
// AspectPerformance（性能计时切面）
// ============================================================

TEST_CASE("AspectTest.AspectPerformance_RecordsTime")
{
    // 验证 Constructor/Before/After 不崩溃
    AspectPerformance perf;

    CHECK_NOTHROW(perf.Before("TimedFunc"));
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    CHECK_NOTHROW(perf.After("TimedFunc"));
}

TEST_CASE("AspectTest.AspectPerformance_ResetTimeOnBefore")
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

TEST_CASE("AspectTest.CombinedLoggerAndPerformance")
{
    AspectValue = 0;

    auto wrapper = [](int x) { SetAspectValue(x); };
    Invoke<AspectLogger, AspectPerformance>(wrapper, "Combined", 123);

    CHECK_EQ(AspectValue, 123);
}
