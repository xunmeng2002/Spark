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

    // 超长单行日志的边界回归：单条日志的缓冲区上限是 64KB（Logger.cpp 的 t_LogBuffer），
    // 本行按 70000 字符传入，越界写的旧行为会破坏相邻数据；紧随其后的哨兵行用于确认落盘内容完好
    string oversizedLine(70000, 'L');
    WriteLog(LogLevel::Info, "Oversized line begin:%s", oversizedLine.c_str());
    WriteLog(LogLevel::Info, "Canary after oversized line.");

    WriteLog(LogLevel::Info, "TestSpark Stop.");

    // 收尾落盘验证：紧邻 Stop() 写入、不留等待间隔，日志文件尾部应同时出现本行与 Logger 自身的
    // "Thread:Logger Exit"——两者都写在 Logger 线程最后一次 Run() 之后，靠退出路径的最后一次落盘保住
    Logger::GetInstance().Stop();
    Logger::GetInstance().Join();

    // Logger 停止后的写入应由 WriteToLog 的 m_LogData 判空静默丢弃（未 Init 的情形由宏的 nullptr 检查拦下），
    // 既不入文件也不得空指针崩溃——此行即该契约的回归点
    WriteLog(LogLevel::Info, "TestSpark WriteAfterStop.");
    return 0;
}
