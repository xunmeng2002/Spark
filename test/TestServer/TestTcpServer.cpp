#include "TestTcpServer.h"
#include "ServerIoSubscriberImpl.h"
#include "TestUtility/TestUtility.h"
#include <Spark/Core/Logger/Logger.h>
#include <Spark/Network/Network.h>

using namespace Spark::Core;
using namespace Spark::Network;

void TestTcpServer()
{
    IoThread* ioThread = new IoThread("TcpServer");
    auto io = IoFactory::CreateIo(ServerTypeType::Server, TcpAddress, IoModel);
    ServerIoSubscriberImpl serverIoSubscriberImpl(io, ioThread);
    ioThread->SetIo(io);

    if (!io->Init())
    {
        WriteLog(LogLevel::Error, "TcpServer Init Failed.");
        return;
    }
    ioThread->Start();
    ioThread->Join();
    delete io;
}

