#pragma once
#include <Spark/Core/CoreExport.h>
#include <chrono>

namespace Spark::Core
{
class CORE_EXPORTS AspectPerformance
{
public:
    AspectPerformance();
    void Before(const char* funcName);
    void After(const char* funcName);

private:
    std::chrono::time_point<std::chrono::system_clock, std::chrono::microseconds> startTimePoint_;
    std::chrono::time_point<std::chrono::system_clock, std::chrono::microseconds> endTimePoint_;
};
}
