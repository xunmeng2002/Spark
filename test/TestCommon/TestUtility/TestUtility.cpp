#include "TestUtility/TestUtility.h"
#include <Spark/Core/Logger/Logger.h>


using namespace std;
using namespace std::chrono;
using namespace spark::core;



TestProtocolType g_TestProtocolType = TestProtocolType::Tcp;

const char* g_ShmAddress = "shm://TestShm:4";
const char* g_Address = "tcp://127.0.0.1:20001";
IOModelType g_IOModel = IOModelType::Select;


void PrintTimeCost(const char* name, time_point<system_clock, milliseconds> startTime, time_point<system_clock, milliseconds> endTime)
{
	WriteLog(LogLevel::Info, "%s: %lldms", name, (endTime - startTime).count());
}
