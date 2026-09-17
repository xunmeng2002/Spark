#pragma once
#include <Spark/Core/CoreExport.h>
#include <chrono>


namespace Spark::Core
{
class CORE_EXPORTS Timer
{
public:
    Timer();
    void SetTimer(int milliSeconds, int eventIntervalMilliSeconds = 100);

protected:
    virtual void CheckTimer();
    virtual void OnTimer() = 0;

    int timeInterval_;
    int eventCount_;
    int currentEventCount_;
    std::chrono::time_point<std::chrono::steady_clock, std::chrono::milliseconds> lastTimePoint_;
};
}
