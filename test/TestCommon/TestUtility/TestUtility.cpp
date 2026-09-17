#include "TestUtility/TestUtility.h"
#include <Spark/Core/Logger/Logger.h>

using namespace std;
using namespace std::chrono;
using namespace Spark::Core;

TestProtocolType TestProtocol = TestProtocolType::Tcp;

const char* ShmAddress = "shm://TestShm:4";
const char* TcpAddress = "tcp://127.0.0.1:20001";
IoModelType IoModel = IoModelType::Select;

void PrintTimeCost(const char* name, time_point<system_clock, milliseconds> startTime, time_point<system_clock, milliseconds> endTime)
{
    WriteLog(LogLevel::Info, "%s: %lldms", name, (endTime - startTime).count());
}
