#include "TestShmClient.h"
#include "ClientIOSubscriberImpl.h"
#include "TestUtility/TestUtility.h"
#include <Spark/Core/Logger/Logger.h>
#include <Spark/Network/Network.h>

using namespace Spark::Core;
using namespace Spark::Network;

void TestShmClient()
{
    IOThread* ioThread = new IOThread("ShmClient");
    auto io = IOFactory::CreateIo(ServerTypeType::Client, ShmAddress);
    ClientIOSubscriberImpl clientIOSubscriberImpl(io, ioThread);
    ioThread->SetIo(io);

    if (!io->Init())
    {
        WriteLog(LogLevel::Error, "ShmClient Init Failed.");
        return;
    }
    ioThread->Start();
    ioThread->Join();
    delete io;
}
