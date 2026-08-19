#pragma once
#include <Spark/Types.h>
#include <string>
#ifdef LINUX
#include <semaphore.h>
#endif


namespace spark::network
{
class Sem
{
public:
	Sem(const char* name, ServerTypeType serverType, unsigned timeOutMilliSecond = 100);
	~Sem();
	bool Init();
	bool Lock();
	bool UnLock();

private:
	bool WindowsInit();
	bool LinuxInit();

private:
	std::string m_SemName;
	ServerTypeType m_ServerType;
	unsigned m_TimeOutMilliSecond;
#ifdef LINUX
	sem_t* m_Sem;
#endif
#ifdef WINDOWS
	void* m_Sem;
#endif
};
}

