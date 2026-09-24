#pragma once
#include <Spark/TemplateLib/ObjectPool/ObjectPool.h>
#include <Spark/Types.h>

#include <algorithm>
#include <atomic>
#include <cassert>
#include <cstddef>
#include <cstring>

namespace Spark
{
struct SingleShmHeader
{
    volatile ConnectStatusType Status;
    volatile unsigned UpWriteCount;
    volatile unsigned UpReadCount;
    volatile unsigned DownWriteCount;
    volatile unsigned DownReadCount;
};

template <unsigned Size>
class ShmBuffer
{
public:
    ShmBuffer() = default;
    ShmBuffer(ServerTypeType serverType, int index, void* shmAddr, ConnectStatusType connectStatus)
    {
        assert(index >= 0);
        ServerType = serverType;
        Index = index;
        ShmHeader = static_cast<SingleShmHeader*>(shmAddr) + index;
        ShmHeader->Status = connectStatus;
        const size_t groupOffset = static_cast<size_t>(index) * 2 * Size;
        UpBuffer = static_cast<char*>(shmAddr) + groupOffset;
        DownBuffer = UpBuffer + Size;
    }
    ~ShmBuffer()
    {
        ShmHeader = nullptr;
        UpBuffer = nullptr;
        DownBuffer = nullptr;
    }
    static ShmBuffer* Allocate(ServerTypeType serverType, int index, void* shmAddr, ConnectStatusType connectStatus)
    {
        return ObjectPool<ShmBuffer<Size>>::GetInstance().Allocate(serverType, index, shmAddr, connectStatus);
    }
    void Deallocate()
    {
        if (ShmHeader != nullptr)
        {
            if (ShmHeader->Status != ConnectStatusType::DisConnected)
            {
                ShmHeader->Status = ConnectStatusType::DisConnected;
            }
            else
            {
                ShmHeader->Status = ConnectStatusType::UnConnected;
                ShmHeader->UpWriteCount = 0;
                ShmHeader->UpReadCount = 0;
                ShmHeader->DownWriteCount = 0;
                ShmHeader->DownReadCount = 0;
            }
        }
        ObjectPool<ShmBuffer<Size>>::GetInstance().Deallocate(this);
    }

    unsigned Write(const char* source, unsigned len)
    {
        if (ServerType == ServerTypeType::Client)
            return UpWrite(source, len);
        return DownWrite(source, len);
    }
    unsigned Read(char* destination, unsigned len)
    {
        if (ServerType == ServerTypeType::Client)
            return DownRead(destination, len);
        return UpRead(destination, len);
    }

    unsigned GetWriteBufferSize() const
    {
        if (ServerType == ServerTypeType::Client)
            return GetUpWriteBufferSize();
        return GetDownWriteBufferSize();
    }
    unsigned GetReadBufferSize() const
    {
        if (ServerType == ServerTypeType::Client)
            return GetDownReadBufferSize();
        return GetUpReadBufferSize();
    }

    SingleShmHeader* ShmHeader = nullptr;
    ServerTypeType ServerType = ServerTypeType::Client;
    int Index = 0;
    char* UpBuffer = nullptr;
    char* DownBuffer = nullptr;

private:
    unsigned GetUpWriteBufferSize() const
    {
        if (ShmHeader->UpReadCount > ShmHeader->UpWriteCount)
        {
            return ShmHeader->UpReadCount - ShmHeader->UpWriteCount - 1;
        }
        return Size - (ShmHeader->UpWriteCount - ShmHeader->UpReadCount) - 1;
    }
    unsigned GetUpReadBufferSize() const
    {
        if (ShmHeader->UpWriteCount >= ShmHeader->UpReadCount)
        {
            return ShmHeader->UpWriteCount - ShmHeader->UpReadCount;
        }
        return Size - (ShmHeader->UpReadCount - ShmHeader->UpWriteCount);
    }
    unsigned GetDownWriteBufferSize() const
    {
        if (ShmHeader->DownReadCount > ShmHeader->DownWriteCount)
        {
            return ShmHeader->DownReadCount - ShmHeader->DownWriteCount - 1;
        }
        return Size - (ShmHeader->DownWriteCount - ShmHeader->DownReadCount) - 1;
    }
    unsigned GetDownReadBufferSize() const
    {
        if (ShmHeader->DownWriteCount >= ShmHeader->DownReadCount)
        {
            return ShmHeader->DownWriteCount - ShmHeader->DownReadCount;
        }
        return Size - (ShmHeader->DownReadCount - ShmHeader->DownWriteCount);
    }

    unsigned UpWrite(const char* source, unsigned len)
    {
        if (ShmHeader->Status != ConnectStatusType::Connected)
            return 0;
        auto size = GetUpWriteBufferSize();
        unsigned int currLen = std::min<unsigned>(len, size);
        if (currLen == 0)
            return 0;
        unsigned int tailLen = std::min<unsigned>(currLen, Size - ShmHeader->UpWriteCount);
        std::memcpy(UpBuffer + ShmHeader->UpWriteCount, source, tailLen);
        if (tailLen < currLen)
        {
            std::memcpy(UpBuffer, source + tailLen, currLen - tailLen);
            std::atomic_thread_fence(std::memory_order_release);
            ShmHeader->UpWriteCount = currLen - tailLen;
        }
        else
        {
            std::atomic_thread_fence(std::memory_order_release);
            ShmHeader->UpWriteCount += currLen;
        }
        return currLen;
    }
    unsigned UpRead(char* destination, unsigned len)
    {
        if (ShmHeader->Status != ConnectStatusType::Connected)
            return 0;
        auto size = GetUpReadBufferSize();
        auto currLen = std::min<unsigned>(len, size);
        if (currLen == 0)
            return 0;
        auto tailLen = std::min<unsigned>(currLen, Size - ShmHeader->UpReadCount);
        std::memcpy(destination, UpBuffer + ShmHeader->UpReadCount, tailLen);
        if (tailLen < currLen)
        {
            std::memcpy(destination + tailLen, UpBuffer, currLen - tailLen);
            std::atomic_thread_fence(std::memory_order_release);
            ShmHeader->UpReadCount = currLen - tailLen;
        }
        else
        {
            std::atomic_thread_fence(std::memory_order_release);
            ShmHeader->UpReadCount += currLen;
        }
        return currLen;
    }
    unsigned DownWrite(const char* source, unsigned len)
    {
        if (ShmHeader->Status != ConnectStatusType::Connected)
            return 0;
        auto size = GetDownWriteBufferSize();
        unsigned int currLen = std::min<unsigned>(len, size);
        if (currLen == 0)
            return 0;
        unsigned int tailLen = std::min<unsigned>(currLen, Size - ShmHeader->DownWriteCount);
        std::memcpy(DownBuffer + ShmHeader->DownWriteCount, source, tailLen);
        if (tailLen < currLen)
        {
            std::memcpy(DownBuffer, source + tailLen, currLen - tailLen);
            std::atomic_thread_fence(std::memory_order_release);
            ShmHeader->DownWriteCount = currLen - tailLen;
        }
        else
        {
            std::atomic_thread_fence(std::memory_order_release);
            ShmHeader->DownWriteCount += currLen;
        }
        return currLen;
    }
    unsigned DownRead(char* destination, unsigned len)
    {
        if (ShmHeader->Status != ConnectStatusType::Connected)
            return 0;
        auto size = GetDownReadBufferSize();
        auto currLen = std::min<unsigned>(len, size);
        if (currLen == 0)
            return 0;
        auto tailLen = std::min<unsigned>(currLen, Size - ShmHeader->DownReadCount);
        std::memcpy(destination, DownBuffer + ShmHeader->DownReadCount, tailLen);
        if (tailLen < currLen)
        {
            std::memcpy(destination + tailLen, DownBuffer, currLen - tailLen);
            std::atomic_thread_fence(std::memory_order_release);
            ShmHeader->DownReadCount = currLen - tailLen;
        }
        else
        {
            std::atomic_thread_fence(std::memory_order_release);
            ShmHeader->DownReadCount += currLen;
        }
        return currLen;
    }
};
}
