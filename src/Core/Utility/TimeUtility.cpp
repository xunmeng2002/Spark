#include <Spark/Core/Utility/TimeUtility.h>
#include <format>
#include <iostream>
#include <sstream>
#include <ctime>
#include <time.h>
#include <cstring>
#include <sys/timeb.h>

using namespace std;
using namespace std::chrono;

namespace spark::core
{
thread_local char t_DateTimeBuff[32];
time_t TimeUtility::GetTime()
{
	std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
	return std::chrono::system_clock::to_time_t(now);
}
time_t TimeUtility::GetTime(const char* date)
{
	auto dateInt = atoi(date);
	tm dateTm;
	memset(&dateTm, 0, sizeof(dateTm));
	dateTm.tm_year = dateInt / 10000 - 1900;
	dateTm.tm_mon = (dateInt / 100) % 100 - 1;
	dateTm.tm_mday = dateInt % 100;
	return mktime(&dateTm);
}
time_t TimeUtility::GetTime(int date)
{
	tm dateTm;
	memset(&dateTm, 0, sizeof(dateTm));
	dateTm.tm_year = date / 10000 - 1900;
	dateTm.tm_mon = (date / 100) % 100 - 1;
	dateTm.tm_mday = date % 100;
	return mktime(&dateTm);
}
tm* TimeUtility::GetTm(int date)
{
	auto t = GetTime(date);
	return localtime(&t);
}
tm* TimeUtility::GetUtcTm()
{
	time_t t = GetTime();
	return gmtime(&t);
}
tm* TimeUtility::GetLocalTm()
{
	time_t t = GetTime();
	return localtime(&t);
}

void TimeUtility::GetPreTradingDay(const char* tradingDay, char* preTradingDay)
{
	auto dateTime = GetTime(tradingDay);

	time_t lastTradingDay = dateTime - 86400LL;
	tm* lastTradingDayTm = localtime(&lastTradingDay);
	while (lastTradingDayTm->tm_wday == 0 || lastTradingDayTm->tm_wday == 6)
	{
		lastTradingDay = lastTradingDay - 86400LL;
		lastTradingDayTm = localtime(&lastTradingDay);
	}
	strftime(preTradingDay, 9, "%Y%m%d", lastTradingDayTm);
}
int TimeUtility::GetPreTradingDay(int tradingDay)
{
	auto dateTime = GetTime(tradingDay);
	time_t lastTradingDay = dateTime - 86400LL;
	tm* lastTradingDayTm = localtime(&lastTradingDay);
	while (lastTradingDayTm->tm_wday == 0 || lastTradingDayTm->tm_wday == 6)
	{
		lastTradingDay = lastTradingDay - 86400LL;
		lastTradingDayTm = localtime(&lastTradingDay);
	}
	return (lastTradingDayTm->tm_year + 1900) * 10000 + (lastTradingDayTm->tm_mon + 1) * 100 + lastTradingDayTm->tm_mday;
}
int TimeUtility::GetPreDate(int date)
{
	auto dateTime = GetTime(date);
	time_t nextDateTime = dateTime - 86400LL;
	tm* nextTm = localtime(&nextDateTime);
	return (nextTm->tm_year + 1900) * 10000 + (nextTm->tm_mon + 1) * 100 + nextTm->tm_mday;
}
void TimeUtility::GetNextTradingDay(const char* tradingDay, char* nextTradingDay)
{
	auto dateTime = GetTime(tradingDay);

	time_t nextTradingDayTime = dateTime + 86400LL;
	tm* nextTradingDayTm = localtime(&nextTradingDayTime);
	while (nextTradingDayTm->tm_wday == 0 || nextTradingDayTm->tm_wday == 6)
	{
		nextTradingDayTime = nextTradingDayTime + 86400LL;
		nextTradingDayTm = localtime(&nextTradingDayTime);
	}
	strftime(nextTradingDay, 9, "%Y%m%d", nextTradingDayTm);
}
int TimeUtility::GetNextTradingDay(int tradingDay)
{
	auto dateTime = GetTime(tradingDay);
	time_t lastTradingDay = dateTime + 86400LL;
	tm* lastTradingDayTm = localtime(&lastTradingDay);
	while (lastTradingDayTm->tm_wday == 0 || lastTradingDayTm->tm_wday == 6)
	{
		lastTradingDay = lastTradingDay + 86400LL;
		lastTradingDayTm = localtime(&lastTradingDay);
	}
	return (lastTradingDayTm->tm_year + 1900) * 10000 + (lastTradingDayTm->tm_mon + 1) * 100 + lastTradingDayTm->tm_mday;
}
int TimeUtility::GetNextDate(int date)
{
	auto dateTime = GetTime(date);
	time_t nextDateTime = dateTime + 86400LL;
	tm* nextTm = localtime(&nextDateTime);
	return (nextTm->tm_year + 1900) * 10000 + (nextTm->tm_mon + 1) * 100 + nextTm->tm_mday;
}
int TimeUtility::GetDateWithDayCount(int dayCount)
{
	auto dateTime = GetTime();
	time_t targetTime = dateTime + 86400LL * dayCount;
	tm* targetTm = localtime(&targetTime);
	return (targetTm->tm_year + 1900) * 10000 + (targetTm->tm_mon + 1) * 100 + targetTm->tm_mday;
}
int TimeUtility::GetDateWithDayCount(int date, int dayCount)
{
	auto dateTime = GetTime(date);
	time_t targetTime = dateTime + 86400LL * dayCount;
	tm* targetTm = localtime(&targetTime);
	return (targetTm->tm_year + 1900) * 10000 + (targetTm->tm_mon + 1) * 100 + targetTm->tm_mday;
}

int TimeUtility::DateAdd(int date, int count)
{
	auto t = GetTime(date);
	t += 86400LL * count;
	return atoi(ToLocalDate(&t).c_str());
}
int TimeUtility::HourAdd(int hourTime, int count)
{
	auto hour = hourTime % 100;
	auto date = hourTime / 100;
	hour += count;

	if (hour >= 0 && hour < 24)
	{
		return date * 100 + hour;
	}

	int dayCount;
	if (hour >= 0)
	{
		dayCount = hour / 24;
		hour %= 24;
	}
	else
	{
		// C++ 对负数除法向零截断（-1/24=0），需做 floor 除法
		dayCount = (hour - 23) / 24;
		hour -= dayCount * 24;  // 归入 [0, 23]
	}

	date = DateAdd(date, dayCount);
	return date * 100 + hour;
}
long long TimeUtility::MinuteAdd(long long minuteTime, int count)
{
	auto minute = minuteTime % 100;
	auto hourTime = minuteTime / 100;
	minute += count;

	if (minute >= 0 && minute < 60)
	{
		return hourTime * 100 + minute;
	}

	int hourCount;
	if (minute >= 0)
	{
		hourCount = minute / 60;
		minute %= 60;
	}
	else
	{
		// C++ 对负数除法向零截断，需做 floor 除法
		hourCount = (minute - 59) / 60;
		minute -= hourCount * 60;  // 归入 [0, 59]
	}

	hourTime = HourAdd(hourTime, hourCount);
	return hourTime * 100 + minute;
}

void TimeUtility::GetPreYearDay(const char* tradingDay, char* preYearDay)
{
	auto dateTime = GetTime(tradingDay);

	time_t preYearDayTime = dateTime - 86400LL * 365LL;
	tm* preYearDayTm = localtime(&preYearDayTime);
	strftime(preYearDay, 9, "%Y%m%d", preYearDayTm);
}


std::string TimeUtility::GetUtcDate()
{
	auto t = GetTime();
	auto localTm = gmtime(&t);
	strftime(t_DateTimeBuff, 32, "%Y%m%d", localTm);
	return std::string(t_DateTimeBuff);
}
std::string TimeUtility::GetUtcTime()
{
	auto t = GetTime();
	auto localTm = gmtime(&t);
	strftime(t_DateTimeBuff, 32, "%H:%M:%S", localTm);
	return std::string(t_DateTimeBuff);
}
std::string TimeUtility::GetUtcDateTime()
{
	auto t = GetTime();
	auto localTm = gmtime(&t);
	strftime(t_DateTimeBuff, 32, "%Y%m%d-%H:%M:%S", localTm);
	return std::string(t_DateTimeBuff);
}
std::string TimeUtility::GetUtcDateTimeWithMilliSecond()
{
	auto now = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now());
	time_t t = std::chrono::system_clock::to_time_t(now);
	int milliSecond = now.time_since_epoch().count() % 1000;
	auto localTm = gmtime(&t);
	auto len = strftime(t_DateTimeBuff, 32, "%Y%m%d-%H:%M:%S", localTm);
	std::format_to_n(t_DateTimeBuff + len, 32 - len, ".{:03}", milliSecond);
	return std::string(t_DateTimeBuff);
}

std::string TimeUtility::GetLocalDate()
{
	auto t = GetTime();
	auto localTm = localtime(&t);
	strftime(t_DateTimeBuff, 32, "%Y%m%d", localTm);
	return std::string(t_DateTimeBuff);
}
std::string TimeUtility::GetLocalTime()
{
	auto t = GetTime();
	auto localTm = localtime(&t);
	strftime(t_DateTimeBuff, 32, "%H:%M:%S", localTm);
	return std::string(t_DateTimeBuff);
}
std::string TimeUtility::GetLocalDateTime()
{
	auto t = GetTime();
	auto localTm = localtime(&t);
	strftime(t_DateTimeBuff, 32, "%Y%m%d-%H:%M:%S", localTm);
	return std::string(t_DateTimeBuff);
}
void TimeUtility::GetLocalDateTime(char* date, char* time)
{
	auto t = GetTime();
	auto localTm = localtime(&t);
	strftime(date, 9, "%Y%m%d", localTm);
	strftime(time, 9, "%H:%M:%S", localTm);
}
std::string TimeUtility::GetLocalDateTimeWithMilliSecond()
{
	auto now = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now());
	time_t t = std::chrono::system_clock::to_time_t(now);
	int milliSecond = now.time_since_epoch().count() % 1000;
	auto localTm = localtime(&t);
	auto len = strftime(t_DateTimeBuff, 32, "%Y%m%d-%H:%M:%S", localTm);
	std::format_to_n(t_DateTimeBuff + len, 32 - len, ".{:03}", milliSecond);
	return std::string(t_DateTimeBuff);
}
long long TimeUtility::GetMilliSecondTimeStamp()
{
	time_point<system_clock, milliseconds> startTime = time_point_cast<milliseconds>(system_clock::now());
	time_t time = startTime.time_since_epoch().count() / 1000;
	int ms = startTime.time_since_epoch().count() % 1000;
	auto len = strftime(t_DateTimeBuff, 32, "%Y%m%d%H%M%S", localtime(&time));
	std::format_to_n(t_DateTimeBuff + len, 32 - len, "{:03}", ms);
	return atoll(t_DateTimeBuff);
}
void TimeUtility::GetDateTimeFromTimeStamp(const Int64Type& timeStamp, DateType& date, TimeType& time)
{
	snprintf(date, sizeof(DateType), "%lld", timeStamp / 1000000000LL);
	snprintf(time, sizeof(TimeType), "%lld", (timeStamp / 1000LL) % 1000000LL);
}

int TimeUtility::GetTimeFromTimeString(const char* time)
{
	std::istringstream iss(time);
	int hour, minute, second;
	char sep;
	iss >> hour >> sep >> minute >> sep >> second;
	return hour * 10000 + minute * 100 + second;
}
time_t TimeUtility::GetTimeFromString(std::string dateTime, std::string format)
{
	tm t;
	int len = sscanf(dateTime.c_str(), format.c_str(), &t.tm_year, &t.tm_mon, &t.tm_mday, &t.tm_hour, &t.tm_min, &t.tm_sec);
	t.tm_year -= 1900;
	t.tm_mon -= 1;

	return mktime(&t);
}
std::string TimeUtility::ToUtcDate(time_t* time)
{
	strftime(t_DateTimeBuff, 32, "%Y%m%d", gmtime(time));
	return std::string(t_DateTimeBuff);
}
std::string TimeUtility::ToUtcTime(time_t* time)
{
	strftime(t_DateTimeBuff, 32, "%H:%M:%S", gmtime(time));
	return std::string(t_DateTimeBuff);
}
std::string TimeUtility::ToUtcDateTime(time_t* time)
{
	strftime(t_DateTimeBuff, 32, "%Y%m%d-%H:%M:%S", gmtime(time));
	return std::string(t_DateTimeBuff);
}
std::string TimeUtility::ToLocalDate(time_t* time)
{
	strftime(t_DateTimeBuff, 32, "%Y%m%d", localtime(time));
	return std::string(t_DateTimeBuff);
}
std::string TimeUtility::ToLocalTime(time_t* time)
{
	strftime(t_DateTimeBuff, 32, "%H:%M:%S", localtime(time));
	return std::string(t_DateTimeBuff);
}
std::string TimeUtility::ToLocalDateTime(time_t* time)
{
	strftime(t_DateTimeBuff, 32, "%Y%m%d-%H:%M:%S", localtime(time));
	return std::string(t_DateTimeBuff);
}


void TimeUtility::GetDateTimeFromUpdateTs(long long updateTs, int& date, int& hour, int& minute, int& second, int& milliSecond)
{
	milliSecond = (int)(updateTs % 1000LL);
	updateTs /= 1000LL;
	second = (int)(updateTs % 100LL);
	updateTs /= 100LL;
	minute = (int)(updateTs % 100LL);
	updateTs /= 100LL;
	hour = (int)(updateTs % 100LL);
	date = (int)(updateTs / 100LL);
}
int TimeUtility::CalculateNextBarFromDayBar(int tradingDay, int barPeriod)
{
	if (barPeriod <= 1)
	{
		return tradingDay;
	}
	else if (barPeriod == 7)
	{
		auto t = GetTm(tradingDay);
		if (t->tm_wday != 0)
		{
			auto time = mktime(t);
			time += (7LL - t->tm_wday) * 86400LL;
			t = localtime(&time);
		}
		return (t->tm_year + 1900) * 10000 + (t->tm_mon + 1) * 100 + t->tm_mday;
	}
	else if (barPeriod == 30)
	{
		return (tradingDay / 100) * 100 + 31;
	}
	else if (barPeriod == 90)
	{
		auto t = GetTm(tradingDay);
		auto mon = t->tm_mon + 1;
		if (mon % 3 != 0)
		{
			mon = ((mon / 3) + 1) * 3;
		}
		return (t->tm_year + 1900) * 10000 + mon * 100 + 31;
	}
	else if (barPeriod >= 365)
	{
		return (tradingDay / 10000) * 10000LL + 1231;
	}
	else
	{
		auto t = GetTm(tradingDay);
		auto dayCount = t->tm_yday + 1;
		if (dayCount % barPeriod == 0)
			return tradingDay;
		auto nextBarDayCount = ((dayCount / barPeriod) + 1) * barPeriod;
		auto time = mktime(t);
		time += (nextBarDayCount - dayCount) * 86400LL;
		t = localtime(&time);
		return (t->tm_year + 1900) * 10000 + (t->tm_mon + 1) * 100 + t->tm_mday;
	}
	return tradingDay;
}
int TimeUtility::CalculateMinutes(int hour, int minute)
{
	return hour * 60 + minute;
}
int TimeUtility::CalculateMinutesTimeStamp(int minutes)
{
	return (minutes / 60) * 100 + minutes % 60;
}
int TimeUtility::CalculateSeconds(int hour, int minute, int second)
{
	return hour * 3600 + minute * 60 + second;
}
int TimeUtility::CalculateSecondsTimeStamp(int seconds)
{
	auto hour = (seconds / 3600);
	seconds %= 3600;
	auto minute = (seconds / 60);
	auto second = seconds % 60;
	return hour * 10000 + minute * 100 + second;
}


int TimeUtility::CalculateNextBarDate(int barPeriod, int date)
{
	if (barPeriod == 1)
	{
		return date;
	}
	return CalculateNextBarFromDayBar(date, barPeriod);
}
long long TimeUtility::CalculateNextMinuteBarTime(long long updateTs)
{
	int date, hour, minute, second, milliSecond;
	GetDateTimeFromUpdateTs(updateTs, date, hour, minute, second, milliSecond);
	auto minutes = CalculateMinutes(hour, minute);
	minutes += 1;
	if (minutes >= 1440)
	{
		date = GetNextDate(date);
		minutes -= 1440;
	}
	long long nextBarTimeStamp = CalculateMinutesTimeStamp(minutes);
	return date * 1000000000LL + nextBarTimeStamp * 100000LL;
}
long long TimeUtility::CalculateNextSecondBarTime(int barPeriod, long long updateTs)
{
	int date, hour, minute, second, milliSecond;
	GetDateTimeFromUpdateTs(updateTs, date, hour, minute, second, milliSecond);
	auto seconds = CalculateSeconds(hour, minute, second);
	auto nextBarTimeSeconds = ((seconds + barPeriod) / barPeriod) * barPeriod;
	if (nextBarTimeSeconds >= 86400)
	{
		date = GetNextDate(date);
		nextBarTimeSeconds -= 86400;
	}
	long long nextBarTimeStamp = CalculateSecondsTimeStamp(nextBarTimeSeconds);
	return date * 1000000000LL + nextBarTimeStamp * 1000LL;
}

void TimeUtility::CalculateRealMinuteBarTime(const char* exchangeID, const char* instrumentID, int calculateBarTime, int& realBarTime, int& realUpdateTs)
{
	if (strcmp(exchangeID, "SSE") == 0 || strcmp(exchangeID, "SZSE") == 0 || strcmp(exchangeID, "CFFEX") == 0)
	{
		if (calculateBarTime <= 93000)
		{
			realBarTime = 93100;
			realUpdateTs = 93100;
		}
		else if (calculateBarTime == 113000)
		{
			realBarTime = 113000;
			realUpdateTs = 113100;
		}
		else if (calculateBarTime > 120000 && calculateBarTime <= 130000)
		{
			realBarTime = 130100;
			realUpdateTs = 130100;
		}
		else if (calculateBarTime == 150000 && instrumentID[0] != 'T')
		{
			realBarTime = 150000;
			realUpdateTs = 160000;
		}
		else if (calculateBarTime == 151500 && instrumentID[0] == 'T')
		{
			realBarTime = 151500;
			realUpdateTs = 160000;
		}
		else
		{
			realBarTime = calculateBarTime;
			realUpdateTs = calculateBarTime;
		}
	}
	else
	{
		if (calculateBarTime > 83000 && calculateBarTime <= 90000)
		{
			realBarTime = 90100;
			realUpdateTs = 90100;
		}
		else if (calculateBarTime == 101500)
		{
			realBarTime = 101500;
			realUpdateTs = 101600;
		}
		else if (calculateBarTime > 102000 && calculateBarTime < 103000)
		{
			realBarTime = 103100;
			realUpdateTs = 103100;
		}
		else if (calculateBarTime == 113000)
		{
			realBarTime = 113000;
			realUpdateTs = 113100;
		}
		else if (calculateBarTime > 130000 && calculateBarTime <= 133000)
		{
			realBarTime = 133100;
			realUpdateTs = 133100;
		}
		else if (calculateBarTime == 150000)
		{
			realBarTime = 150000;
			realUpdateTs = 160000;
		}
		else if (calculateBarTime > 203000 && calculateBarTime <= 210000)
		{
			realBarTime = 210100;
			realUpdateTs = 210100;
		}
		else if (calculateBarTime == 23000)
		{
			realBarTime = 23000;
			realUpdateTs = 23100;
		}
		else
		{
			realBarTime = calculateBarTime;
			realUpdateTs = calculateBarTime;
		}
	}
}
}
