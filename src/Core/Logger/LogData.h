#pragma once
#include <Spark/TemplateLib/Buffer/Buffer.h>
#include <stdio.h>
#include <list>
#include <mutex>
#include <condition_variable>


namespace Spark::Core
{
struct LogData
{
    LogData();
    ~LogData();

    void PushBuffer();

    FILE* LogFile;
    Spark::Buffer<LogBuffSize>* CurrBuffer;
    std::list<Spark::Buffer<LogBuffSize>*> LogBuffers;
    std::list<Spark::Buffer<LogBuffSize>*> InnerLogBuffers;

    std::mutex Mutex;
    std::condition_variable ConditionVariable;
};
}
