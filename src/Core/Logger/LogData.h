#pragma once
#include <Spark/TemplateLib/Buffer/LinearBuffer.h>
#include <stdio.h>
#include <chrono>
#include <cstdint>
#include <list>
#include <memory>
#include <mutex>
#include <condition_variable>

namespace Spark::Core
{
constexpr unsigned int LogBufferSize = 1024 * 1024;

struct LogData;

// 每个写过日志的线程独占一个写入器：ChunkMutex 只保护本线程的块指针与块内追加，
// 跨线程只交接整块，避免每行争用全局锁
struct LogThreadWriter
{
public:
    ~LogThreadWriter()
    {
        for (auto* fullChunk : FullChunks)
        {
            fullChunk->Deallocate();
        }
        if (Chunk != nullptr)
        {
            Chunk->Deallocate();
        }
    }

    std::mutex ChunkMutex;
    // 当前块开始写入的时刻：消费者据此判断小块是否已攒够交接间隔，避免频繁换块
    std::chrono::steady_clock::time_point ChunkStartTime;
    Spark::LinearBuffer<LogBufferSize>* Chunk = nullptr;
    // 已写满、待消费者取走；必须排在 Chunk 之前入队，否则新块会被消费者先取走而打乱行序
    std::list<Spark::LinearBuffer<LogBufferSize>*> FullChunks;
};

struct LogData
{
public:
    LogData();
    ~LogData();

    // 单调递增：线程 TLS 用它判断写入器是否属于当前这一代，不能用 LogData 地址（分配器会复用地址）
    std::uint64_t Generation;
    FILE* LogFile;
    std::list<std::unique_ptr<LogThreadWriter>> Writers;
    std::list<Spark::LinearBuffer<LogBufferSize>*> LogBuffers;
    std::list<Spark::LinearBuffer<LogBufferSize>*> InnerLogBuffers;

    std::mutex Mutex;
    std::condition_variable ConditionVariable;
};
}
