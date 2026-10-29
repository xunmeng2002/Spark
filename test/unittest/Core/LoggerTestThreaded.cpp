#include <Spark/Core/Logger/Logger.h>
#include "doctest/doctest.h"

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <string>
#include <system_error>
#include <thread>
#include <vector>
using namespace Spark::Core;

// ============================================================
// Logger 多线程写入测试
// 验证每线程块交接：行完整、单线程内有序、超长行不粘连、线程退出不丢日志、不满一块的行运行期可见
// ============================================================

namespace
{
// 在临时目录里启动真实日志线程；析构时停线程、恢复全局状态并清理目录
class LoggerSession
{
public:
    LoggerSession()
    {
        workDir_ = std::filesystem::temp_directory_path() /
                   ("SparkLoggerTest-" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
        std::filesystem::create_directories(workDir_);
        originalDir_ = std::filesystem::current_path();
        std::filesystem::current_path(workDir_);

        savedLogLevel_ = Logger::GetLogLevel();
        savedConsoleLevel_ = Logger::GetConsoleLogLevel();
        savedWriteLogFunc_ = Logger::GetWriteLogFunc();
        Logger::GetInstance().SetLogLevel(LogLevel::Debug, LogLevel::Emergency);
        Logger::GetInstance().Init("UnitTests.exe");
        Logger::GetInstance().Start();
    }
    ~LoggerSession()
    {
        StopAndJoin();
        Logger::SetExternLogger(savedWriteLogFunc_);
        Logger::GetInstance().SetLogLevel(savedLogLevel_, savedConsoleLevel_);

        std::error_code errorCode;
        std::filesystem::current_path(originalDir_, errorCode);
        std::filesystem::remove_all(workDir_, errorCode);
    }

    void StopAndJoin()
    {
        Logger::GetInstance().Stop();
        Logger::GetInstance().Join();
    }

    std::vector<std::string> ReadLogLines()
    {
        std::vector<std::string> lines;
        for (const auto& entry : std::filesystem::directory_iterator("log"))
        {
            std::ifstream logStream(entry.path());
            std::string line;
            while (std::getline(logStream, line))
            {
                lines.push_back(line);
            }
        }
        return lines;
    }

private:
    std::filesystem::path workDir_;
    std::filesystem::path originalDir_;
    LogLevel savedLogLevel_;
    LogLevel savedConsoleLevel_;
    WriteLogFunc savedWriteLogFunc_;
};
}

TEST_CASE("LoggerThreadedTest.MultiThreadLinesCompleteAndOrdered")
{
    constexpr int ThreadCount = 4;
    constexpr int LinesPerThread = 4000;
    LoggerSession session;

    std::vector<std::thread> writerThreads;
    writerThreads.reserve(ThreadCount);
    for (int threadIndex = 0; threadIndex < ThreadCount; ++threadIndex)
    {
        writerThreads.emplace_back(
            [threadIndex]
            {
                for (int lineIndex = 0; lineIndex < LinesPerThread; ++lineIndex)
                {
                    WriteLog(LogLevel::Info, "MT %d %d", threadIndex, lineIndex);
                }
            });
    }
    for (auto& writerThread : writerThreads)
    {
        writerThread.join();
    }
    session.StopAndJoin();

    std::vector<int> nextLineIndex(ThreadCount, 0);
    int matchedLineCount = 0;
    for (const auto& line : session.ReadLogLines())
    {
        const size_t markerPos = line.find("MT ");
        if (markerPos == std::string::npos)
        {
            continue;
        }
        int threadIndex = -1;
        int lineIndex = -1;
        if (std::sscanf(line.c_str() + markerPos, "MT %d %d", &threadIndex, &lineIndex) != 2)
        {
            continue;
        }
        REQUIRE(threadIndex >= 0);
        REQUIRE(threadIndex < ThreadCount);
        // 单线程内行序必须严格递增：块交接不得重排或丢行
        CHECK_EQ(lineIndex, nextLineIndex[threadIndex]);
        ++nextLineIndex[threadIndex];
        ++matchedLineCount;
    }

    CHECK_EQ(matchedLineCount, ThreadCount * LinesPerThread);
    for (int threadIndex = 0; threadIndex < ThreadCount; ++threadIndex)
    {
        CHECK_EQ(nextLineIndex[threadIndex], LinesPerThread);
    }
}

TEST_CASE("LoggerThreadedTest.OversizedLineDoesNotCorruptNeighbors")
{
    LoggerSession session;

    const std::string oversizedText(70000, 'X');
    WriteLog(LogLevel::Info, "BIG %s", oversizedText.c_str());
    WriteLog(LogLevel::Info, "AFTERBIG marker");
    session.StopAndJoin();

    const std::vector<std::string> lines = session.ReadLogLines();
    size_t bigLinePos = lines.size();
    for (size_t lineIndex = 0; lineIndex < lines.size(); ++lineIndex)
    {
        if (lines[lineIndex].find("BIG ") != std::string::npos)
        {
            bigLinePos = lineIndex;
            break;
        }
    }
    REQUIRE(bigLinePos < lines.size());
    // 单行被截断到上限内，且整行落在一个块里，不与其他行粘连
    CHECK(lines[bigLinePos].size() <= 64 * 1024);
    CHECK(lines[bigLinePos].size() > 60000);
    // 跨线程按块交错，哨兵行只需出现在超长行之后，不要求紧邻（Logger 自身线程的行可能插在中间）
    REQUIRE(bigLinePos + 1 < lines.size());
    bool afterBigFound = false;
    for (size_t lineIndex = bigLinePos + 1; lineIndex < lines.size(); ++lineIndex)
    {
        if (lines[lineIndex].find("AFTERBIG marker") != std::string::npos)
        {
            afterBigFound = true;
            break;
        }
    }
    CHECK(afterBigFound);
}

TEST_CASE("LoggerThreadedTest.ExitingThreadFlushesItsLine")
{
    LoggerSession session;
    {
        std::thread exitingThread([] { WriteLog(LogLevel::Info, "EXITTHREAD marker"); });
        exitingThread.join();
    }
    session.StopAndJoin();

    const std::vector<std::string> lines = session.ReadLogLines();
    const bool lineFound =
        std::any_of(lines.begin(), lines.end(), [](const std::string& line) { return line.find("EXITTHREAD marker") != std::string::npos; });
    CHECK(lineFound);
}

TEST_CASE("LoggerThreadedTest.LineBecomesVisibleWithoutStop")
{
    LoggerSession session;
    WriteLog(LogLevel::Info, "LIVELINE marker");

    // 不满一块的行也必须在运行期送达文件，不能只等到 Stop 才落盘
    constexpr auto timeout = std::chrono::seconds(3);
    const auto beginTime = std::chrono::steady_clock::now();
    bool lineFound = false;
    while (!lineFound && std::chrono::steady_clock::now() - beginTime < timeout)
    {
        const std::vector<std::string> lines = session.ReadLogLines();
        lineFound =
            std::any_of(lines.begin(), lines.end(), [](const std::string& line) { return line.find("LIVELINE marker") != std::string::npos; });
        if (!lineFound)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }
    CHECK(lineFound);
    session.StopAndJoin();
}

TEST_CASE("LoggerThreadedTest.WriteAfterStopIsDiscarded")
{
    LoggerSession session;
    WriteLog(LogLevel::Info, "BEFORESTOP marker");
    session.StopAndJoin();
    CHECK_NOTHROW(WriteLog(LogLevel::Info, "AFTERSTOP marker"));

    const std::vector<std::string> lines = session.ReadLogLines();
    const bool beforeStopFound =
        std::any_of(lines.begin(), lines.end(), [](const std::string& line) { return line.find("BEFORESTOP marker") != std::string::npos; });
    const bool afterStopFound =
        std::any_of(lines.begin(), lines.end(), [](const std::string& line) { return line.find("AFTERSTOP marker") != std::string::npos; });
    CHECK(beforeStopFound);
    CHECK_FALSE(afterStopFound);
}
