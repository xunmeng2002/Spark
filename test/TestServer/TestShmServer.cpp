#include "TestShmServer.h"
#include "ServerIoSubscriberImpl.h"
#include "TestUtility/TestUtility.h"
#include <Spark/Core/Logger/Logger.h>
#include <Spark/Network/Network.h>

using namespace Spark::Core;
using namespace Spark::Network;

void TestShmServer()
{
    IoThread* ioThread = new IoThread("ShmServer");
    auto io = IoFactory::CreateIo(ServerTypeType::Server, ShmAddress);
    ServerIoSubscriberImpl serverIoSubscriberImpl(io, ioThread);
    ioThread->SetIo(io);

    if (!io->Init())
    {
        WriteLog(LogLevel::Error, "ShmServer Init Failed.");
        return;
    }
    ioThread->Start();
    ioThread->Join();
    delete io;
}
