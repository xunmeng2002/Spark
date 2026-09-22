#pragma once
#include <Spark/TemplateLib/Buffer/LinearBuffer.h>
#include <stdio.h>
#include <list>
#include <mutex>
#include <condition_variable>

namespace Spark::Core
{
constexpr unsigned int LogBufferSize = 1024 * 1024;

struct LogData
{
    LogData();
    ~LogData();

    void PushBuffer();

    FILE* LogFile;
    Spark::LinearBuffer<LogBufferSize>* CurrBuffer;
    std::list<Spark::LinearBuffer<LogBufferSize>*> LogBuffers;
    std::list<Spark::LinearBuffer<LogBufferSize>*> InnerLogBuffers;

    std::mutex Mutex;
    std::condition_variable ConditionVariable;
};
}
