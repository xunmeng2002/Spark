#include <Spark/Core/Utility/TimeUtility.h>

#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace Spark::Core
{
namespace
{
constexpr size_t DateTimeBufferSize = 32;

size_t WriteTwoDigits(char* buffer, int value)
{
    buffer[0] = static_cast<char>('0' + value / 10 % 10);
    buffer[1] = static_cast<char>('0' + value % 10);
    return 2;
}

size_t WriteThreeDigits(char* buffer, int value)
{
    buffer[0] = static_cast<char>('0' + value / 100 % 10);
    buffer[1] = static_cast<char>('0' + value / 10 % 10);
    buffer[2] = static_cast<char>('0' + value % 10);
    return 3;
}

size_t WriteFourDigits(char* buffer, int value)
{
    buffer[0] = static_cast<char>('0' + value / 1000 % 10);
    buffer[1] = static_cast<char>('0' + value / 100 % 10);
    buffer[2] = static_cast<char>('0' + value / 10 % 10);
    buffer[3] = static_cast<char>('0' + value % 10);
    return 4;
}

size_t WriteMilliSecond(char* buffer, int milliSecond)
{
    buffer[0] = '.';
    return 1 + WriteThreeDigits(buffer + 1, milliSecond);
}

// "YYYYMMDD"
size_t FormatDate(const tm& brokenDownTime, char* buffer)
{
    size_t length = WriteFourDigits(buffer, brokenDownTime.tm_year + 1900);
    length += WriteTwoDigits(buffer + length, brokenDownTime.tm_mon + 1);
    length += WriteTwoDigits(buffer + length, brokenDownTime.tm_mday);
    return length;
}

// "HH:MM:SS"
size_t FormatTime(const tm& brokenDownTime, char* buffer)
{
    size_t length = WriteTwoDigits(buffer, brokenDownTime.tm_hour);
    buffer[length++] = ':';
    length += WriteTwoDigits(buffer + length, brokenDownTime.tm_min);
    buffer[length++] = ':';
    length += WriteTwoDigits(buffer + length, brokenDownTime.tm_sec);
    return length;
}

// "YYYYMMDD-HH:MM:SS"
size_t FormatDateTime(const tm& brokenDownTime, char* buffer)
{
    const size_t length = FormatDate(brokenDownTime, buffer);
    buffer[length] = '-';
    return length + 1 + FormatTime(brokenDownTime, buffer + length + 1);
}

int FormatDateValue(const tm& brokenDownTime)
{
    return (brokenDownTime.tm_year + 1900) * 10000 + (brokenDownTime.tm_mon + 1) * 100 + brokenDownTime.tm_mday;
}

int FormatTimeValue(const tm& brokenDownTime)
{
    return brokenDownTime.tm_hour * 10000 + brokenDownTime.tm_min * 100 + brokenDownTime.tm_sec;
}

tm ToBrokenDownTime(time_t timeValue, bool isUtc)
{
    return isUtc ? *gmtime(&timeValue) : *localtime(&timeValue);
}

// 日期时间前缀一秒才变一次：缓存住，秒内只补毫秒，省掉每行一次 localtime 与格式化
struct SecondPrefixCache
{
    long long Second = -1;
    bool IsUtc = false;
    size_t Length = 0;
    char Text[TimeUtility::MaxDateTimeWithMilliSecondLength];
};

thread_local SecondPrefixCache CachedSecondPrefix;

size_t FormatSecondPrefix(bool isUtc, long long secondsSinceEpoch, char* buffer)
{
    if (CachedSecondPrefix.Second != secondsSinceEpoch || CachedSecondPrefix.IsUtc != isUtc)
    {
        const tm brokenDownTime = ToBrokenDownTime(static_cast<time_t>(secondsSinceEpoch), isUtc);
        CachedSecondPrefix.Second = secondsSinceEpoch;
        CachedSecondPrefix.IsUtc = isUtc;
        CachedSecondPrefix.Length = FormatDateTime(brokenDownTime, CachedSecondPrefix.Text);
    }
    std::memcpy(buffer, CachedSecondPrefix.Text, CachedSecondPrefix.Length);
    return CachedSecondPrefix.Length;
}
}

time_t TimeUtility::GetTime()
{
    std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
    return std::chrono::system_clock::to_time_t(now);
}
time_t TimeUtility::GetTime(const char* date)
{
    auto dateInt = atoi(date);
    tm dateTm{};
    dateTm.tm_year = dateInt / 10000 - 1900;
    dateTm.tm_mon = (dateInt / 100) % 100 - 1;
    dateTm.tm_mday = dateInt % 100;
    return mktime(&dateTm);
}
time_t TimeUtility::GetTime(int date)
{
    tm dateTm{};
    dateTm.tm_year = date / 10000 - 1900;
    dateTm.tm_mon = (date / 100) % 100 - 1;
    dateTm.tm_mday = date % 100;
    return mktime(&dateTm);
}
tm TimeUtility::GetTm(int date)
{
    return ToBrokenDownTime(GetTime(date), false);
}
tm TimeUtility::GetUtcTm()
{
    return ToBrokenDownTime(GetTime(), true);
}
tm TimeUtility::GetLocalTm()
{
    return ToBrokenDownTime(GetTime(), false);
}

void TimeUtility::GetPreTradingDay(const char* tradingDay, char* preTradingDay)
{
    time_t lastTradingDay = GetTime(tradingDay) - 86400LL;
    tm lastTradingDayTm = ToBrokenDownTime(lastTradingDay, false);
    while (lastTradingDayTm.tm_wday == 0 || lastTradingDayTm.tm_wday == 6)
    {
        lastTradingDay = lastTradingDay - 86400LL;
        lastTradingDayTm = ToBrokenDownTime(lastTradingDay, false);
    }
    preTradingDay[FormatDate(lastTradingDayTm, preTradingDay)] = '\0';
}
int TimeUtility::GetPreTradingDay(int tradingDay)
{
    time_t lastTradingDay = GetTime(tradingDay) - 86400LL;
    tm lastTradingDayTm = ToBrokenDownTime(lastTradingDay, false);
    while (lastTradingDayTm.tm_wday == 0 || lastTradingDayTm.tm_wday == 6)
    {
        lastTradingDay = lastTradingDay - 86400LL;
        lastTradingDayTm = ToBrokenDownTime(lastTradingDay, false);
    }
    return FormatDateValue(lastTradingDayTm);
}
int TimeUtility::GetPreDate(int date)
{
    return FormatDateValue(ToBrokenDownTime(GetTime(date) - 86400LL, false));
}
void TimeUtility::GetNextTradingDay(const char* tradingDay, char* nextTradingDay)
{
    time_t nextTradingDayTime = GetTime(tradingDay) + 86400LL;
    tm nextTradingDayTm = ToBrokenDownTime(nextTradingDayTime, false);
    while (nextTradingDayTm.tm_wday == 0 || nextTradingDayTm.tm_wday == 6)
    {
        nextTradingDayTime = nextTradingDayTime + 86400LL;
        nextTradingDayTm = ToBrokenDownTime(nextTradingDayTime, false);
    }
    nextTradingDay[FormatDate(nextTradingDayTm, nextTradingDay)] = '\0';
}
int TimeUtility::GetNextTradingDay(int tradingDay)
{
    time_t nextTradingDayTime = GetTime(tradingDay) + 86400LL;
    tm nextTradingDayTm = ToBrokenDownTime(nextTradingDayTime, false);
    while (nextTradingDayTm.tm_wday == 0 || nextTradingDayTm.tm_wday == 6)
    {
        nextTradingDayTime = nextTradingDayTime + 86400LL;
        nextTradingDayTm = ToBrokenDownTime(nextTradingDayTime, false);
    }
    return FormatDateValue(nextTradingDayTm);
}
int TimeUtility::GetNextDate(int date)
{
    return FormatDateValue(ToBrokenDownTime(GetTime(date) + 86400LL, false));
}
int TimeUtility::GetDateWithDayCount(int dayCount)
{
    return FormatDateValue(ToBrokenDownTime(GetTime() + 86400LL * dayCount, false));
}
int TimeUtility::GetDateWithDayCount(int date, int dayCount)
{
    return FormatDateValue(ToBrokenDownTime(GetTime(date) + 86400LL * dayCount, false));
}

int TimeUtility::DateAdd(int date, int count)
{
    return FormatDateValue(ToBrokenDownTime(GetTime(date) + 86400LL * count, false));
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
        hour -= dayCount * 24; // 归入 [0, 23]
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
        hourCount = static_cast<int>(minute / 60);
        minute %= 60;
    }
    else
    {
        // C++ 对负数除法向零截断（-1/60=0），需做 floor 除法
        hourCount = static_cast<int>((minute - 59) / 60);
        minute -= hourCount * 60; // 归入 [0, 59]
    }

    hourTime = HourAdd(static_cast<int>(hourTime), hourCount);
    return hourTime * 100 + minute;
}

void TimeUtility::GetPreYearDay(const char* tradingDay, char* preYearDay)
{
    time_t preYearDayTime = GetTime(tradingDay) - 86400LL * 365LL;
    preYearDay[FormatDate(ToBrokenDownTime(preYearDayTime, false), preYearDay)] = '\0';
}

std::string TimeUtility::GetUtcDate()
{
    char buffer[DateTimeBufferSize];
    return std::string(buffer, FormatDate(ToBrokenDownTime(GetTime(), true), buffer));
}
std::string TimeUtility::GetUtcTime()
{
    char buffer[DateTimeBufferSize];
    return std::string(buffer, FormatTime(ToBrokenDownTime(GetTime(), true), buffer));
}
std::string TimeUtility::GetUtcDateTime()
{
    char buffer[DateTimeBufferSize];
    return std::string(buffer, FormatDateTime(ToBrokenDownTime(GetTime(), true), buffer));
}
std::string TimeUtility::GetUtcDateTimeWithMilliSecond()
{
    const auto now = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now());
    const long long millisecondsSinceEpoch = now.time_since_epoch().count();
    char buffer[DateTimeBufferSize];
    size_t length = FormatSecondPrefix(true, millisecondsSinceEpoch / 1000, buffer);
    length += WriteMilliSecond(buffer + length, static_cast<int>(millisecondsSinceEpoch % 1000));
    return std::string(buffer, length);
}

std::string TimeUtility::GetLocalDate()
{
    char buffer[DateTimeBufferSize];
    return std::string(buffer, FormatDate(ToBrokenDownTime(GetTime(), false), buffer));
}
std::string TimeUtility::GetLocalTime()
{
    char buffer[DateTimeBufferSize];
    return std::string(buffer, FormatTime(ToBrokenDownTime(GetTime(), false), buffer));
}
std::string TimeUtility::GetLocalDateTime()
{
    char buffer[DateTimeBufferSize];
    return std::string(buffer, FormatDateTime(ToBrokenDownTime(GetTime(), false), buffer));
}
void TimeUtility::GetLocalDateTime(char* date, char* time)
{
    const tm localTm = ToBrokenDownTime(GetTime(), false);
    date[FormatDate(localTm, date)] = '\0';
    time[FormatTime(localTm, time)] = '\0';
}
std::string TimeUtility::GetLocalDateTimeWithMilliSecond()
{
    char buffer[MaxDateTimeWithMilliSecondLength];
    return std::string(buffer, GetLocalDateTimeWithMilliSecond(buffer, sizeof(buffer)));
}
size_t TimeUtility::GetLocalDateTimeWithMilliSecond(char* buffer, size_t capacity)
{
    if (buffer == nullptr || capacity < MaxDateTimeWithMilliSecondLength)
    {
        return 0;
    }
    const auto now = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now());
    const long long millisecondsSinceEpoch = now.time_since_epoch().count();
    size_t length = FormatSecondPrefix(false, millisecondsSinceEpoch / 1000, buffer);
    return length + WriteMilliSecond(buffer + length, static_cast<int>(millisecondsSinceEpoch % 1000));
}
long long TimeUtility::GetMilliSecondTimeStamp()
{
    const auto now = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now());
    const long long millisecondsSinceEpoch = now.time_since_epoch().count();
    const tm localTm = ToBrokenDownTime(static_cast<time_t>(millisecondsSinceEpoch / 1000), false);
    return static_cast<long long>(FormatDateValue(localTm)) * 1000000000LL + static_cast<long long>(FormatTimeValue(localTm)) * 1000LL +
           millisecondsSinceEpoch % 1000;
}
void TimeUtility::GetDateTimeFromTimeStamp(const Int64Type& timeStamp, DateType& date, TimeType& time)
{
    snprintf(date, sizeof(DateType), "%lld", timeStamp / 1000000000LL);
    snprintf(time, sizeof(TimeType), "%lld", (timeStamp / 1000LL) % 1000000LL);
}

int TimeUtility::GetTimeFromTimeString(const char* time)
{
    int hour = 0;
    int minute = 0;
    int second = 0;
    if (std::sscanf(time, "%d:%d:%d", &hour, &minute, &second) != 3)
    {
        return 0;
    }
    return hour * 10000 + minute * 100 + second;
}
time_t TimeUtility::GetTimeFromString(std::string_view dateTime, std::string_view format)
{
    // sscanf 与 mktime 都要 NUL 结尾：视图先各自落成字符串，二者只在本函数内用
    const std::string dateTimeText(dateTime);
    const std::string formatText(format);
    tm brokenDownTime{};
    if (std::sscanf(dateTimeText.c_str(), formatText.c_str(), &brokenDownTime.tm_year, &brokenDownTime.tm_mon, &brokenDownTime.tm_mday,
                    &brokenDownTime.tm_hour, &brokenDownTime.tm_min, &brokenDownTime.tm_sec) != 6)
    {
        return -1;
    }
    brokenDownTime.tm_year -= 1900;
    brokenDownTime.tm_mon -= 1;
    brokenDownTime.tm_isdst = -1;
    return mktime(&brokenDownTime);
}
std::string TimeUtility::ToUtcDate(time_t* time)
{
    char buffer[DateTimeBufferSize];
    return std::string(buffer, FormatDate(ToBrokenDownTime(*time, true), buffer));
}
std::string TimeUtility::ToUtcTime(time_t* time)
{
    char buffer[DateTimeBufferSize];
    return std::string(buffer, FormatTime(ToBrokenDownTime(*time, true), buffer));
}
std::string TimeUtility::ToUtcDateTime(time_t* time)
{
    char buffer[DateTimeBufferSize];
    return std::string(buffer, FormatDateTime(ToBrokenDownTime(*time, true), buffer));
}
std::string TimeUtility::ToLocalDate(time_t* time)
{
    char buffer[DateTimeBufferSize];
    return std::string(buffer, FormatDate(ToBrokenDownTime(*time, false), buffer));
}
std::string TimeUtility::ToLocalTime(time_t* time)
{
    char buffer[DateTimeBufferSize];
    return std::string(buffer, FormatTime(ToBrokenDownTime(*time, false), buffer));
}
std::string TimeUtility::ToLocalDateTime(time_t* time)
{
    char buffer[DateTimeBufferSize];
    return std::string(buffer, FormatDateTime(ToBrokenDownTime(*time, false), buffer));
}

void TimeUtility::GetDateTimeFromUpdateTs(long long updateTs, int& date, int& hour, int& minute, int& second, int& milliSecond)
{
    milliSecond = static_cast<int>(updateTs % 1000LL);
    updateTs /= 1000LL;
    second = static_cast<int>(updateTs % 100LL);
    updateTs /= 100LL;
    minute = static_cast<int>(updateTs % 100LL);
    updateTs /= 100LL;
    hour = static_cast<int>(updateTs % 100LL);
    date = static_cast<int>(updateTs / 100LL);
}
int TimeUtility::CalculateNextBarFromDayBar(int tradingDay, int barPeriod)
{
    if (barPeriod <= 1)
    {
        return tradingDay;
    }
    else if (barPeriod == 7)
    {
        tm brokenDownTime = GetTm(tradingDay);
        if (brokenDownTime.tm_wday != 0)
        {
            auto time = mktime(&brokenDownTime);
            time += (7LL - brokenDownTime.tm_wday) * 86400LL;
            brokenDownTime = ToBrokenDownTime(time, false);
        }
        return FormatDateValue(brokenDownTime);
    }
    else if (barPeriod == 30)
    {
        return (tradingDay / 100) * 100 + 31;
    }
    else if (barPeriod == 90)
    {
        const tm brokenDownTime = GetTm(tradingDay);
        auto mon = brokenDownTime.tm_mon + 1;
        if (mon % 3 != 0)
        {
            mon = ((mon / 3) + 1) * 3;
        }
        return (brokenDownTime.tm_year + 1900) * 10000 + mon * 100 + 31;
    }
    else if (barPeriod >= 365)
    {
        return (tradingDay / 10000) * 10000LL + 1231;
    }
    else
    {
        tm brokenDownTime = GetTm(tradingDay);
        auto dayCount = brokenDownTime.tm_yday + 1;
        if (dayCount % barPeriod == 0)
            return tradingDay;
        auto nextBarDayCount = ((dayCount / barPeriod) + 1) * barPeriod;
        auto time = mktime(&brokenDownTime);
        time += (nextBarDayCount - dayCount) * 86400LL;
        brokenDownTime = ToBrokenDownTime(time, false);
        return FormatDateValue(brokenDownTime);
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

void TimeUtility::CalculateRealMinuteBarTime(const char* exchangeId, const char* instrumentId, int calculateBarTime, int& realBarTime,
                                             int& realUpdateTs)
{
    if (strcmp(exchangeId, "SSE") == 0 || strcmp(exchangeId, "SZSE") == 0 || strcmp(exchangeId, "CFFEX") == 0)
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
        else if (calculateBarTime == 150000 && instrumentId[0] != 'T')
        {
            realBarTime = 150000;
            realUpdateTs = 160000;
        }
        else if (calculateBarTime == 151500 && instrumentId[0] == 'T')
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
