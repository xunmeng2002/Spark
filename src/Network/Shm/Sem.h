#pragma once
#include <Spark/Types.h>
#include <string>
#ifdef __linux__
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
#ifdef __linux__
	sem_t* m_Sem;
#endif
#ifdef _WIN32
	void* m_Sem;
#endif
};
}

