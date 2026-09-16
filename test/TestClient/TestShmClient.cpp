#include "TestShmClient.h"
#include "ClientIoSubscriberImpl.h"
#include "TestUtility/TestUtility.h"
#include <Spark/Core/Logger/Logger.h>
#include <Spark/Network/Network.h>

using namespace Spark::Core;
using namespace Spark::Network;

void TestShmClient()
{
    IoThread* ioThread = new IoThread("ShmClient");
    auto io = IoFactory::CreateIo(ServerTypeType::Client, ShmAddress);
    ClientIoSubscriberImpl clientIoSubscriberImpl(io, ioThread);
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
