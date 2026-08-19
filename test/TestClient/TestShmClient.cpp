#include "TestShmClient.h"
#include "ClientIOSubscriberImpl.h"
#include "TestUtility/TestUtility.h"
#include <Spark/Core/Logger/Logger.h>
#include <Spark/Network/Network.h>

using namespace spark::core;
using namespace spark::network;

void TestShmClient()
{
    IOThread* ioThread = new IOThread("ShmClient");
    auto io = IOFactory::CreateIO(ServerTypeType::Client, g_ShmAddress);
    ClientIOSubscriberImpl clientIOSubscriberImpl(io, ioThread);
    ioThread->SetIO(io);

    if (!io->Init())
    {
        WriteLog(LogLevel::Error, "ShmClient Init Failed.");
        return;
    }
    ioThread->Start();
    ioThread->Join();
    delete io;
}
