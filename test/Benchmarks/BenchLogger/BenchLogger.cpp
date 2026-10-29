#include "BenchmarkRunner.h"

#include <Spark/Core/Logger/Logger.h>

#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <string>
#include <thread>
#include <vector>

using namespace Spark::Core;

namespace
{
double ToMilliseconds(const std::chrono::steady_clock::time_point& beginTime)
{
    return std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - beginTime).count();
}
}

// 用法：LoggerBench [线程数] [每线程行数] [工作目录]
// 日志文件落在工作目录（默认 out/bench-work，已被 .gitignore 覆盖），不污染仓库根目录
int main(int argc, char* argv[])
{
    Benchmark::WarnIfUnoptimized();
    const int threadCount = argc > 1 ? std::atoi(argv[1]) : 4;
    const int linesPerThread = argc > 2 ? std::atoi(argv[2]) : 100000;
    const char* workRoot = argc > 3 ? argv[3] : "out/bench-work";

    std::error_code errorCode;
    std::filesystem::create_directories(workRoot, errorCode);
    std::filesystem::current_path(workRoot, errorCode);
    if (errorCode)
    {
        std::fprintf(stderr, "cannot enter work directory %s: %s\n", workRoot, errorCode.message().c_str());
        return 1;
    }

    Logger::GetInstance().SetLogLevel(LogLevel::Debug, LogLevel::Emergency);
    Logger::GetInstance().Init("LoggerBench");
    Logger::GetInstance().Start();

    std::vector<std::thread> writers;
    writers.reserve(static_cast<size_t>(threadCount));
    const auto beginTime = std::chrono::steady_clock::now();
    for (int threadIndex = 0; threadIndex < threadCount; ++threadIndex)
    {
        writers.emplace_back(
            [threadIndex, linesPerThread]()
            {
                for (int lineIndex = 0; lineIndex < linesPerThread; ++lineIndex)
                {
                    WriteLog(LogLevel::Info, "BENCH %d %d payload", threadIndex, lineIndex);
                }
            });
    }
    for (auto& writer : writers)
    {
        writer.join();
    }
    const double writeMilliseconds = ToMilliseconds(beginTime);

    Logger::GetInstance().Stop();
    Logger::GetInstance().Join();
    const double totalMilliseconds = ToMilliseconds(beginTime);

    const double totalLines = static_cast<double>(threadCount) * static_cast<double>(linesPerThread);
    std::printf("# threads,lines_per_thread,write_ms,total_ms,write_lines_per_sec\n");
    std::printf("%d,%d,%.1f,%.1f,%.0f\n", threadCount, linesPerThread, writeMilliseconds, totalMilliseconds, totalLines * 1000.0 / writeMilliseconds);
    return 0;
}
