#include <Spark/Core/Timer/Timer.h>

using namespace std::chrono;


namespace spark::core
{
Timer::Timer()
	:m_TimeInterval(60000), m_EventCount(600), m_CurrentEventCount(0)
{
	m_LastTimePoint = time_point_cast<milliseconds>(steady_clock::now());
}
void Timer::SetTimer(int milliSeconds, int eventIntervalMilliSeconds)
{
	m_TimeInterval = milliSeconds;
	m_EventCount = m_TimeInterval / eventIntervalMilliSeconds;
}
void Timer::CheckTimer()
{
	if (++m_CurrentEventCount > m_EventCount)
	{
		auto now = time_point_cast<milliseconds>(steady_clock::now());
		if ((now - m_LastTimePoint).count() > m_TimeInterval)
		{
			m_LastTimePoint = now;
			m_CurrentEventCount = 0;
			OnTimer();
		}
	}
}
}
