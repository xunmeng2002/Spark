#include <Spark/Network/Io/IoThread.h>
#include <Spark/Network/Io/IoUtility.h>
#include <Spark/Core/Utility/TimeUtility.h>
#include <Spark/Core/Logger/Logger.h>

using namespace std;

using namespace Spark::Core;
namespace Spark::Network
{
IoThread::IoThread(const char* threadName) : ThreadBase(threadName), io_(nullptr) {}
IoThread::~IoThread()
{
    if (io_)
        delete io_;
}

void IoThread::SetIo(IoBase* io)
{
    io_ = io;
}
void IoThread::Run()
{
    if (io_)
        io_->HandleIoEvent();
}
void IoThread::ThreadExit()
{
    ThreadBase::ThreadExit();
    io_->DisConnectAll();
}
}
