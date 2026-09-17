#pragma once
#include <Spark/Core/CoreExport.h>
#include <thread>
#include <string>
#include <chrono>

namespace Spark::Core
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

    std::thread thread_;
    std::string threadName_;
    volatile bool shouldRun_;
    std::chrono::milliseconds timeOut_;
};
}

