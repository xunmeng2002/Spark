#include "TestShmClient.h"
#include "TestTcpClient.h"
#include "TestXtpClient.h"
#include "TestStepClient.h"
#include "TestUtility/TestUtility.h"
#include <Spark/Core/Logger/Logger.h>
#include <thread>

using namespace spark::core;
using namespace spark::network;

int main(int argc, const char* argv[])
{
    Logger::GetInstance().Init(argv[0]);
    Logger::GetInstance().SetLogLevel(LogLevel::Info, LogLevel::Info);
    Logger::GetInstance().Start();

    switch (g_TestProtocolType)
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

    //std::this_thread::sleep_for(std::chrono::seconds(10));
    Logger::GetInstance().Stop();
    Logger::GetInstance().Join();

	return 0;
}
