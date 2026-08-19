#include <Spark/Network/IO/IOThread.h>
#include <Spark/Network/IO/IOUtility.h>
#include <Spark/Core/Utility/TimeUtility.h>
#include <Spark/Core/Logger/Logger.h>

using namespace std;

using namespace spark::core;
namespace spark::network
{
IOThread::IOThread(const char* threadName)
	:ThreadBase(threadName), m_IO(nullptr)
{
}
IOThread::~IOThread()
{
	if (m_IO)
		delete m_IO;
}

void IOThread::SetIO(IOBase* io)
{
	m_IO = io;
}
void IOThread::Run()
{
	if (m_IO)
		m_IO->HandleIOEvent();
}
void IOThread::ThreadExit()
{
	ThreadBase::ThreadExit();
	m_IO->DisConnectAll();
}
}

