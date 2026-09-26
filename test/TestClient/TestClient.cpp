#include "TestShmClient.h"
#include "TestTcpClient.h"
#include "TestXtpClient.h"
#include "TestStepClient.h"
#include "TestUtility/TestUtility.h"
#include <Spark/Core/Logger/Logger.h>

using namespace Spark::Core;
using namespace Spark::Network;

int main(int argc, const char* argv[])
{
    Logger::GetInstance().Init(argv[0]);
    Logger::GetInstance().SetLogLevel(LogLevel::Info, LogLevel::Info);
    Logger::GetInstance().Start();

    if (const int startupExitCode = ApplyTestProtocolFromCommandLine(argc, argv); startupExitCode != 0)
    {
        return startupExitCode;
    }
    if (const int startupExitCode = ApplyIoModelFromCommandLine(argc, argv); startupExitCode != 0)
    {
        return startupExitCode;
    }

    switch (TestProtocol)
    {
    case TestProtocolType::Shm:
        TestShmClient();
        break;
    case TestProtocolType::Tcp:
        TestTcpClient();
        break;
    case TestProtocolType::Xtp:
        TestXtpClient();
        break;
    case TestProtocolType::Step:
        TestStepClient();
        break;
    default:
        break;
    }

    ShutdownTestLogger();

    return 0;
}
