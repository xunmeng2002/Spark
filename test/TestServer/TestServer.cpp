#include "TestShmServer.h"
#include "TestTcpServer.h"
#include "TestXtpServer.h"
#include "TestStepServer.h"
#include "TestUtility/TestUtility.h"
#include <Spark/Core/Logger/Logger.h>
#include <Spark/Network/IO/IOFactory.h>


int main(int argc, const char* argv[])
{
    Logger::GetInstance().Init(argv[0]);
    Logger::GetInstance().SetLogLevel(LogLevel::Info, LogLevel::Info);
    Logger::GetInstance().Start();

    switch (g_TestProtocolType)
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

    Logger::GetInstance().Stop();
    Logger::GetInstance().Join();
	return 0;
}
