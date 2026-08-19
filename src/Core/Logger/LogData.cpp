#include "Logger/LogData.h"

using namespace spark;
namespace spark::core
{
LogData::LogData()
{
	LogFile = nullptr;
	CurrBuffer = Buffer<LogBuffSize>::Allocate();
}
LogData::~LogData()
{
	if (LogFile)
	{
		fclose(LogFile);
		LogFile = nullptr;
	}
	CurrBuffer->Deallocate();
	for (auto& logBuffer : LogBuffers)
	{
		logBuffer->Deallocate();
	}
	LogBuffers.clear();

	for (auto& logBuffer : InnerLogBuffers)
	{
		logBuffer->Deallocate();
	}
	InnerLogBuffers.clear();
}
void LogData::PushBuffer()
{
	LogBuffers.push_back(CurrBuffer);
	CurrBuffer = Buffer<LogBuffSize>::Allocate();
}
}
