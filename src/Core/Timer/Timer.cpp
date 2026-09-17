#include <Spark/Core/Timer/Timer.h>

using namespace std::chrono;

namespace Spark::Core
{
Timer::Timer() : timeInterval_(60000), eventCount_(600), currentEventCount_(0)
{
    lastTimePoint_ = time_point_cast<milliseconds>(steady_clock::now());
}
void Timer::SetTimer(int milliSeconds, int eventIntervalMilliSeconds)
{
    timeInterval_ = milliSeconds;
    eventCount_ = timeInterval_ / eventIntervalMilliSeconds;
}
void Timer::CheckTimer()
{
    if (++currentEventCount_ > eventCount_)
    {
        auto now = time_point_cast<milliseconds>(steady_clock::now());
        if ((now - lastTimePoint_).count() > timeInterval_)
        {
            lastTimePoint_ = now;
            currentEventCount_ = 0;
            OnTimer();
        }
    }
}
}
