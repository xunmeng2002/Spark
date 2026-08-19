#include <Spark/Core/Aspect/AspectPerformance.h>
#include <stdio.h>

using namespace std::chrono;

namespace spark::core
{
AspectPerformance::AspectPerformance()
{
	m_StartTimePoint = time_point_cast<microseconds>(system_clock::now());
}
void AspectPerformance::Before(const char* funcName)
{
	m_StartTimePoint = time_point_cast<microseconds>(system_clock::now());
}
void AspectPerformance::After(const char* funcName)
{
	m_EndTimePoint = time_point_cast<microseconds>(system_clock::now());
	printf("%s: %lldus\n", funcName, (long long)((m_EndTimePoint - m_StartTimePoint).count()));
}
}
