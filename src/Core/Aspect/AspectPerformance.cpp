#include <Spark/Core/Aspect/AspectPerformance.h>
#include <stdio.h>

using namespace std::chrono;

namespace Spark::Core
{
AspectPerformance::AspectPerformance()
{
	startTimePoint_ = time_point_cast<microseconds>(system_clock::now());
}
void AspectPerformance::Before(const char* funcName)
{
	startTimePoint_ = time_point_cast<microseconds>(system_clock::now());
}
void AspectPerformance::After(const char* funcName)
{
	endTimePoint_ = time_point_cast<microseconds>(system_clock::now());
	printf("%s: %lldus\n", funcName, (long long)((endTimePoint_ - startTimePoint_).count()));
}
}
