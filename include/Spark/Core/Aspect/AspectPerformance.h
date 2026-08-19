#pragma once
#include <Spark/Core/CoreExport.h>
#include <chrono>

namespace spark::core
{
class CORE_EXPORTS AspectPerformance
{
public:
	AspectPerformance();
	void Before(const char* funcName);
	void After(const char* funcName);
private:
	std::chrono::time_point<std::chrono::system_clock, std::chrono::microseconds> m_StartTimePoint;
    std::chrono::time_point<std::chrono::system_clock, std::chrono::microseconds> m_EndTimePoint;
};
}


