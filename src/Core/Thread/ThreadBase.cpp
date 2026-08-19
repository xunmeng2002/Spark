#include <Spark/Core/Thread/ThreadBase.h>
#include <Spark/Core/Logger/Logger.h>
#include <functional>
#include <assert.h>


namespace spark::core
{
ThreadBase::ThreadBase(const char* name, int milliSeconds)
	:m_ThreadName(name), m_ShouldRun(false), m_TimeOut(milliSeconds)
{
}
ThreadBase::~ThreadBase()
{
	Stop();
	Join();
}

void ThreadBase::SetTimeOut(int milliSeconds)
{
	assert(!m_ShouldRun && "Cannot modify timeout while thread is running");
	m_TimeOut = std::chrono::milliseconds(milliSeconds);
}
bool ThreadBase::Start()
{
	if (m_Thread.joinable() || m_ShouldRun)
		return false;

	m_ShouldRun = true;
	m_Thread = std::thread(std::bind(&ThreadBase::ThreadFunc, this));
	return true;
}
void ThreadBase::Stop()
{
	m_ShouldRun = false;
}
void ThreadBase::Join()
{
	if (m_Thread.joinable())
		m_Thread.join();
}
std::thread::id ThreadBase::GetThreadId() const
{
	if (m_Thread.joinable())
		return m_Thread.get_id();
	else
		return std::thread::id();
}

void ThreadBase::ThreadFunc()
{
	ThreadInit();
	while (m_ShouldRun)
	{
		Run();
	}
	ThreadExit();
}
void ThreadBase::ThreadInit()
{
	WriteLog(LogLevel::Info, "Thread:%s Start", m_ThreadName.c_str());
}
void ThreadBase::ThreadExit()
{
	WriteLog(LogLevel::Info, "Thread:%s Exit", m_ThreadName.c_str());
}
}
