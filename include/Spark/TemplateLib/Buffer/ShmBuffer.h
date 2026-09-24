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
    static_assert(Size > 0, "Size must be greater than 0");

public:
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
    ConnectStatusType GetConnectStatus() const { return shmHeader_ != nullptr ? shmHeader_->Status : ConnectStatusType::UnConnected; }
    void SetConnectStatus(ConnectStatusType status)
    {
        if (shmHeader_ != nullptr)
        {
            shmHeader_->Status = status;
        }
    }
    void ResetSharedHeader()
    {
        if (shmHeader_ == nullptr)
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
        if (shmHeader_ == nullptr)
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

    unsigned Write(const char* source, unsigned len)
    {
        if (serverType_ == ServerTypeType::Client)
            return UpWrite(source, len);
        return DownWrite(source, len);
    }
    unsigned Read(char* destination, unsigned len)
    {
        if (serverType_ == ServerTypeType::Client)
            return DownRead(destination, len);
        return UpRead(destination, len);
    }

    unsigned GetWriteBufferSize() const
    {
        if (serverType_ == ServerTypeType::Client)
            return GetUpWriteBufferSize();
        return GetDownWriteBufferSize();
    }
    unsigned GetReadBufferSize() const
    {
        if (serverType_ == ServerTypeType::Client)
            return GetDownReadBufferSize();
        return GetUpReadBufferSize();
    }

private:
    static constexpr bool IsValidConnectionIndex(int connectionIndex) { return connectionIndex >= 1; }

    unsigned GetUpWriteBufferSize() const { return CountWritableBytes(shmHeader_->UpWriteCount, shmHeader_->UpReadCount); }
    unsigned GetUpReadBufferSize() const { return CountReadableBytes(shmHeader_->UpWriteCount, shmHeader_->UpReadCount); }
    unsigned GetDownWriteBufferSize() const { return CountWritableBytes(shmHeader_->DownWriteCount, shmHeader_->DownReadCount); }
    unsigned GetDownReadBufferSize() const { return CountReadableBytes(shmHeader_->DownWriteCount, shmHeader_->DownReadCount); }

    static unsigned CountWritableBytes(unsigned writeCount, unsigned readCount)
    {
        return readCount > writeCount ? readCount - writeCount - 1 : Size - (writeCount - readCount) - 1;
    }
    static unsigned CountReadableBytes(unsigned writeCount, unsigned readCount)
    {
        return writeCount >= readCount ? writeCount - readCount : Size - (readCount - writeCount);
    }

    unsigned UpWrite(const char* source, unsigned len)
    {
        return WriteIntoChannel(upBuffer_, shmHeader_->UpWriteCount, shmHeader_->UpReadCount, source, len);
    }
    unsigned UpRead(char* destination, unsigned len)
    {
        return ReadFromChannel(upBuffer_, shmHeader_->UpWriteCount, shmHeader_->UpReadCount, destination, len);
    }
    unsigned DownWrite(const char* source, unsigned len)
    {
        return WriteIntoChannel(downBuffer_, shmHeader_->DownWriteCount, shmHeader_->DownReadCount, source, len);
    }
    unsigned DownRead(char* destination, unsigned len)
    {
        return ReadFromChannel(downBuffer_, shmHeader_->DownWriteCount, shmHeader_->DownReadCount, destination, len);
    }

    unsigned WriteIntoChannel(char* channelBuffer, volatile unsigned& writeCount, volatile unsigned& readCount, const char* source, unsigned len)
    {
        if (shmHeader_->Status != ConnectStatusType::Connected)
            return 0;
        const unsigned writableBytes = CountWritableBytes(writeCount, readCount);
        const unsigned copiedLength = (std::min)(len, writableBytes);
        if (copiedLength == 0)
            return 0;
        std::atomic_thread_fence(std::memory_order_acquire);
        const unsigned writeIndex = writeCount;
        assert(writeIndex <= Size);
        const unsigned headLength = (std::min)(copiedLength, Size - writeIndex);
        std::memcpy(channelBuffer + writeIndex, source, headLength);
        if (headLength < copiedLength)
        {
            std::memcpy(channelBuffer, source + headLength, copiedLength - headLength);
        }
        std::atomic_thread_fence(std::memory_order_release);
        writeCount = headLength < copiedLength ? copiedLength - headLength : writeIndex + copiedLength;
        return copiedLength;
    }
    unsigned ReadFromChannel(char* channelBuffer, volatile unsigned& writeCount, volatile unsigned& readCount, char* destination, unsigned len)
    {
        if (shmHeader_->Status != ConnectStatusType::Connected)
            return 0;
        const unsigned readableBytes = CountReadableBytes(writeCount, readCount);
        const unsigned copiedLength = (std::min)(len, readableBytes);
        if (copiedLength == 0)
            return 0;
        std::atomic_thread_fence(std::memory_order_acquire);
        const unsigned readIndex = readCount;
        assert(readIndex <= Size);
        const unsigned headLength = (std::min)(copiedLength, Size - readIndex);
        std::memcpy(destination, channelBuffer + readIndex, headLength);
        if (headLength < copiedLength)
        {
            std::memcpy(destination + headLength, channelBuffer, copiedLength - headLength);
        }
        std::atomic_thread_fence(std::memory_order_release);
        readCount = headLength < copiedLength ? copiedLength - headLength : readIndex + copiedLength;
        return copiedLength;
    }

    SingleShmHeader* shmHeader_ = nullptr;
    ServerTypeType serverType_ = ServerTypeType::Client;
    char* upBuffer_ = nullptr;
    char* downBuffer_ = nullptr;
};
}
