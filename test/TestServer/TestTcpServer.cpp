#include "TestTcpServer.h"
#include "ServerIOSubscriberImpl.h"
#include "TestUtility/TestUtility.h"
#include <Spark/Core/Logger/Logger.h>
#include <Spark/Network/Network.h>

using namespace spark::core;
using namespace spark::network;

void TestTcpServer()
{
    IOThread* ioThread = new IOThread("TcpServer");
    auto io = IOFactory::CreateIO(ServerTypeType::Server, g_Address, g_IOModel);
    ServerIOSubscriberImpl serverIOSubscriberImpl(io, ioThread);
    ioThread->SetIO(io);

    if (!io->Init())
    {
        WriteLog(LogLevel::Error, "TcpServer Init Failed.");
        return;
    }
    ioThread->Start();
    ioThread->Join();
    delete io;
}

