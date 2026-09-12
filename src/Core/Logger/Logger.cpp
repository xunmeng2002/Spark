#include <Spark/Core/Logger/Logger.h>
#include <Spark/TemplateLib/Buffer/Buffer.h>
#include <Spark/Core/Utility/Utility.h>
#include <Spark/Core/Utility/TimeUtility.h>
#include "Logger/LogData.h"
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <stdarg.h>
#include <filesystem>
#include <format>
#include <system_error>
#ifdef _WIN32
#include <Windows.h>
#endif


namespace spark::core
{
constexpr unsigned int LogLineLength = 64 * 1024;
constexpr unsigned int MaxLogFormatLength = 1024;
constexpr unsigned int MaxLogLineContentLength = (LogLineLength - MaxLogFormatLength);

static std::map<LogLevel, std::string> s_LogLevelName = {
	{ LogLevel::Ignore, "IGNORE"},
	{ LogLevel::Debug, "DEBUG"},
	{ LogLevel::Info, "INFO"},
	{ LogLevel::Warning, "WARNING"},
	{ LogLevel::Error, "ERROR"},
	{ LogLevel::Critical, "CRITICAL"},
	{ LogLevel::Emergency, "EMERGENCY"},
};

thread_local char t_LogBuffer[LogLineLength];

Logger::Logger()
	:ThreadBase("Logger"), m_ProcessName(""), m_CreateLogFileTime(), m_LogData(nullptr)
{
}
Logger::~Logger()
{

}

Logger& Logger::GetInstance()
{
	static Logger instance;
	return instance;
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
	Utility::ParseProcessName(fullProcessName, m_ProcessName, 128);
	m_LogData = new LogData();
	// 日志目录建不出来、或日志文件打不开，等于整个进程"无日志运行"：早失败并把原因说到能照着排查，
	// 不让宿主带着看不见的故障跑起来。此处在 Start() 之前，没有已建立的状态需要收尾
	if (!CreateLogDir("log"))
	{
		fprintf(stderr, "Logger: create log directory failed, process exit. Path:log, check that it is not occupied by a file and is writable.\n");
		std::exit(EXIT_FAILURE);
	}
	// 启动期就把日志文件打开（而非留到线程里的 ThreadInit）：能否写日志只有在这一刻判定才叫"启动失败"
	m_CreateLogFileTime = *TimeUtility::GetLocalTm();
	CreateLogFile();
	if (m_LogData->LogFile == nullptr)
	{
		fprintf(stderr, "Logger: cannot write log file, process exit. Check the log directory free space and write permission.\n");
		std::exit(EXIT_FAILURE);
	}

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
	if (m_LogData == nullptr)
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
	if (m_LogData)
	{
		FlushRemainingBuffers();
		delete m_LogData;
	}
	m_LogData = nullptr;
}
void Logger::Run()
{
	SwapInnerLogBuffers();
	FlushBuffers();

	static int count = 0;
	if (++count >= 120)
	{
		count = 0;
		auto currTime = *TimeUtility::GetLocalTm();
		if (m_CreateLogFileTime.tm_mday != currTime.tm_mday)
		{
			m_CreateLogFileTime = currTime;
			CreateLogFile();
			if (m_LogData->LogFile == nullptr)
			{
				// 运行期换日志文件失败：此时让进程退出造成的损失大于"暂时无日志文件"，
				// 记 ERROR 继续跑（控制台仍可见），下个跨日或下次重启会重试
				WriteLog(LogLevel::Error, "Logger: reopen log file for the new day failed, log will not be written to file until a later reopen succeeds.");
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
	std::unique_lock<std::mutex> lock(m_LogData->Mutex);
	if (m_LogData->LogBuffers.empty())
	{
		m_LogData->ConditionVariable.wait_for(lock, std::chrono::seconds(1));
		if (m_LogData->CurrBuffer->GetLength() > 0)
		{
			m_LogData->PushBuffer();
		}
	}
	m_LogData->InnerLogBuffers.swap(m_LogData->LogBuffers);
}
void Logger::FlushBuffers()
{
	bool isLogFileOpened = (m_LogData->LogFile != nullptr);
	for (auto& buffer : m_LogData->InnerLogBuffers)
	{
		if (isLogFileOpened)
		{
			fwrite(buffer->GetData(), buffer->GetLength(), 1, m_LogData->LogFile);
		}
		buffer->Deallocate();
	}
	m_LogData->InnerLogBuffers.clear();
	if (isLogFileOpened)
	{
		fflush(m_LogData->LogFile);
	}
}
// 退出路径专用：SwapInnerLogBuffers 在没有待落盘数据时会等满一个超时周期（最长 1s），
// 而这里只求把 Logger 线程最后一次 Run() 之后写下的日志（含各线程的 ThreadExit）落盘，不该再等
void Logger::FlushRemainingBuffers()
{
	{
		std::lock_guard<std::mutex> guard(m_LogData->Mutex);
		if (m_LogData->CurrBuffer->GetLength() > 0)
		{
			m_LogData->PushBuffer();
		}
		m_LogData->InnerLogBuffers.swap(m_LogData->LogBuffers);
	}
	FlushBuffers();
}

void Logger::WriteToLog(LogLevel level, const char* file, int line, const char* func, const char* format, va_list va)
{
	if (m_LogData == nullptr)
	{
		// Logger 已停止（未 Init 的写入由 WriteLog 宏的 nullptr 检查拦下），停止后的写入静默丢弃
		return;
	}
	if (level < GetLogLevel())
		return;
	for (auto p = file; *p != '\0'; p++)
		if (*p == '\\' || *p == '/')
			file = p + 1;
	unsigned len1 = std::format_to_n(t_LogBuffer, MaxLogFormatLength, "{} {} {} ", TimeUtility::GetLocalDateTimeWithMilliSecond(), GetCurrentThreadID(), s_LogLevelName[level]).out - t_LogBuffer;
	// vsnprintf 返回的是"本该写入"的长度（负数表示编码错误），内容被截断时该值不会随之变小，
	// 不收敛到可写区间会让下一行计算剩余空间 LogLineLength - len1 - len2 - 1 发生无符号回绕
	int formattedContentLength = vsnprintf(t_LogBuffer + len1, MaxLogLineContentLength, format, va);
	unsigned len2 = static_cast<unsigned>(std::clamp(formattedContentLength, 0, static_cast<int>(MaxLogLineContentLength) - 1));
	unsigned len3 = std::format_to_n(t_LogBuffer + len1 + len2, LogLineLength - len1 - len2 - 1, "\t\t---{}:{}[{}]\n", file, line, func).out - (t_LogBuffer + len1 + len2);
	unsigned len = len1 + len2 + len3;
	std::lock_guard<std::mutex> guard(m_LogData->Mutex);
	if (m_LogData->CurrBuffer->GetWriteBufferSize() < len)
	{
		m_LogData->PushBuffer();
	}
	m_LogData->CurrBuffer->Append(t_LogBuffer, len);
	m_LogData->ConditionVariable.notify_one();
}
void Logger::WriteToConsole(LogLevel level, const char* formatStr, va_list va)
{
	static thread_local char logString[LogLineLength] = {0};
	int len = std::format_to_n(logString, MaxLogFormatLength, "ThreadID[{}] ", GetCurrentThreadID()).out - logString;
	len += vsnprintf(logString + len, LogLineLength - len - 1, formatStr, va);

	printf("%s\n", logString);
}
void Logger::CreateLogFile()
{
	if (m_LogData->LogFile)
	{
		fclose(m_LogData->LogFile);
		m_LogData->LogFile = nullptr;
	}
	char timeBuff[32];
	strftime(timeBuff, 32, "%Y%m%d-%H%M%S", &m_CreateLogFileTime);
	char fileName[256]{};
	std::format_to_n(fileName, sizeof(fileName) - 1, "log/{}.{}.log", m_ProcessName, timeBuff);
	m_LogData->LogFile = fopen(fileName, "a+");
	if (m_LogData->LogFile == nullptr)
	{
		// 打不开日志文件时不能只靠断言（Release 下断言会被去掉，空的 FILE* 会流进 fwrite/fflush）。
		// 本函数只如实报告失败，由调用方决定语义：启动期（Init）判失败即退出，
		// 运行期（跨日换文件）由 Run 记 ERROR 继续；FlushBuffers 按 LogFile 是否为空决定是否落盘
		fprintf(stderr, "Logger: open log file failed. Path:%s\n", fileName);
	}
}
static int64_t GetCurrentThreadIdSysCall() noexcept {
#ifdef _WIN32
	// Windows: 直接调用API
	return static_cast<int64_t>(::GetCurrentThreadId());
#elif defined(__linux__)
	return static_cast<int64_t>(syscall(SYS_gettid));
#else
	return static_cast<int64_t>(pthread_self());
#endif
}
long long Logger::GetCurrentThreadID()
{
	static thread_local long long tid = GetCurrentThreadIdSysCall();
	return tid;
}
}


