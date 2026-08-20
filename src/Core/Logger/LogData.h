#pragma once
#include <Spark/TemplateLib/Buffer/Buffer.h>
#include <stdio.h>
#include <list>
#include <mutex>
#include <condition_variable>


namespace spark::core
{
struct LogData
{
	LogData();
	~LogData();

	void PushBuffer();

	FILE* LogFile;
    spark::Buffer<LogBuffSize>* CurrBuffer;
	std::list<spark::Buffer<LogBuffSize>*> LogBuffers;
	std::list<spark::Buffer<LogBuffSize>*> InnerLogBuffers;

	std::mutex Mutex;
	std::condition_variable ConditionVariable;
};
}
