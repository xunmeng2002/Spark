#include "Shm/Sem.h"
#include <Spark/Core/Logger/Logger.h>
#include <Spark/Core/Utility/TimeUtility.h>
#include <thread>
#include <chrono>
#ifdef _WIN32
#include <Windows.h>
#endif
#ifdef __linux__
#include <unistd.h>
#include <fcntl.h>
#endif

using namespace std;
using namespace Spark::Core;

namespace Spark::Network
{
Sem::Sem(const char* name, ServerTypeType serverType, unsigned timeOutMilliSecond) : semName_(name), serverType_(serverType), sem_(nullptr)
{
    timeOutMilliSecond_ = timeOutMilliSecond;
}
Sem::~Sem()
{
    if (sem_ != nullptr)
    {
#ifdef _WIN32
        CloseHandle(sem_);
#endif
#ifdef __linux__
        sem_close(sem_);
        if (serverType_ == ServerTypeType::Server)
        {
            sem_unlink(semName_.c_str());
        }
#endif
        sem_ = nullptr;
    }
}
bool Sem::Init()
{
#ifdef _WIN32
    return WindowsInit();
#endif
#ifdef __linux__
    return LinuxInit();
#endif
}
bool Sem::Lock()
{
#ifdef _WIN32
    return WaitForSingleObject(sem_, timeOutMilliSecond_) == WAIT_OBJECT_0;
#endif

#ifdef __linux__
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    ts.tv_sec += timeOutMilliSecond_ / 1000;
    ts.tv_nsec += (timeOutMilliSecond_ % 1000) * 1000000L;
    if (ts.tv_nsec >= 1000000000)
    {
        ts.tv_sec += ts.tv_nsec / 1000000000;
        ts.tv_nsec = ts.tv_nsec % 1000000000;
    }
    return sem_clockwait(sem_, CLOCK_MONOTONIC, &ts) == 0;
#endif
}
bool Sem::UnLock()
{
    bool result;
#ifdef _WIN32
    result = ReleaseSemaphore(sem_, 1, NULL);
#endif
#ifdef __linux__
    result = sem_post(sem_) == 0;
#endif
    if (!result)
    {
        WriteLog(LogLevel::Error, "Sem UnLock Failed.");
    }
    return result;
}

bool Sem::WindowsInit()
{
#ifdef _WIN32
    sem_ = CreateSemaphoreA(NULL, 1, 1, semName_.c_str());
    if (sem_ == nullptr)
    {
        WriteLog(LogLevel::Error, "CreateSemaphoreA Failed. LastError:%d", GetLastError());
        sem_ = OpenSemaphoreA(SEMAPHORE_ALL_ACCESS, FALSE, semName_.c_str());
        if (sem_ == nullptr)
        {
            WriteLog(LogLevel::Error, "OpenSemaphoreA Failed. LastError:%d", GetLastError());
        }
    }
    if (sem_ == nullptr)
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
#ifdef __linux__
    sem_ = sem_open(semName_.c_str(), O_CREAT | O_EXCL, 0666, 1);
    if (sem_ == SEM_FAILED)
    {
        if (errno == EEXIST)
        {
            sem_ = sem_open(semName_.c_str(), O_EXCL, 0666, 1);
            if (sem_ == SEM_FAILED)
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
