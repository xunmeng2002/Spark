#include <Spark/Core/Thread/ThreadBase.h>
#include <Spark/Core/Logger/Logger.h>
#include <functional>
#include <assert.h>


namespace Spark::Core
{
ThreadBase::ThreadBase(const char* name, int milliSeconds)
    :threadName_(name), shouldRun_(false), timeOut_(milliSeconds)
{
}
ThreadBase::~ThreadBase()
{
    Stop();
    Join();
}

void ThreadBase::SetTimeOut(int milliSeconds)
{
    assert(!shouldRun_ && "Cannot modify timeout while thread is running");
    timeOut_ = std::chrono::milliseconds(milliSeconds);
}
bool ThreadBase::Start()
{
    if (thread_.joinable() || shouldRun_)
        return false;

    shouldRun_ = true;
    thread_ = std::thread(std::bind(&ThreadBase::ThreadFunc, this));
    return true;
}
void ThreadBase::Stop()
{
    shouldRun_ = false;
}
void ThreadBase::Join()
{
    if (thread_.joinable())
        thread_.join();
}
std::thread::id ThreadBase::GetThreadId() const
{
    if (thread_.joinable())
        return thread_.get_id();
    else
        return std::thread::id();
}

void ThreadBase::ThreadFunc()
{
    ThreadInit();
    while (shouldRun_)
    {
        Run();
    }
    ThreadExit();
}
void ThreadBase::ThreadInit()
{
    WriteLog(LogLevel::Info, "Thread:%s Start", threadName_.c_str());
}
void ThreadBase::ThreadExit()
{
    WriteLog(LogLevel::Info, "Thread:%s Exit", threadName_.c_str());
}
}
