#include "Shm/Sem.h"
#include <Spark/Core/Logger/Logger.h>
#include <Spark/Core/Utility/TimeUtility.h>
#include <thread>
#include <chrono>
#ifdef WINDOWS
#include <Windows.h>
#endif
#ifdef LINUX
#include <unistd.h>
#include <fcntl.h>
#endif

using namespace std;
using namespace spark::core;

namespace spark::network
{
Sem::Sem(const char* name, ServerTypeType serverType, unsigned timeOutMilliSecond)
	:m_SemName(name), m_Sem(nullptr), m_ServerType(serverType)
{
	m_TimeOutMilliSecond = timeOutMilliSecond;
}
Sem::~Sem()
{
	if (m_Sem != nullptr)
	{
#ifdef WINDOWS
		CloseHandle(m_Sem);
#endif
#ifdef LINUX
		sem_close(m_Sem);
		if (m_ServerType == ServerTypeType::Server)
		{
			sem_unlink(m_SemName.c_str());
		}
#endif
		m_Sem = nullptr;
	}
}
bool Sem::Init()
{
#ifdef WINDOWS
	return WindowsInit();
#endif
#ifdef LINUX
	return LinuxInit();
#endif
}
bool Sem::Lock()
{
#ifdef WINDOWS
	return WaitForSingleObject(m_Sem, m_TimeOutMilliSecond) == WAIT_OBJECT_0;
#endif
	
#ifdef LINUX
	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);
	ts.tv_sec += m_TimeOutMilliSecond / 1000;
	ts.tv_nsec += (m_TimeOutMilliSecond % 1000) * 1000000L;
	if (ts.tv_nsec >= 1000000000)
	{
		ts.tv_sec += ts.tv_nsec / 1000000000;
		ts.tv_nsec = ts.tv_nsec % 1000000000;
	}
	return sem_timedwait(m_Sem, &ts) == 0;
#endif
}
bool Sem::UnLock()
{
	bool result;
#ifdef WINDOWS
	result = ReleaseSemaphore(m_Sem, 1, NULL);
#endif
#ifdef LINUX
	result = sem_post(m_Sem) == 0;
#endif
	if (!result)
	{
		WriteLog(LogLevel::Error, "Sem UnLock Failed.");
	}
	return result;
}

bool Sem::WindowsInit()
{
#ifdef WINDOWS
	m_Sem = CreateSemaphoreA(NULL, 1, 1, m_SemName.c_str());
	if (m_Sem == nullptr)
	{
		WriteLog(LogLevel::Error, "CreateSemaphoreA Failed. LastError:%d", GetLastError());
		m_Sem = OpenSemaphoreA(SEMAPHORE_ALL_ACCESS, FALSE, m_SemName.c_str());
		if (m_Sem == nullptr)
		{
			WriteLog(LogLevel::Error, "OpenSemaphoreA Failed. LastError:%d", GetLastError());
		}
	}
	if (m_Sem == nullptr)
	{
		WriteLog(LogLevel::Info, "Create Or Open Semaphore Success.");
		return false;
	}
	WriteLog(LogLevel::Info, "Sem::Init Successed");
	return true;
#else
	return false;
#endif
	
}
bool Sem::LinuxInit()
{
#ifdef LINUX
	m_Sem = sem_open(m_SemName.c_str(), O_CREAT | O_EXCL, 0666, 1);
	if (m_Sem == SEM_FAILED)
	{
		if (errno == EEXIST)
		{
			m_Sem = sem_open(m_SemName.c_str(), O_EXCL, 0666, 1);
			if (m_Sem == SEM_FAILED)
			{
				WriteLog(LogLevel::Warning, "sem_open Failed. ErrNo:%d", errno);
				return false;
			}
			else
			{
				WriteLog(LogLevel::Warning, "sem_open Successed ReOpen");
			}
		}
		else
		{
			WriteLog(LogLevel::Warning, "sem_open Create Failed. ErrNo:%d", errno);
			return false;
		}
	}
	else
	{
		WriteLog(LogLevel::Warning, "sem_open Successed FirstOpen");
	}
	WriteLog(LogLevel::Info, "Sem::Init Successed");
	return true;
#else
	return false;
#endif
}
}
