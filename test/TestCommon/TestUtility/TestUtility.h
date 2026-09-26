#pragma once
#include <chrono>
#include <optional>
#include <Spark/Types.h>

enum class TestProtocolType : int
{
    Shm = 0,
    Tcp,
    Xtp,
    Step
};

struct ShmPackage
{
    int ShmType;
    int Count;
    char Data[16];
};

extern TestProtocolType TestProtocol;
extern const char* ShmAddress;
extern const char* TcpAddress;
extern IoModelType IoModel;

// 名字无法识别时返回空，不由解析函数决定报错与退出码
std::optional<TestProtocolType> TryParseTestProtocol(const char* name);

// 用命令行第一个参数（若有）覆盖 TestProtocol；返回进程退出码，0 表示可继续
int ApplyTestProtocolFromCommandLine(int argc, const char* const argv[]);

// 名字取自 GetIoModelString，保证与日志/工厂用的拼写同源
std::optional<IoModelType> TryParseIoModel(const char* name);

// 用命令行第二个参数（若有）覆盖 IoModel；返回进程退出码，0 表示可继续
int ApplyIoModelFromCommandLine(int argc, const char* const argv[]);

// 进程退出前必须调用，否则最后一次缓冲必丢
void ShutdownTestLogger();

void PrintTimeCost(const char* name, std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds> startTime,
                   std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds> endTime);
