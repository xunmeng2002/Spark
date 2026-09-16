#include "TestTcpClient.h"
#include "ClientIoSubscriberImpl.h"
#include "TestUtility/TestUtility.h"
#include <Spark/Core/Logger/Logger.h>
#include <Spark/Network/Network.h>

using namespace Spark::Core;
using namespace Spark::Network;

void TestTcpClient()
{
    WriteLog(LogLevel::Info, "TestTcpClient");
    IoThread* ioThread = new IoThread("TestTcpClient");
    auto io = IoFactory::CreateIo(ServerTypeType::Client, TcpAddress, IoModel);
    ClientIoSubscriberImpl clientIoSubscriberImpl(io, ioThread);
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
