#pragma once
#include <Spark/Core/CoreExport.h>
#include <chrono>


namespace spark::core
{
class CORE_EXPORTS Timer
{
public:
	Timer();
	void SetTimer(int milliSeconds, int eventIntervalMilliSeconds = 100);

protected:
	virtual void CheckTimer();
	virtual void OnTimer() = 0;

protected:
	int m_TimeInterval;
	int m_EventCount;
	int m_CurrentEventCount;
	std::chrono::time_point<std::chrono::steady_clock, std::chrono::milliseconds> m_LastTimePoint;
};
}
