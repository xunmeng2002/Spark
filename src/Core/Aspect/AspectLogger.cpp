#include <Spark/Core/Aspect/AspectLogger.h>
#include <Spark/Core/Logger/Logger.h>


namespace spark::core
{
void AspectLogger::Before(const char* funcName)
{
	WriteLog(LogLevel::Info, "Before Call %s", funcName);
}
void AspectLogger::After(const char* funcName)
{
	WriteLog(LogLevel::Info, "After Call %s", funcName);
}
}
