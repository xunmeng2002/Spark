#pragma once
#include <Spark/Types.h>

#include <algorithm>
#include <atomic>
#include <cassert>
#include <cstddef>
#include <cstring>
#include <limits>

namespace Spark
{
constexpr unsigned ShmMappingMagic = 0x53504B4D;
constexpr unsigned ShmMappingLayoutVersion = 1;

struct SingleShmHeader
{
    static ConnectStatusType LoadStatus(SingleShmHeader* header)
    {
        return std::atomic_ref<ConnectStatusType>(header->Status).load(std::memory_order_acquire);
    }
    static void StoreStatus(SingleShmHeader* header, ConnectStatusType status)
    {
        std::atomic_ref<ConnectStatusType>(header->Status).store(status, std::memory_order_release);
    }
    static std::atomic_ref<ConnectStatusType> StatusReference(SingleShmHeader* header) { return std::atomic_ref<ConnectStatusType>(header->Status); }
    static unsigned LoadMappedField(unsigned& mappedField) { return std::atomic_ref<unsigned>(mappedField).load(std::memory_order_acquire); }
    static void StoreMappedField(unsigned& mappedField, unsigned value)
    {
        std::atomic_ref<unsigned>(mappedField).store(value, std::memory_order_release);
    }
    static bool IsMappingLayoutCompatible(SingleShmHeader* header)
    {
        return LoadMappedField(header->MappingMagic) == ShmMappingMagic && LoadMappedField(header->MappingLayoutVersion) == ShmMappingLayoutVersion;
    }
    static void StoreMappingStamp(SingleShmHeader* header)
    {
        StoreMappedField(header->MappingMagic, ShmMappingMagic);
        StoreMappedField(header->MappingLayoutVersion, ShmMappingLayoutVersion);
    }
    static void ResetChannelHeader(SingleShmHeader* header)
    {
        StoreStatus(header, ConnectStatusType::UnConnected);
        StoreMappedField(header->UpWriteCount, 0);
        StoreMappedField(header->UpReadCount, 0);
        StoreMappedField(header->DownWriteCount, 0);
        StoreMappedField(header->DownReadCount, 0);
    }

    ConnectStatusType Status;
    unsigned MappingMagic;
    unsigned MappingLayoutVersion;
    unsigned UpWriteCount;
    unsigned UpReadCount;
    unsigned DownWriteCount;
    unsigned DownReadCount;
};

static_assert(std::atomic_ref<ConnectStatusType>::is_always_lock_free, "The shared connection status must be lock-free across processes");
static_assert(std::atomic_ref<unsigned>::is_always_lock_free, "The shared counters must be lock-free across processes");

template <size_t Size>
class ShmBuffer
{
public:
    static_assert(Size > 0, "Size must be greater than 0");
    static_assert(Size <= (std::numeric_limits<unsigned>::max)(), "Size must fit the 32-bit shared memory counters");

    ShmBuffer() = default;
    ShmBuffer(ServerTypeType serverType, int connectionIndex, void* shmBase, ConnectStatusType connectStatus, unsigned connectionCount)
    {
        assert(IsValidConnectionIndex(connectionIndex, connectionCount));
        if (!IsValidConnectionIndex(connectionIndex, connectionCount) || shmBase == nullptr)
        {
            return;
        }
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

    static constexpr bool IsConnectionIndexWithinMapping(unsigned connectionIndex, unsigned connectionCount)
    {
        return connectionIndex >= 1 && connectionIndex < connectionCount;
    }

    const SingleShmHeader* GetShmHeader() const { return shmHeader_; }
    ConnectStatusType GetConnectStatus() const { return IsAttached() ? SingleShmHeader::LoadStatus(shmHeader_) : ConnectStatusType::UnConnected; }
    void SetConnectStatus(ConnectStatusType status)
    {
        if (IsAttached())
        {
            SingleShmHeader::StoreStatus(shmHeader_, status);
        }
    }
    void ResetSharedHeader()
    {
        if (IsAttached())
        {
            SingleShmHeader::ResetChannelHeader(shmHeader_);
        }
    }
    bool MarkDisconnectedAndReportWhetherLastHolder()
    {
        if (!IsAttached())
        {
            return false;
        }
        std::atomic_ref<ConnectStatusType> connectionStatus = SingleShmHeader::StatusReference(shmHeader_);
        ConnectStatusType expectedStatus = connectionStatus.load(std::memory_order_acquire);
        while (expectedStatus != ConnectStatusType::DisConnected)
        {
            if (connectionStatus.compare_exchange_weak(expectedStatus, ConnectStatusType::DisConnected, std::memory_order_acq_rel,
                                                       std::memory_order_acquire))
            {
                return false;
            }
        }
        return true;
    }
    void MarkDisconnectedAndResetChannelWhenLastHolder()
    {
        if (MarkDisconnectedAndReportWhetherLastHolder())
        {
            ResetSharedHeader();
        }
    }
    bool RevokeUnconfirmedAccept()
    {
        if (!IsAttached())
        {
            return false;
        }
        ConnectStatusType expectedStatus = ConnectStatusType::Accepted;
        return SingleShmHeader::StatusReference(shmHeader_)
            .compare_exchange_strong(expectedStatus, ConnectStatusType::DisConnected, std::memory_order_acq_rel, std::memory_order_acquire);
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
    static constexpr bool IsValidConnectionIndex(int connectionIndex, unsigned connectionCount)
    {
        return IsConnectionIndexWithinMapping(static_cast<unsigned>(connectionIndex), connectionCount);
    }
    bool IsAttached() const { return shmHeader_ != nullptr; }

    size_t GetUpWriteBufferSize() const
    {
        return CountWritableBytes(SingleShmHeader::LoadMappedField(shmHeader_->UpWriteCount),
                                  SingleShmHeader::LoadMappedField(shmHeader_->UpReadCount));
    }
    size_t GetUpReadBufferSize() const
    {
        return CountReadableBytes(SingleShmHeader::LoadMappedField(shmHeader_->UpWriteCount),
                                  SingleShmHeader::LoadMappedField(shmHeader_->UpReadCount));
    }
    size_t GetDownWriteBufferSize() const
    {
        return CountWritableBytes(SingleShmHeader::LoadMappedField(shmHeader_->DownWriteCount),
                                  SingleShmHeader::LoadMappedField(shmHeader_->DownReadCount));
    }
    size_t GetDownReadBufferSize() const
    {
        return CountReadableBytes(SingleShmHeader::LoadMappedField(shmHeader_->DownWriteCount),
                                  SingleShmHeader::LoadMappedField(shmHeader_->DownReadCount));
    }

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

    size_t WriteIntoChannel(char* channelBuffer, unsigned& writeCount, unsigned& readCount, const char* source, size_t len)
    {
        if (SingleShmHeader::LoadStatus(shmHeader_) != ConnectStatusType::Connected)
            return 0;
        const unsigned writeIndex = SingleShmHeader::LoadMappedField(writeCount);
        const unsigned readIndex = SingleShmHeader::LoadMappedField(readCount);
        const size_t writableBytes = CountWritableBytes(writeIndex, readIndex);
        const size_t copiedLength = (std::min)(len, writableBytes);
        if (copiedLength == 0)
            return 0;
        assert(writeIndex <= Size);
        const size_t headLength = (std::min)(copiedLength, Size - writeIndex);
        std::memcpy(channelBuffer + writeIndex, source, headLength);
        if (headLength < copiedLength)
        {
            std::memcpy(channelBuffer, source + headLength, copiedLength - headLength);
        }
        SingleShmHeader::StoreMappedField(writeCount,
                                          static_cast<unsigned>(headLength < copiedLength ? copiedLength - headLength : writeIndex + copiedLength));
        return copiedLength;
    }
    size_t ReadFromChannel(char* channelBuffer, unsigned& writeCount, unsigned& readCount, char* destination, size_t len)
    {
        if (SingleShmHeader::LoadStatus(shmHeader_) != ConnectStatusType::Connected)
            return 0;
        const unsigned writeIndex = SingleShmHeader::LoadMappedField(writeCount);
        const unsigned readIndex = SingleShmHeader::LoadMappedField(readCount);
        const size_t readableBytes = CountReadableBytes(writeIndex, readIndex);
        const size_t copiedLength = (std::min)(len, readableBytes);
        if (copiedLength == 0)
            return 0;
        assert(readIndex <= Size);
        const size_t headLength = (std::min)(copiedLength, Size - readIndex);
        std::memcpy(destination, channelBuffer + readIndex, headLength);
        if (headLength < copiedLength)
        {
            std::memcpy(destination + headLength, channelBuffer, copiedLength - headLength);
        }
        SingleShmHeader::StoreMappedField(readCount,
                                          static_cast<unsigned>(headLength < copiedLength ? copiedLength - headLength : readIndex + copiedLength));
        return copiedLength;
    }

    SingleShmHeader* shmHeader_ = nullptr;
    ServerTypeType serverType_ = ServerTypeType::Client;
    char* upBuffer_ = nullptr;
    char* downBuffer_ = nullptr;
};
}
