#include <Spark/Core/Logger/Logger.h>
#include <Spark/TemplateLib/Buffer/LinearBuffer.h>
#include <Spark/Core/Utility/Utility.h>
#include <Spark/Core/Utility/TimeUtility.h>
#include "Logger/LogData.h"
#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <sstream>
#include <stdarg.h>
#include <filesystem>
#include <format>
#include <system_error>
#ifdef _WIN32
#include <Windows.h>
#endif

namespace Spark::Core
{
constexpr unsigned int LogLineLength = 64 * 1024;
constexpr unsigned int MaxLogFormatLength = 1024;
constexpr unsigned int MaxLogLineContentLength = (LogLineLength - MaxLogFormatLength);
static_assert(LogBufferSize >= LogLineLength, "一条日志必须整行落进一个块");

// 小块交接的最小间隔：交接后写入线程要重新分配并清零一整块，交接过密会把成本摊到每一行上
constexpr std::chrono::milliseconds ChunkHandOffInterval{20};

namespace
{
// 停止后线程 TLS 析构不得再解引用已销毁的 LogData
std::atomic<bool> LoggerActive{false};
}

// 线程退出时注销本线程写入器；Writer 可能属于已销毁的旧一代 LogData，故只在代次匹配时解引用
struct LogThreadWriterHolder
{
public:
    ~LogThreadWriterHolder();

    std::uint64_t Generation = 0;
    LogThreadWriter* Writer = nullptr;
};
static thread_local LogThreadWriterHolder ThreadLogWriterState;

static std::map<LogLevel, std::string> LogLevelName = {
    {LogLevel::Ignore, "IGNORE"}, {LogLevel::Debug, "DEBUG"},       {LogLevel::Info, "INFO"},           {LogLevel::Warning, "WARNING"},
    {LogLevel::Error, "ERROR"},   {LogLevel::Critical, "CRITICAL"}, {LogLevel::Emergency, "EMERGENCY"},
};

thread_local char LogBuffer[LogLineLength];

Logger::Logger() : ThreadBase("Logger"), processName_(""), createLogFileTime_(), logData_(nullptr) {}
Logger::~Logger() {}

Logger& Logger::GetInstance()
{
    static Logger instance;
    return instance;
}
LogThreadWriterHolder::~LogThreadWriterHolder()
{
    if (Writer != nullptr && LoggerActive.load(std::memory_order_acquire))
    {
        Logger::GetInstance().RetireThreadLogWriter();
    }
    Writer = nullptr;
}
WriteLogFunc& Logger::GetWriteLogFunc()
{
    static WriteLogFunc externLogger = nullptr;
    return externLogger;
}
LogLevel& Logger::GetLogLevel()
{
    static LogLevel logLevel = LogLevel::Info;
    return logLevel;
}
LogLevel& Logger::GetConsoleLogLevel()
{
    static LogLevel consoleLogLevel = LogLevel::Warning;
    return consoleLogLevel;
}
bool Logger::Init(const char* fullProcessName)
{
    Utility::ParseProcessName(fullProcessName, processName_, 128);
    logData_ = new LogData();
    // 日志目录建不出来、或日志文件打不开，等于整个进程"无日志运行"：早失败并把原因说到能照着排查，
    // 不让宿主带着看不见的故障跑起来。此处在 Start() 之前，没有已建立的状态需要收尾
    if (!CreateLogDir("log"))
    {
        fprintf(stderr, "Logger: create log directory failed, process exit. Path:log, check that it is not occupied by a file and is writable.\n");
        std::exit(EXIT_FAILURE);
    }
    // 启动期就把日志文件打开（而非留到线程里的 ThreadInit）：能否写日志只有在这一刻判定才叫"启动失败"
    createLogFileTime_ = TimeUtility::GetLocalTm();
    CreateLogFile();
    if (logData_->LogFile == nullptr)
    {
        fprintf(stderr, "Logger: cannot write log file, process exit. Check the log directory free space and write permission.\n");
        std::exit(EXIT_FAILURE);
    }

    LoggerActive.store(true, std::memory_order_release);
    GetWriteLogFunc() = Logger::Write;
    return true;
}
void Logger::SetLogLevel(LogLevel logLevel, LogLevel logLevelConsole)
{
    GetLogLevel() = logLevel;
    GetConsoleLogLevel() = logLevelConsole;
}
void Logger::SetExternLogger(WriteLogFunc externLogger)
{
    GetWriteLogFunc() = externLogger;
}
void Logger::Write(LogLevel level, const char* file, int line, const char* func, const char* formatStr, ...)
{
    va_list va;
    va_start(va, formatStr);
    GetInstance().WriteToLog(level, file, line, func, formatStr, va);
    va_end(va);
    if (level >= GetConsoleLogLevel())
    {
        va_start(va, formatStr);
        GetInstance().WriteToConsole(level, formatStr, va);
        va_end(va);
    }
}
void Logger::ThreadInit()
{
    if (logData_ == nullptr)
    {
        // 未调用 Init() 时既无日志文件也无缓冲区，Run() 无处可写：直接停车，让线程不进入循环体
        Stop();
        return;
    }
    // 日志文件已由 Init() 打开：能否写日志在启动期就判定过了，这里不再重复打开
    ThreadBase::ThreadInit();
}
void Logger::ThreadExit()
{
    ThreadBase::ThreadExit();
    if (logData_)
    {
        FlushRemainingBuffers();
        LoggerActive.store(false, std::memory_order_release);
        delete logData_;
    }
    logData_ = nullptr;
    // 本线程的写入器随 LogData 一并释放，TLS 只清指针
    ThreadLogWriterState.Generation = 0;
    ThreadLogWriterState.Writer = nullptr;
}
void Logger::Run()
{
    SwapInnerLogBuffers();
    FlushBuffers();

    static int count = 0;
    if (++count >= 120)
    {
        count = 0;
        auto currTime = TimeUtility::GetLocalTm();
        if (createLogFileTime_.tm_mday != currTime.tm_mday)
        {
            createLogFileTime_ = currTime;
            CreateLogFile();
            if (logData_->LogFile == nullptr)
            {
                // 运行期换日志文件失败：此时让进程退出造成的损失大于"暂时无日志文件"，
                // 记 ERROR 继续跑（控制台仍可见），下个跨日或下次重启会重试
                WriteLog(LogLevel::Error,
                         "Logger: reopen log file for the new day failed, log will not be written to file until a later reopen succeeds.");
            }
        }
    }
}

bool Logger::CreateLogDir(const std::string& path)
{
    // 返回值不能当成功判据：目录已存在时该重载同样返回 false（且不置 error_code），只有 error_code 才表示失败。
    // 不用无 error_code 的重载是因为"同名文件已存在"时它会抛 filesystem_error，而调用点（Init）无人接
    std::error_code errorCode;
    std::filesystem::create_directories(path, errorCode);
    return !errorCode;
}
void Logger::SwapInnerLogBuffers()
{
    std::unique_lock<std::mutex> lock(logData_->Mutex);
    TakeThreadLogChunksLocked(false);
    if (logData_->LogBuffers.empty())
    {
        logData_->ConditionVariable.wait_until(lock, EarliestChunkHandOffTimeLocked());
        TakeThreadLogChunksLocked(false);
    }
    logData_->InnerLogBuffers.swap(logData_->LogBuffers);
}

// 调用方必须持有 LogData::Mutex：无成熟小块可交接时按空闲时限等待，有待成熟的就精确等到它成熟
std::chrono::steady_clock::time_point Logger::EarliestChunkHandOffTimeLocked()
{
    auto earliestTime = std::chrono::steady_clock::now() + std::chrono::seconds(1);
    for (auto& writer : logData_->Writers)
    {
        std::lock_guard<std::mutex> guard(writer->ChunkMutex);
        if (writer->Chunk == nullptr || writer->Chunk->GetLength() == 0)
        {
            continue;
        }
        const auto handOffTime = writer->ChunkStartTime + ChunkHandOffInterval;
        if (handOffTime < earliestTime)
        {
            earliestTime = handOffTime;
        }
    }
    return earliestTime;
}
void Logger::FlushBuffers()
{
    bool isLogFileOpened = (logData_->LogFile != nullptr);
    for (auto& buffer : logData_->InnerLogBuffers)
    {
        if (isLogFileOpened)
        {
            fwrite(buffer->GetData(), buffer->GetLength(), 1, logData_->LogFile);
        }
        buffer->Deallocate();
    }
    logData_->InnerLogBuffers.clear();
    if (isLogFileOpened)
    {
        fflush(logData_->LogFile);
    }
}
// 退出路径专用：SwapInnerLogBuffers 在没有待落盘数据时会等满一个超时周期（最长 1s），
// 而这里只求把 Logger 线程最后一次 Run() 之后写下的日志（含各线程的 ThreadExit）落盘，不该再等
void Logger::FlushRemainingBuffers()
{
    {
        std::lock_guard<std::mutex> guard(logData_->Mutex);
        TakeThreadLogChunksLocked(true);
        logData_->InnerLogBuffers.swap(logData_->LogBuffers);
    }
    FlushBuffers();
}

LogThreadWriter* Logger::AcquireThreadLogWriter()
{
    if (ThreadLogWriterState.Writer != nullptr && ThreadLogWriterState.Generation == logData_->Generation)
    {
        return ThreadLogWriterState.Writer;
    }
    std::unique_ptr<LogThreadWriter> writer = std::make_unique<LogThreadWriter>();
    LogThreadWriter* rawWriter = writer.get();
    {
        std::lock_guard<std::mutex> guard(logData_->Mutex);
        logData_->Writers.push_back(std::move(writer));
    }
    ThreadLogWriterState.Generation = logData_->Generation;
    ThreadLogWriterState.Writer = rawWriter;
    return rawWriter;
}

// 调用方必须持有 LogData::Mutex：遍历与增删 Writers 都在该锁内完成
// 小块的交接时刻未到就不取（写者要重新分配并清零一整块）；停止/退出路径置 flushAllChunks 无条件取走
void Logger::TakeThreadLogChunksLocked(bool flushAllChunks)
{
    const auto currentTime = std::chrono::steady_clock::now();
    for (auto& writer : logData_->Writers)
    {
        std::lock_guard<std::mutex> guard(writer->ChunkMutex);
        for (auto* fullChunk : writer->FullChunks)
        {
            logData_->LogBuffers.push_back(fullChunk);
        }
        writer->FullChunks.clear();
        if (writer->Chunk == nullptr || writer->Chunk->GetLength() == 0)
        {
            continue;
        }
        if (!flushAllChunks && currentTime - writer->ChunkStartTime < ChunkHandOffInterval)
        {
            continue;
        }
        logData_->LogBuffers.push_back(writer->Chunk);
        writer->Chunk = nullptr;
    }
}

void Logger::RetireThreadLogWriter()
{
    LogThreadWriter* writer = ThreadLogWriterState.Writer;
    // 代次不匹配说明旧 LogData 已销毁，写入器对象随之释放：只清 TLS，不再解引用
    if (writer == nullptr || ThreadLogWriterState.Generation != logData_->Generation)
    {
        ThreadLogWriterState.Generation = 0;
        ThreadLogWriterState.Writer = nullptr;
        return;
    }

    std::list<Spark::LinearBuffer<LogBufferSize>*> pendingChunks;
    {
        std::lock_guard<std::mutex> guard(writer->ChunkMutex);
        pendingChunks.swap(writer->FullChunks);
        if (writer->Chunk != nullptr)
        {
            pendingChunks.push_back(writer->Chunk);
            writer->Chunk = nullptr;
        }
    }
    {
        std::lock_guard<std::mutex> guard(logData_->Mutex);
        auto writerIt = std::find_if(logData_->Writers.begin(), logData_->Writers.end(),
                                     [writer](const std::unique_ptr<LogThreadWriter>& registered) { return registered.get() == writer; });
        if (writerIt != logData_->Writers.end())
        {
            logData_->Writers.erase(writerIt);
        }
        for (auto* pendingChunk : pendingChunks)
        {
            logData_->LogBuffers.push_back(pendingChunk);
        }
    }
    logData_->ConditionVariable.notify_one();
    ThreadLogWriterState.Generation = 0;
    ThreadLogWriterState.Writer = nullptr;
}

void Logger::WriteToLog(LogLevel level, const char* file, int line, const char* func, const char* format, va_list va)
{
    if (logData_ == nullptr)
    {
        // Logger 已停止（未 Init 的写入由 WriteLog 宏的 nullptr 检查拦下），停止后的写入静默丢弃
        return;
    }
    if (level < GetLogLevel())
        return;
    for (auto p = file; *p != '\0'; p++)
        if (*p == '\\' || *p == '/')
            file = p + 1;
    const size_t timestampLength = TimeUtility::GetLocalDateTimeWithMilliSecond(LogBuffer, MaxLogFormatLength);
    const auto prefixFormatResult =
        std::format_to_n(LogBuffer + timestampLength, MaxLogFormatLength - timestampLength, " {} {} ", GetCurrentThreadId(), LogLevelName[level]);
    unsigned len1 = static_cast<unsigned>(prefixFormatResult.out - LogBuffer);
    // vsnprintf 返回的是"本该写入"的长度（负数表示编码错误），内容被截断时该值不会随之变小，
    // 不收敛到可写区间会让下一行计算剩余空间 LogLineLength - len1 - len2 - 1 发生无符号回绕
    int formattedContentLength = vsnprintf(LogBuffer + len1, MaxLogLineContentLength, format, va);
    unsigned len2 = static_cast<unsigned>(std::clamp(formattedContentLength, 0, static_cast<int>(MaxLogLineContentLength) - 1));
    const auto suffixFormatResult =
        std::format_to_n(LogBuffer + len1 + len2, LogLineLength - len1 - len2 - 1, "\t\t---{}:{}[{}]\n", file, line, func);
    unsigned len3 = static_cast<unsigned>(suffixFormatResult.out - (LogBuffer + len1 + len2));
    unsigned len = len1 + len2 + len3;
    LogThreadWriter* writer = AcquireThreadLogWriter();
    bool hasChunkToCollect = false;
    {
        std::lock_guard<std::mutex> guard(writer->ChunkMutex);
        if (writer->Chunk == nullptr)
        {
            writer->Chunk = Spark::LinearBuffer<LogBufferSize>::Allocate();
            writer->ChunkStartTime = std::chrono::steady_clock::now();
            hasChunkToCollect = true;
        }
        if (writer->Chunk->GetWriteBufferSize() < len)
        {
            writer->FullChunks.push_back(writer->Chunk);
            writer->Chunk = Spark::LinearBuffer<LogBufferSize>::Allocate();
            writer->ChunkStartTime = std::chrono::steady_clock::now();
            hasChunkToCollect = true;
        }
        writer->Chunk->Append(LogBuffer, len);
    }
    // 只在块开头与换新时通知：消费者本就按 ChunkHandOffInterval 定时来取，按行通知只会把它按行率唤醒空转
    if (hasChunkToCollect)
    {
        logData_->ConditionVariable.notify_one();
    }
}
void Logger::WriteToConsole(LogLevel level, const char* formatStr, va_list va)
{
    static thread_local char logString[LogLineLength] = {0};
    int len = static_cast<int>(std::format_to_n(logString, MaxLogFormatLength, "ThreadId[{}] ", GetCurrentThreadId()).out - logString);
    len += vsnprintf(logString + len, LogLineLength - len - 1, formatStr, va);

    printf("%s\n", logString);
}
void Logger::CreateLogFile()
{
    if (logData_->LogFile)
    {
        fclose(logData_->LogFile);
        logData_->LogFile = nullptr;
    }
    char timeBuff[32];
    strftime(timeBuff, 32, "%Y%m%d-%H%M%S", &createLogFileTime_);
    char fileName[256]{};
    std::format_to_n(fileName, sizeof(fileName) - 1, "log/{}.{}.log", processName_, timeBuff);
    logData_->LogFile = fopen(fileName, "a+");
    if (logData_->LogFile == nullptr)
    {
        // 打不开日志文件时不能只靠断言（Release 下断言会被去掉，空的 FILE* 会流进 fwrite/fflush）。
        // 本函数只如实报告失败，由调用方决定语义：启动期（Init）判失败即退出，
        // 运行期（跨日换文件）由 Run 记 ERROR 继续；FlushBuffers 按 LogFile 是否为空决定是否落盘
        fprintf(stderr, "Logger: open log file failed. Path:%s\n", fileName);
    }
}
static int64_t GetCurrentThreadIdSysCall() noexcept
{
#ifdef _WIN32
    // Windows: 直接调用API
    return static_cast<int64_t>(::GetCurrentThreadId());
#elif defined(__linux__)
    return static_cast<int64_t>(syscall(SYS_gettid));
#else
    return static_cast<int64_t>(pthread_self());
#endif
}
long long Logger::GetCurrentThreadId()
{
    static thread_local long long tid = GetCurrentThreadIdSysCall();
    return tid;
}
}
