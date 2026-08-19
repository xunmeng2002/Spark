#pragma once
#include <Spark/Core/CoreExport.h>
#include <thread>
#include <string>
#include <chrono>

namespace spark::core
{
class CORE_EXPORTS ThreadBase
{
public:
	ThreadBase(const char* name, int milliSeconds = 100);
	virtual ~ThreadBase();
	
	virtual void SetTimeOut(int milliSeconds);
	virtual bool Start();
	virtual void Stop();
	virtual void Join();
	std::thread::id GetThreadId() const;
	
protected:
	void ThreadFunc();
	virtual void ThreadInit();
	virtual void ThreadExit();


	virtual void Run() = 0;
	

protected:
	std::thread m_Thread;
	std::string m_ThreadName;
	volatile bool m_ShouldRun;
	std::chrono::milliseconds m_TimeOut;
};
}

