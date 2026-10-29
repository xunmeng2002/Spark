#include "Logger/LogData.h"

#include <atomic>

using namespace Spark;
namespace Spark::Core
{
namespace
{
std::atomic<std::uint64_t> NextLogGeneration{1};
}

LogData::LogData() : Generation(NextLogGeneration.fetch_add(1, std::memory_order_relaxed))
{
    LogFile = nullptr;
}
LogData::~LogData()
{
    if (LogFile)
    {
        fclose(LogFile);
        LogFile = nullptr;
    }
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

    // 写入器析构时释放各自尚未交接的块
    Writers.clear();
}
}
