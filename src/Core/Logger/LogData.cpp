#include "Logger/LogData.h"

using namespace Spark;
namespace Spark::Core
{
LogData::LogData()
{
    LogFile = nullptr;
    CurrBuffer = LinearBuffer<LogBufferSize>::Allocate();
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
    CurrBuffer = LinearBuffer<LogBufferSize>::Allocate();
}
}
