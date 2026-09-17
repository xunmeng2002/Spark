#pragma once
#include <chrono>
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

void PrintTimeCost(const char* name, std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds> startTime,
                   std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds> endTime);
