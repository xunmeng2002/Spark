#pragma once
#include <Spark/Core/CoreExport.h>
#include <Spark/Types.h>
#include <chrono>
#include <string>


namespace spark::core
{
class CORE_EXPORTS TimeUtility
{
public:
	static time_t GetTime();
	static time_t GetTime(const char* date);
	static time_t GetTime(int date);
	static tm* GetTm(int date);
	static tm* GetUtcTm();
	static tm* GetLocalTm();

	static void GetPreTradingDay(const char* tradingDay, char* preTradingDay);
	static int GetPreTradingDay(int tradingDay);
	static int GetPreDate(int date);
	static void GetNextTradingDay(const char* tradingDay, char* nextTradingDay);
	static int GetNextTradingDay(int tradingDay);
	static int GetNextDate(int date);
	static int GetDateWithDayCount(int dayCount);
	static int GetDateWithDayCount(int date, int dayCount);

	static int DateAdd(int date, int count);
	static int HourAdd(int hourTime, int count);
	static long long MinuteAdd(long long minuteTime, int count);

	static void GetPreYearDay(const char* tradingDay, char* preYearDay);

	static std::string GetUtcDate();
	static std::string GetUtcTime();
	static std::string GetUtcDateTime();
	static std::string GetUtcDateTimeWithMilliSecond();

	static std::string GetLocalDate();
	static std::string GetLocalTime();
	static std::string GetLocalDateTime();
	static void GetLocalDateTime(char* date, char* time);
	static std::string GetLocalDateTimeWithMilliSecond();
	static long long GetMilliSecondTimeStamp();
	static void GetDateTimeFromTimeStamp(const Int64Type& timeStamp, DateType& date, TimeType& time);

	static int GetTimeFromTimeString(const char* time);
	static time_t GetTimeFromString(std::string dateTime, std::string format = "%04d%02d%02d-%02d:%02d:%02d");
	static std::string ToUtcDate(time_t* time);
	static std::string ToUtcTime(time_t* time);
	static std::string ToUtcDateTime(time_t* time);
	static std::string ToLocalDate(time_t* time);
	static std::string ToLocalTime(time_t* time);
	static std::string ToLocalDateTime(time_t* time);


	static void GetDateTimeFromUpdateTs(long long updateTs, int& date, int& hour, int& minute, int& second, int& milliSecond);
	static int CalculateNextBarFromDayBar(int tradingDay, int barPeriod);
	static int CalculateMinutes(int hour, int minute);
	static int CalculateMinutesTimeStamp(int minutes);
	static int CalculateSeconds(int hour, int minute, int second);
	static int CalculateSecondsTimeStamp(int seconds);

	static int CalculateNextBarDate(int barPeriod, int date);
	static long long CalculateNextMinuteBarTime(long long updateTs);
	static long long CalculateNextSecondBarTime(int barPeriod, long long updateTs);

	static void CalculateRealMinuteBarTime(const char* exchangeID, const char* instrumentID, int calculateBarTime, int& realBarTime, int& realUpdateTs);

	template<typename T>
	static long long GetDuration(std::chrono::steady_clock::time_point& start)
	{
		auto end = std::chrono::steady_clock::now();
		return std::chrono::duration_cast<T>(end - start).count();
	}
};
}

