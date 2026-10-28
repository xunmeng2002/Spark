#pragma once

namespace Spark::Test
{
// 子进程模式下从命令行取出的探针名；未请求子进程模式时返回 nullptr
const char* FindAssertAbortProbeName(int argc, char** argv);

// 执行 probeName 对应的断言触发路径。命中断言时进程在此终止；名字未知则正常返回
void RunAssertAbortProbe(const char* probeName);

// 以子进程方式跑探针，核对子进程是否因 expectedMessage 断言而异常终止
bool ObserveAssertAbortInChildProcess(const char* probeName, const char* expectedMessage);
}
