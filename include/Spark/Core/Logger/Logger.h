#pragma once
#include <Spark/Core/CoreExport.h>
#include <Spark/Core/Thread/ThreadBase.h>
#include <list>
#include <vector>
#include <map>
#include <mutex>
#include <condition_variable>

namespace Spark::Core
{
enum class LogLevel : int
{
    Ignore = 0,
    Debug = 1,
    Info = 2,
    Warning = 3,
    Error = 4,
    Critical = 5,
    Emergency = 6,
};

typedef void (*WriteLogFunc)(LogLevel level, const char* fileName, int lineNo, const char* funcName, const char* formatStr, ...);

struct LogData;
class CORE_EXPORTS Logger : public ThreadBase
{
private:
    Logger();
    ~Logger();
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

public:
    static Logger& GetInstance();
    static WriteLogFunc& GetWriteLogFunc();
    static LogLevel& GetLogLevel();
    static LogLevel& GetConsoleLogLevel();
    bool Init(const char* fullProcessName);
    void SetLogLevel(LogLevel logLevel = LogLevel::Info, LogLevel logLevelConsole = LogLevel::Warning);
    static void SetExternLogger(WriteLogFunc externLogger);

protected:
    virtual void ThreadInit() override;
    virtual void ThreadExit() override;
    virtual void Run() override;

    bool CreateLogDir(const std::string& path);
    void SwapInnerLogBuffers();
    void FlushBuffers();
    void FlushRemainingBuffers();
    static void Write(LogLevel level, const char* file, int line, const char* func, const char* formatStr, ...);
    void WriteToLog(LogLevel level, const char* file, int line, const char* func, const char* format, va_list va);
    void WriteToConsole(LogLevel level, const char* formatStr, va_list va);
    void CreateLogFile();
    long long GetCurrentThreadId();

private:
    char processName_[128];
    tm createLogFileTime_;
    LogData* logData_;
};

#define WriteLog(level, formatStr, ...) \
    if (Spark::Core::Logger::GetWriteLogFunc() != nullptr) \
        Spark::Core::Logger::GetWriteLogFunc()(level, __FILE__, __LINE__, __func__, formatStr, ##__VA_ARGS__);

#define WriteErrorLog(errorId, errorMsg) \
    if (Spark::Core::Logger::GetWriteLogFunc() != nullptr) \
        Spark::Core::Logger::GetWriteLogFunc()(Spark::Core::LogLevel::Error, __FILE__, __LINE__, __func__, "ErrorId:[%d], ErrorMsg:[%s].", errorId, \
                                               errorMsg);
}
