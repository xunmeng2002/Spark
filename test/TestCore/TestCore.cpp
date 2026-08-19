#include <Spark/Core/MD5/MD5.h>
#include <Spark/Core/Logger/Logger.h>
#include <Spark/Core/Aspect/Aspect.h>
#include <Spark/Core/Aspect/AspectLogger.h>
#include <Spark/Core/Aspect/AspectPerformance.h>
#include <iostream>
using namespace std;
using namespace spark::core;

void CalcalateMD5(const char* src, int len)
{
    string s = getMD5((const unsigned char*)src, len);
    WriteLog(LogLevel::Info, "getMD5 Result:%s", s.c_str());
}

int main(int argc, const char* argv[])
{
    Logger::GetInstance().Init(argv[0]);
    Logger::GetInstance().SetLogLevel(LogLevel::Info, LogLevel::Info);
    Logger::GetInstance().Start();

    WriteLog(LogLevel::Info, "TestSpark Start.");

    string src = "HelloWorldHelloWorldHelloWorldHelloWorldHelloWorldHelloWorld";

    Invoke<AspectLogger, AspectPerformance>(CalcalateMD5, "CalcalateMD5", src.c_str(), src.length());

    WriteLog(LogLevel::Info, "TestSpark Stop.");

    this_thread::sleep_for(chrono::seconds(1));
    Logger::GetInstance().Stop();
    Logger::GetInstance().Join();
    return 0;
}
