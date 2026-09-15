#include <Spark/Network/IO/IOThread.h>
#include <Spark/Network/IO/IOUtility.h>
#include <Spark/Core/Utility/TimeUtility.h>
#include <Spark/Core/Logger/Logger.h>

using namespace std;

using namespace Spark::Core;
namespace Spark::Network
{
IOThread::IOThread(const char* threadName)
	:ThreadBase(threadName), io_(nullptr)
{
}
IOThread::~IOThread()
{
	if (io_)
		delete io_;
}

void IOThread::SetIO(IOBase* io)
{
	io_ = io;
}
void IOThread::Run()
{
	if (io_)
		io_->HandleIOEvent();
}
void IOThread::ThreadExit()
{
	ThreadBase::ThreadExit();
	io_->DisConnectAll();
}
}

