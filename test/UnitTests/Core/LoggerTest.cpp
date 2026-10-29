#include <Spark/Core/Logger/Logger.h>
#include "doctest/doctest.h"

#include <cstdarg>
#include <cstdio>
#include <string>
#include <cstring>
using namespace Spark::Core;
// ============================================================
// Logger 测试
// 测试日志单例、级别设置、外部日志回调
// ============================================================

namespace
{
// 测试辅助：记录最后一次 WriteLog 调用
static LogLevel LastLevel = LogLevel::Ignore;
static char LastMessage[1024] = {};

void TestLogCallback(LogLevel level, const char* fileName, int lineNo, const char* funcName, const char* formatStr, ...)
{
    LastLevel = level;
    va_list va;
    va_start(va, formatStr);
    vsnprintf(LastMessage, sizeof(LastMessage), formatStr, va);
    va_end(va);
}

void ClearTestCallback()
{
    LastLevel = LogLevel::Ignore;
    LastMessage[0] = '\0';
}
}

// ---------- 单例 ----------

TEST_CASE("LoggerTest.GetInstance_SameInstance")
{
    Logger& instance1 = Logger::GetInstance();
    Logger& instance2 = Logger::GetInstance();

    CHECK_EQ(&instance1, &instance2);
}

// ---------- 默认日志级别 ----------

TEST_CASE("LoggerTest.DefaultLogLevel")
{
    // 默认日志级别为 Info
    CHECK_EQ(Logger::GetLogLevel(), LogLevel::Info);
}

TEST_CASE("LoggerTest.DefaultConsoleLogLevel")
{
    // 默认控制台级别为 Warning
    CHECK_EQ(Logger::GetConsoleLogLevel(), LogLevel::Warning);
}

// ---------- SetLogLevel ----------

TEST_CASE("LoggerTest.SetLogLevel_Info")
{
    LogLevel saved_level = Logger::GetLogLevel();
    LogLevel saved_console = Logger::GetConsoleLogLevel();

    Logger::GetInstance().SetLogLevel(LogLevel::Info, LogLevel::Warning);
    CHECK_EQ(Logger::GetLogLevel(), LogLevel::Info);
    CHECK_EQ(Logger::GetConsoleLogLevel(), LogLevel::Warning);

    // 恢复
    Logger::GetInstance().SetLogLevel(saved_level, saved_console);
}

TEST_CASE("LoggerTest.SetLogLevel_Debug")
{
    LogLevel saved_level = Logger::GetLogLevel();
    LogLevel saved_console = Logger::GetConsoleLogLevel();

    Logger::GetInstance().SetLogLevel(LogLevel::Debug, LogLevel::Debug);
    CHECK_EQ(Logger::GetLogLevel(), LogLevel::Debug);
    CHECK_EQ(Logger::GetConsoleLogLevel(), LogLevel::Debug);

    Logger::GetInstance().SetLogLevel(saved_level, saved_console);
}

TEST_CASE("LoggerTest.SetLogLevel_ErrorOnly")
{
    LogLevel saved_level = Logger::GetLogLevel();
    LogLevel saved_console = Logger::GetConsoleLogLevel();

    Logger::GetInstance().SetLogLevel(LogLevel::Error, LogLevel::Emergency);
    CHECK_EQ(Logger::GetLogLevel(), LogLevel::Error);
    CHECK_EQ(Logger::GetConsoleLogLevel(), LogLevel::Emergency);

    Logger::GetInstance().SetLogLevel(saved_level, saved_console);
}

// ---------- 外部日志回调 ----------

TEST_CASE("LoggerTest.GetWriteLogFunc_DefaultIsNull")
{
    CHECK_EQ(Logger::GetWriteLogFunc(), nullptr);
}

TEST_CASE("LoggerTest.SetExternLogger_CallbackIsCalled")
{
    ClearTestCallback();

    // 保存原状态
    WriteLogFunc saved_func = Logger::GetWriteLogFunc();

    // 注册测试回调
    Logger::SetExternLogger(TestLogCallback);
    CHECK_EQ(Logger::GetWriteLogFunc(), TestLogCallback);

    // 通过 WriteLog 宏调用（设计上日志入口只有宏）
    WriteLog(LogLevel::Info, "Test message %d", 42);

    CHECK_EQ(LastLevel, LogLevel::Info);
    CHECK_NE(std::strlen(LastMessage), 0);

    // 恢复
    Logger::SetExternLogger(saved_func);
}

TEST_CASE("LoggerTest.SetExternLogger_MultipleLevels")
{
    ClearTestCallback();

    WriteLogFunc saved_func = Logger::GetWriteLogFunc();
    Logger::SetExternLogger(TestLogCallback);

    WriteLog(LogLevel::Warning, "Warning test");
    CHECK_EQ(LastLevel, LogLevel::Warning);

    WriteLog(LogLevel::Error, "Error test");
    CHECK_EQ(LastLevel, LogLevel::Error);

    WriteLog(LogLevel::Debug, "Debug test");
    CHECK_EQ(LastLevel, LogLevel::Debug);

    Logger::SetExternLogger(saved_func);
}

// ---------- WriteLog 宏 ----------

TEST_CASE("LoggerTest.WriteLogMacro_WithRegisteredCallback")
{
    ClearTestCallback();

    WriteLogFunc saved_func = Logger::GetWriteLogFunc();
    Logger::SetExternLogger(TestLogCallback);

    // WriteLog 宏在回调存在时调用回调
    WriteLog(LogLevel::Critical, "Macro test %s %d", "hello", 99);

    CHECK_EQ(LastLevel, LogLevel::Critical);
    CHECK_NE(std::strlen(LastMessage), 0);

    Logger::SetExternLogger(saved_func);
}

TEST_CASE("LoggerTest.WriteLogMacro_NoCallbackDoesNothing")
{
    // 确保回调为空时不崩溃
    WriteLogFunc saved_func = Logger::GetWriteLogFunc();
    Logger::SetExternLogger(nullptr);

    CHECK_NOTHROW(WriteLog(LogLevel::Info, "Should not crash"));
    CHECK_NOTHROW(WriteErrorLog(1001, "Should not crash either"));

    Logger::SetExternLogger(saved_func);
}

// ---------- WriteErrorLog 宏 ----------

TEST_CASE("LoggerTest.WriteErrorLogMacro")
{
    ClearTestCallback();

    WriteLogFunc saved_func = Logger::GetWriteLogFunc();
    Logger::SetExternLogger(TestLogCallback);

    WriteErrorLog(404, "Not found");

    CHECK_EQ(LastLevel, LogLevel::Error);

    Logger::SetExternLogger(saved_func);
}

// ---------- 恢复默认状态 ----------

TEST_CASE("LoggerTest.RestoreDefaultAfterTests")
{
    // 确保不影响后续测试
    Logger::GetInstance().SetLogLevel(LogLevel::Info, LogLevel::Warning);
    Logger::SetExternLogger(nullptr);

    CHECK_EQ(Logger::GetLogLevel(), LogLevel::Info);
    CHECK_EQ(Logger::GetConsoleLogLevel(), LogLevel::Warning);
    CHECK_EQ(Logger::GetWriteLogFunc(), nullptr);
}
