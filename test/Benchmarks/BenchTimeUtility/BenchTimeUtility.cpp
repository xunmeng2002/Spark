#include <Spark/Core/Utility/TimeUtility.h>
#include <Spark/TemplateLib/Benchmark/BenchmarkRunner.h>

#include <chrono>
#include <cstddef>
#include <ctime>

using Spark::Core::TimeUtility;
namespace Benchmark = Spark::Benchmark;

// reference.* 行保留改造前的实现口径（time + localtime + strftime 整条），
// 用来随时检验"手写格式化比标准库快多少"，不受 TimeUtility 自身改动影响
int main()
{
    Benchmark::PrintHeader();

    Benchmark::Measure("TimeUtility.GetLocalDateTimeWithMilliSecond_string", []() { return TimeUtility::GetLocalDateTimeWithMilliSecond().size(); });
    Benchmark::Measure("TimeUtility.GetLocalDateTimeWithMilliSecond_buffer",
                       []()
                       {
                           char dateTimeText[TimeUtility::MaxDateTimeWithMilliSecondLength];
                           return TimeUtility::GetLocalDateTimeWithMilliSecond(dateTimeText, sizeof(dateTimeText));
                       });
    Benchmark::Measure("TimeUtility.GetLocalDateTime", []() { return TimeUtility::GetLocalDateTime().size(); });
    Benchmark::Measure("TimeUtility.GetLocalDate", []() { return TimeUtility::GetLocalDate().size(); });
    Benchmark::Measure("TimeUtility.GetLocalTime", []() { return TimeUtility::GetLocalTime().size(); });
    Benchmark::Measure("TimeUtility.GetLocalTm", []() { return static_cast<size_t>(TimeUtility::GetLocalTm().tm_sec); });
    Benchmark::Measure("TimeUtility.GetMilliSecondTimeStamp", []() { return static_cast<size_t>(TimeUtility::GetMilliSecondTimeStamp()); });
    Benchmark::Measure("TimeUtility.GetTimeFromTimeString", []() { return static_cast<size_t>(TimeUtility::GetTimeFromTimeString("10:30:45")); });
    Benchmark::Measure("TimeUtility.DateAdd", []() { return static_cast<size_t>(TimeUtility::DateAdd(20240115, 3)); });

    Benchmark::Measure("reference.clock_now",
                       []()
                       {
                           const auto now = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now());
                           return static_cast<size_t>(now.time_since_epoch().count());
                       });
    Benchmark::Measure("reference.localtime",
                       []()
                       {
                           const std::time_t now = std::time(nullptr);
                           return static_cast<size_t>(std::localtime(&now)->tm_sec);
                       });
    Benchmark::Measure("reference.strftime_whole_string",
                       []()
                       {
                           const std::time_t now = std::time(nullptr);
                           const std::tm brokenDownTime = *std::localtime(&now);
                           char buffer[32];
                           return std::strftime(buffer, sizeof(buffer), "%Y%m%d-%H:%M:%S", &brokenDownTime);
                       });
    return 0;
}
