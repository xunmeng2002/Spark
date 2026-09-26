#include "TestShmServer.h"
#include "TestTcpServer.h"
#include "TestXtpServer.h"
#include "TestStepServer.h"
#include "TestUtility/TestUtility.h"
#include <Spark/Core/Logger/Logger.h>
#include <Spark/Network/Io/IoFactory.h>

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
        TestShmServer();
        break;
    case TestProtocolType::Tcp:
        TestTcpServer();
        break;
    case TestProtocolType::Xtp:
        TestXtpServer();
        break;
    case TestProtocolType::Step:
        TestStepServer();
        break;
    default:
        break;
    }

    ShutdownTestLogger();
    return 0;
}
