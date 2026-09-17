#pragma once
#include <Spark/Types.h>
#include <string>
#ifdef __linux__
#include <semaphore.h>
#endif

namespace Spark::Network
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

    std::string semName_;
    ServerTypeType serverType_;
    unsigned timeOutMilliSecond_;
#ifdef __linux__
    sem_t* sem_;
#endif
#ifdef _WIN32
    void* sem_;
#endif
};
}
