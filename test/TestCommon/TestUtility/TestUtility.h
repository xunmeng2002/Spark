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

extern TestProtocolType g_TestProtocolType;
extern const char* g_ShmAddress;
extern const char* g_Address;
extern IOModelType g_IOModel;

void PrintTimeCost(const char* name, std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds> startTime, std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds> endTime);