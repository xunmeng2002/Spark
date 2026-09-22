#pragma once
#include <Spark/TemplateLib/Buffer/Buffer.h>
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
    Spark::Buffer<LogBufferSize>* CurrBuffer;
    std::list<Spark::Buffer<LogBufferSize>*> LogBuffers;
    std::list<Spark::Buffer<LogBufferSize>*> InnerLogBuffers;

    std::mutex Mutex;
    std::condition_variable ConditionVariable;
};
}
