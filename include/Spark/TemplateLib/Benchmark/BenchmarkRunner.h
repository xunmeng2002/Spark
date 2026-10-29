#pragma once
#if defined(_MSC_VER)
#include <intrin.h>
#endif

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <cstddef>

namespace Spark::Benchmark
{
// 每轮目标时长：太短会被计时分辨率与调度噪声淹没，太长则一行要跑掉几十秒
constexpr double TargetRoundNanoseconds = 50'000'000.0;
constexpr long long ProbeCalls = 1000;
constexpr long long MinCallsPerRound = 1000;
constexpr long long MaxCallsPerRound = 50'000'000;
constexpr int SampleCount = 5;

// 纯优化屏障：让编译器认为该值被外部用掉了，也不得把纯函数提到循环外复用。
// 只防优化，不承担线程同步职责（同步一律用 std::mutex / std::atomic）
inline void KeepAlive(const void* valuePtr) noexcept
{
#if defined(_MSC_VER)
    _ReadWriteBarrier();
    (void)valuePtr;
#elif defined(__GNUC__) || defined(__clang__)
    __asm__ __volatile__("" ::"r"(valuePtr) : "memory");
#else
    (void)valuePtr;
#endif
}

inline void WarnIfUnoptimized()
{
#ifndef NDEBUG
    std::fprintf(stderr, "warning: unoptimized (Debug) build, numbers only verify the harness\n");
#endif
}

// 输出 CSV 以便跨提交/跨机器直接比对，不比对人眼对齐的文本表格
inline void PrintHeader()
{
    WarnIfUnoptimized();
    std::printf("# name,ns_per_call,ns_slowest,calls_per_round\n");
}

template <typename Func>
double MeasureOnce(Func& func, long long callsPerRound)
{
    const auto beginTime = std::chrono::steady_clock::now();
    for (long long call = 0; call < callsPerRound; ++call)
    {
        const size_t value = static_cast<size_t>(func());
        KeepAlive(&value);
    }
    const double elapsedNanoseconds = std::chrono::duration<double, std::nano>(std::chrono::steady_clock::now() - beginTime).count();
    return elapsedNanoseconds / static_cast<double>(callsPerRound);
}

// 先在短探测轮里估出单次成本，再把轮内次数凑到 TargetRoundNanoseconds：
// 写死次数时，25 ns 的接口一轮只有 25 ms，而 600 ns 的接口一轮要跑 10 分钟
template <typename Func>
long long EstimateCallsPerRound(Func& func)
{
    const double probeNanoseconds = MeasureOnce(func, ProbeCalls);
    if (probeNanoseconds <= 0.0)
    {
        return MaxCallsPerRound;
    }
    const double targetCalls = TargetRoundNanoseconds / probeNanoseconds;
    return static_cast<long long>(std::clamp(targetCalls, static_cast<double>(MinCallsPerRound), static_cast<double>(MaxCallsPerRound)));
}

// 测一行并打印 CSV。取 5 次的中位数，同时输出最慢的一次用于看抖动
template <typename Func>
void Measure(const char* name, Func&& func)
{
    // 预热：首次调用带页面缺页、TLS 初始化、localtime 首次读时区数据等一次性成本
    for (long long warmupCall = 0; warmupCall < ProbeCalls; ++warmupCall)
    {
        const size_t value = static_cast<size_t>(func());
        KeepAlive(&value);
    }
    const long long callsPerRound = EstimateCallsPerRound(func);
    double samples[SampleCount];
    for (int sample = 0; sample < SampleCount; ++sample)
    {
        samples[sample] = MeasureOnce(func, callsPerRound);
    }
    std::sort(samples, samples + SampleCount);
    std::printf("%s,%.1f,%.1f,%lld\n", name, samples[SampleCount / 2], samples[SampleCount - 1], callsPerRound);
}
}
