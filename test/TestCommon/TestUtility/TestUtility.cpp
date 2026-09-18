#include "TestUtility/TestUtility.h"
#include <Spark/Core/Logger/Logger.h>

#include <cstring>

using namespace std;
using namespace std::chrono;
using namespace Spark::Core;

TestProtocolType TestProtocol = TestProtocolType::Tcp;

const char* ShmAddress = "shm://TestShm:4";
const char* TcpAddress = "tcp://127.0.0.1:20001";
IoModelType IoModel = IoModelType::Select;

namespace
{
struct TestProtocolName
{
    const char* Name;
    TestProtocolType Protocol;
};

const TestProtocolName TestProtocolNames[] = {
    {"Shm", TestProtocolType::Shm},
    {"Tcp", TestProtocolType::Tcp},
    {"Xtp", TestProtocolType::Xtp},
    {"Step", TestProtocolType::Step},
};
}

std::optional<TestProtocolType> TryParseTestProtocol(const char* name)
{
    if (name == nullptr)
    {
        return std::nullopt;
    }
    for (const auto& entry : TestProtocolNames)
    {
        if (strcmp(name, entry.Name) == 0)
        {
            return entry.Protocol;
        }
    }
    return std::nullopt;
}

int ApplyTestProtocolFromCommandLine(int argc, const char* const argv[])
{
    if (argc <= 1)
    {
        return 0;
    }
    const std::optional<TestProtocolType> protocol = TryParseTestProtocol(argv[1]);
    if (!protocol.has_value())
    {
        WriteLog(LogLevel::Error, "Unknown Protocol:[%s], Expected: Shm/Tcp/Xtp/Step", argv[1]);
        ShutdownTestLogger();
        return 2;
    }
    TestProtocol = *protocol;
    return 0;
}

void ShutdownTestLogger()
{
    Logger::GetInstance().Stop();
    Logger::GetInstance().Join();
}

void PrintTimeCost(const char* name, time_point<system_clock, milliseconds> startTime, time_point<system_clock, milliseconds> endTime)
{
    WriteLog(LogLevel::Info, "%s: %lldms", name, (endTime - startTime).count());
}
