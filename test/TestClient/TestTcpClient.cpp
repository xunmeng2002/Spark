#include "TestTcpClient.h"
#include "ClientIOSubscriberImpl.h"
#include "TestUtility/TestUtility.h"
#include <Spark/Core/Logger/Logger.h>
#include <Spark/Network/Network.h>

using namespace Spark::Core;
using namespace Spark::Network;

void TestTcpClient()
{
    WriteLog(LogLevel::Info, "TestTcpClient");
    IOThread* ioThread = new IOThread("TestTcpClient");
    auto io = IOFactory::CreateIo(ServerTypeType::Client, TcpAddress, IoModel);
    ClientIOSubscriberImpl clientIOSubscriberImpl(io, ioThread);
    ioThread->SetIo(io);

    if (!io->Init())
    {
        WriteLog(LogLevel::Error, "TcpClient Init Failed.");
        return;
    }
    ioThread->Start();
    ioThread->Join();
    delete io;
}
