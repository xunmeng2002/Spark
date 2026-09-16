#include "TestTcpServer.h"
#include "ServerIOSubscriberImpl.h"
#include "TestUtility/TestUtility.h"
#include <Spark/Core/Logger/Logger.h>
#include <Spark/Network/Network.h>

using namespace Spark::Core;
using namespace Spark::Network;

void TestTcpServer()
{
    IOThread* ioThread = new IOThread("TcpServer");
    auto io = IOFactory::CreateIo(ServerTypeType::Server, TcpAddress, IoModel);
    ServerIOSubscriberImpl serverIOSubscriberImpl(io, ioThread);
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

