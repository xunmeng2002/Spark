#include "TestShmServer.h"
#include "ServerIOSubscriberImpl.h"
#include "TestUtility/TestUtility.h"
#include <Spark/Core/Logger/Logger.h>
#include <Spark/Network/Network.h>


using namespace spark::core;
using namespace spark::network;

void TestShmServer()
{
    IOThread* ioThread = new IOThread("ShmServer");
    auto io = IOFactory::CreateIO(ServerTypeType::Server, g_ShmAddress);
    ServerIOSubscriberImpl serverIOSubscriberImpl(io, ioThread);
    ioThread->SetIO(io);

    if (!io->Init())
    {
        WriteLog(LogLevel::Error, "ShmServer Init Failed.");
        return;
    }
    ioThread->Start();
    ioThread->Join();
    delete io;
}

