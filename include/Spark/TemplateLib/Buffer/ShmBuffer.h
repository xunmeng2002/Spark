#pragma once
#include <Spark/TemplateLib/ObjectPool/ObjectPool.h>
#include <Spark/Types.h>

#include <algorithm>
#include <atomic>
#include <cassert>
#include <cstddef>
#include <cstring>
#include <limits>

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

template <size_t Size>
class ShmBuffer
{
public:
    static_assert(Size > 0, "Size must be greater than 0");
    static_assert(Size <= (std::numeric_limits<unsigned>::max)(), "Size must fit the 32-bit shared memory counters");

    ShmBuffer() = default;
    ShmBuffer(ServerTypeType serverType, int connectionIndex, void* shmBase, ConnectStatusType connectStatus)
    {
        assert(IsValidConnectionIndex(connectionIndex));
        serverType_ = serverType;
        shmHeader_ = static_cast<SingleShmHeader*>(shmBase) + connectionIndex;
        const size_t channelOffset = static_cast<size_t>(connectionIndex) * 2 * Size;
        upBuffer_ = static_cast<char*>(shmBase) + channelOffset;
        downBuffer_ = upBuffer_ + Size;
        SetConnectStatus(connectStatus);
    }
    ~ShmBuffer() = default;
    ShmBuffer(const ShmBuffer&) = delete;
    ShmBuffer& operator=(const ShmBuffer&) = delete;
    ShmBuffer(ShmBuffer&&) = delete;
    ShmBuffer& operator=(ShmBuffer&&) = delete;

    static ShmBuffer* Allocate(ServerTypeType serverType, int connectionIndex, void* shmBase, ConnectStatusType connectStatus)
    {
        return ObjectPool<ShmBuffer<Size>>::GetInstance().Allocate(serverType, connectionIndex, shmBase, connectStatus);
    }
    void Deallocate()
    {
        if (MarkDisconnected())
        {
            ResetSharedHeader();
        }
        ObjectPool<ShmBuffer<Size>>::GetInstance().Deallocate(this);
    }

    void AttachSingleConnectionSharedMemory(void* shmBase, ServerTypeType serverType)
    {
        serverType_ = serverType;
        shmHeader_ = static_cast<SingleShmHeader*>(shmBase);
        upBuffer_ = static_cast<char*>(shmBase) + sizeof(SingleShmHeader);
        downBuffer_ = upBuffer_ + Size;
    }
    const SingleShmHeader* GetShmHeader() const { return shmHeader_; }
    ConnectStatusType GetConnectStatus() const { return IsAttached() ? shmHeader_->Status : ConnectStatusType::UnConnected; }
    void SetConnectStatus(ConnectStatusType status)
    {
        if (IsAttached())
        {
            shmHeader_->Status = status;
        }
    }
    void ResetSharedHeader()
    {
        if (!IsAttached())
        {
            return;
        }
        shmHeader_->Status = ConnectStatusType::UnConnected;
        shmHeader_->UpWriteCount = 0;
        shmHeader_->UpReadCount = 0;
        shmHeader_->DownWriteCount = 0;
        shmHeader_->DownReadCount = 0;
    }
    bool MarkDisconnected()
    {
        if (!IsAttached())
        {
            return false;
        }
        if (shmHeader_->Status == ConnectStatusType::DisConnected)
        {
            return true;
        }
        shmHeader_->Status = ConnectStatusType::DisConnected;
        return false;
    }

    size_t Write(const char* source, size_t len)
    {
        if (!IsAttached())
        {
            return 0;
        }
        if (serverType_ == ServerTypeType::Client)
            return UpWrite(source, len);
        return DownWrite(source, len);
    }
    size_t Read(char* destination, size_t len)
    {
        if (!IsAttached())
        {
            return 0;
        }
        if (serverType_ == ServerTypeType::Client)
            return DownRead(destination, len);
        return UpRead(destination, len);
    }

    size_t GetWriteBufferSize() const
    {
        if (!IsAttached())
        {
            return 0;
        }
        if (serverType_ == ServerTypeType::Client)
            return GetUpWriteBufferSize();
        return GetDownWriteBufferSize();
    }
    size_t GetReadBufferSize() const
    {
        if (!IsAttached())
        {
            return 0;
        }
        if (serverType_ == ServerTypeType::Client)
            return GetDownReadBufferSize();
        return GetUpReadBufferSize();
    }

private:
    static constexpr bool IsValidConnectionIndex(int connectionIndex) { return connectionIndex >= 1; }
    bool IsAttached() const { return shmHeader_ != nullptr; }

    size_t GetUpWriteBufferSize() const { return CountWritableBytes(shmHeader_->UpWriteCount, shmHeader_->UpReadCount); }
    size_t GetUpReadBufferSize() const { return CountReadableBytes(shmHeader_->UpWriteCount, shmHeader_->UpReadCount); }
    size_t GetDownWriteBufferSize() const { return CountWritableBytes(shmHeader_->DownWriteCount, shmHeader_->DownReadCount); }
    size_t GetDownReadBufferSize() const { return CountReadableBytes(shmHeader_->DownWriteCount, shmHeader_->DownReadCount); }

    static size_t CountWritableBytes(unsigned writeCount, unsigned readCount)
    {
        return readCount > writeCount ? readCount - writeCount - 1 : Size - (writeCount - readCount) - 1;
    }
    static size_t CountReadableBytes(unsigned writeCount, unsigned readCount)
    {
        return writeCount >= readCount ? writeCount - readCount : Size - (readCount - writeCount);
    }

    size_t UpWrite(const char* source, size_t len)
    {
        return WriteIntoChannel(upBuffer_, shmHeader_->UpWriteCount, shmHeader_->UpReadCount, source, len);
    }
    size_t UpRead(char* destination, size_t len)
    {
        return ReadFromChannel(upBuffer_, shmHeader_->UpWriteCount, shmHeader_->UpReadCount, destination, len);
    }
    size_t DownWrite(const char* source, size_t len)
    {
        return WriteIntoChannel(downBuffer_, shmHeader_->DownWriteCount, shmHeader_->DownReadCount, source, len);
    }
    size_t DownRead(char* destination, size_t len)
    {
        return ReadFromChannel(downBuffer_, shmHeader_->DownWriteCount, shmHeader_->DownReadCount, destination, len);
    }

    size_t WriteIntoChannel(char* channelBuffer, volatile unsigned& writeCount, volatile unsigned& readCount, const char* source, size_t len)
    {
        if (shmHeader_->Status != ConnectStatusType::Connected)
            return 0;
        const size_t writableBytes = CountWritableBytes(writeCount, readCount);
        const size_t copiedLength = (std::min)(len, writableBytes);
        if (copiedLength == 0)
            return 0;
        std::atomic_thread_fence(std::memory_order_acquire);
        const size_t writeIndex = writeCount;
        assert(writeIndex <= Size);
        const size_t headLength = (std::min)(copiedLength, Size - writeIndex);
        std::memcpy(channelBuffer + writeIndex, source, headLength);
        if (headLength < copiedLength)
        {
            std::memcpy(channelBuffer, source + headLength, copiedLength - headLength);
        }
        std::atomic_thread_fence(std::memory_order_release);
        writeCount = static_cast<unsigned>(headLength < copiedLength ? copiedLength - headLength : writeIndex + copiedLength);
        return copiedLength;
    }
    size_t ReadFromChannel(char* channelBuffer, volatile unsigned& writeCount, volatile unsigned& readCount, char* destination, size_t len)
    {
        if (shmHeader_->Status != ConnectStatusType::Connected)
            return 0;
        const size_t readableBytes = CountReadableBytes(writeCount, readCount);
        const size_t copiedLength = (std::min)(len, readableBytes);
        if (copiedLength == 0)
            return 0;
        std::atomic_thread_fence(std::memory_order_acquire);
        const size_t readIndex = readCount;
        assert(readIndex <= Size);
        const size_t headLength = (std::min)(copiedLength, Size - readIndex);
        std::memcpy(destination, channelBuffer + readIndex, headLength);
        if (headLength < copiedLength)
        {
            std::memcpy(destination + headLength, channelBuffer, copiedLength - headLength);
        }
        std::atomic_thread_fence(std::memory_order_release);
        readCount = static_cast<unsigned>(headLength < copiedLength ? copiedLength - headLength : readIndex + copiedLength);
        return copiedLength;
    }

    SingleShmHeader* shmHeader_ = nullptr;
    ServerTypeType serverType_ = ServerTypeType::Client;
    char* upBuffer_ = nullptr;
    char* downBuffer_ = nullptr;
};
}
