#include <gtest/gtest.h>
#include <Spark/Core/Logger/Logger.h>

#include <cstdarg>
#include <cstdio>
#include <string>
#include <cstring>
using namespace spark::core;
// ============================================================
// Logger 测试
// 测试日志单例、级别设置、外部日志回调
// ============================================================

namespace
{
    // 测试辅助：记录最后一次 WriteLog 调用
    static LogLevel s_last_level = LogLevel::Ignore;
    static char s_last_message[1024] = {};

    void TestLogCallback(LogLevel level, const char* fileName, int lineNo,
                         const char* funcName, const char* formatStr, ...)
    {
        s_last_level = level;
        va_list va;
        va_start(va, formatStr);
        vsnprintf(s_last_message, sizeof(s_last_message), formatStr, va);
        va_end(va);
    }

    void ClearTestCallback()
    {
        s_last_level = LogLevel::Ignore;
        s_last_message[0] = '\0';
    }
}

// ---------- 单例 ----------

TEST(LoggerTest, GetInstance_SameInstance)
{
    Logger& instance1 = Logger::GetInstance();
    Logger& instance2 = Logger::GetInstance();

    EXPECT_EQ(&instance1, &instance2);
}

// ---------- 默认日志级别 ----------

TEST(LoggerTest, DefaultLogLevel)
{
    // 默认日志级别为 Info
    EXPECT_EQ(Logger::GetLogLevel(), LogLevel::Info);
}

TEST(LoggerTest, DefaultConsoleLogLevel)
{
    // 默认控制台级别为 Warning
    EXPECT_EQ(Logger::GetConsoleLogLevel(), LogLevel::Warning);
}

// ---------- SetLogLevel ----------

TEST(LoggerTest, SetLogLevel_Info)
{
    LogLevel saved_level = Logger::GetLogLevel();
    LogLevel saved_console = Logger::GetConsoleLogLevel();

    Logger::GetInstance().SetLogLevel(LogLevel::Info, LogLevel::Warning);
    EXPECT_EQ(Logger::GetLogLevel(), LogLevel::Info);
    EXPECT_EQ(Logger::GetConsoleLogLevel(), LogLevel::Warning);

    // 恢复
    Logger::GetInstance().SetLogLevel(saved_level, saved_console);
}

TEST(LoggerTest, SetLogLevel_Debug)
{
    LogLevel saved_level = Logger::GetLogLevel();
    LogLevel saved_console = Logger::GetConsoleLogLevel();

    Logger::GetInstance().SetLogLevel(LogLevel::Debug, LogLevel::Debug);
    EXPECT_EQ(Logger::GetLogLevel(), LogLevel::Debug);
    EXPECT_EQ(Logger::GetConsoleLogLevel(), LogLevel::Debug);

    Logger::GetInstance().SetLogLevel(saved_level, saved_console);
}

TEST(LoggerTest, SetLogLevel_ErrorOnly)
{
    LogLevel saved_level = Logger::GetLogLevel();
    LogLevel saved_console = Logger::GetConsoleLogLevel();

    Logger::GetInstance().SetLogLevel(LogLevel::Error, LogLevel::Emergency);
    EXPECT_EQ(Logger::GetLogLevel(), LogLevel::Error);
    EXPECT_EQ(Logger::GetConsoleLogLevel(), LogLevel::Emergency);

    Logger::GetInstance().SetLogLevel(saved_level, saved_console);
}

// ---------- 外部日志回调 ----------

TEST(LoggerTest, GetWriteLogFunc_DefaultIsNull)
{
    EXPECT_EQ(Logger::GetWriteLogFunc(), nullptr);
}

TEST(LoggerTest, SetExternLogger_CallbackIsCalled)
{
    ClearTestCallback();

    // 保存原状态
    WriteLogFunc saved_func = Logger::GetWriteLogFunc();

    // 注册测试回调
    Logger::SetExternLogger(TestLogCallback);
    EXPECT_EQ(Logger::GetWriteLogFunc(), TestLogCallback);

    // 通过 WriteLog 宏调用（设计上日志入口只有宏）
    WriteLog(LogLevel::Info, "Test message %d", 42);

    EXPECT_EQ(s_last_level, LogLevel::Info);
    EXPECT_NE(std::strlen(s_last_message), 0);

    // 恢复
    Logger::SetExternLogger(saved_func);
}

TEST(LoggerTest, SetExternLogger_MultipleLevels)
{
    ClearTestCallback();

    WriteLogFunc saved_func = Logger::GetWriteLogFunc();
    Logger::SetExternLogger(TestLogCallback);

    WriteLog(LogLevel::Warning, "Warning test");
    EXPECT_EQ(s_last_level, LogLevel::Warning);

    WriteLog(LogLevel::Error, "Error test");
    EXPECT_EQ(s_last_level, LogLevel::Error);

    WriteLog(LogLevel::Debug, "Debug test");
    EXPECT_EQ(s_last_level, LogLevel::Debug);

    Logger::SetExternLogger(saved_func);
}

// ---------- WriteLog 宏 ----------

TEST(LoggerTest, WriteLogMacro_WithRegisteredCallback)
{
    ClearTestCallback();

    WriteLogFunc saved_func = Logger::GetWriteLogFunc();
    Logger::SetExternLogger(TestLogCallback);

    // WriteLog 宏在回调存在时调用回调
    WriteLog(LogLevel::Critical, "Macro test %s %d", "hello", 99);

    EXPECT_EQ(s_last_level, LogLevel::Critical);
    EXPECT_NE(std::strlen(s_last_message), 0);

    Logger::SetExternLogger(saved_func);
}

TEST(LoggerTest, WriteLogMacro_NoCallbackDoesNothing)
{
    // 确保回调为空时不崩溃
    WriteLogFunc saved_func = Logger::GetWriteLogFunc();
    Logger::SetExternLogger(nullptr);

    EXPECT_NO_THROW(WriteLog(LogLevel::Info, "Should not crash"));
    EXPECT_NO_THROW(WriteErrorLog(1001, "Should not crash either"));

    Logger::SetExternLogger(saved_func);
}

// ---------- WriteErrorLog 宏 ----------

TEST(LoggerTest, WriteErrorLogMacro)
{
    ClearTestCallback();

    WriteLogFunc saved_func = Logger::GetWriteLogFunc();
    Logger::SetExternLogger(TestLogCallback);

    WriteErrorLog(404, "Not found");

    EXPECT_EQ(s_last_level, LogLevel::Error);

    Logger::SetExternLogger(saved_func);
}

// ---------- 恢复默认状态 ----------

TEST(LoggerTest, RestoreDefaultAfterTests)
{
    // 确保不影响后续测试
    Logger::GetInstance().SetLogLevel(LogLevel::Info, LogLevel::Warning);
    Logger::SetExternLogger(nullptr);

    EXPECT_EQ(Logger::GetLogLevel(), LogLevel::Info);
    EXPECT_EQ(Logger::GetConsoleLogLevel(), LogLevel::Warning);
    EXPECT_EQ(Logger::GetWriteLogFunc(), nullptr);
}
