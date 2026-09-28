#pragma once
#include <Spark/Types.h>

#include <algorithm>
#include <atomic>
#include <cassert>
#include <cstddef>
#include <cstring>

namespace Spark
{
constexpr size_t ShmMappingMagic = 0x53504B4D;
constexpr size_t ShmMappingLayoutVersion = 2;

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
    static size_t LoadMappedField(size_t& mappedField) { return std::atomic_ref<size_t>(mappedField).load(std::memory_order_acquire); }
    static void StoreMappedField(size_t& mappedField, size_t value) { std::atomic_ref<size_t>(mappedField).store(value, std::memory_order_release); }
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
        StoreMappedField(header->UpWriteCount, 0);
        StoreMappedField(header->UpReadCount, 0);
        StoreMappedField(header->DownWriteCount, 0);
        StoreMappedField(header->DownReadCount, 0);
        StoreStatus(header, ConnectStatusType::UnConnected);
    }

    ConnectStatusType Status;
    size_t MappingMagic;
    size_t MappingLayoutVersion;
    size_t UpWriteCount;
    size_t UpReadCount;
    size_t DownWriteCount;
    size_t DownReadCount;
};

static_assert(std::atomic_ref<ConnectStatusType>::is_always_lock_free, "The shared connection status must be lock-free across processes");
static_assert(std::atomic_ref<size_t>::is_always_lock_free, "The shared counters must be lock-free across processes");
static_assert(sizeof(SingleShmHeader) == 8 + 6 * sizeof(size_t),
              "The shared header is a cross-process ABI record: a 4-byte status, 4 bytes of padding, then six 8-byte fields");
static_assert(alignof(SingleShmHeader) == alignof(size_t), "The 8-byte counters must stay naturally aligned for atomic_ref");

template <size_t Size>
class ShmBuffer
{
public:
    static_assert(Size > 0, "Size must be greater than 0");
    static_assert((Size & (Size - 1)) == 0, "Size must be a power of two, because the channel position wraps by a bit mask");

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

    static constexpr bool IsConnectionIndexWithinMapping(size_t connectionIndex, size_t connectionCount)
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
    static constexpr size_t Mask = Size - 1;

    static constexpr bool IsValidConnectionIndex(int connectionIndex, unsigned connectionCount)
    {
        return IsConnectionIndexWithinMapping(static_cast<size_t>(connectionIndex), connectionCount);
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

    static size_t CountWritableBytes(size_t writeIndex, size_t readIndex) { return Size - (writeIndex - readIndex); }
    static size_t CountReadableBytes(size_t writeIndex, size_t readIndex) { return writeIndex - readIndex; }

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

    size_t WriteIntoChannel(char* channelBuffer, size_t& writeCount, size_t& readCount, const char* source, size_t len)
    {
        if (SingleShmHeader::LoadStatus(shmHeader_) != ConnectStatusType::Connected)
            return 0;
        const size_t writeIndex = SingleShmHeader::LoadMappedField(writeCount);
        const size_t readIndex = SingleShmHeader::LoadMappedField(readCount);
        const size_t copiedLength = (std::min)(len, CountWritableBytes(writeIndex, readIndex));
        if (copiedLength == 0)
            return 0;
        CopyIntoChannel(channelBuffer, writeIndex, source, copiedLength);
        SingleShmHeader::StoreMappedField(writeCount, writeIndex + copiedLength);
        return copiedLength;
    }
    size_t ReadFromChannel(char* channelBuffer, size_t& writeCount, size_t& readCount, char* destination, size_t len)
    {
        if (SingleShmHeader::LoadStatus(shmHeader_) != ConnectStatusType::Connected)
            return 0;
        const size_t writeIndex = SingleShmHeader::LoadMappedField(writeCount);
        const size_t readIndex = SingleShmHeader::LoadMappedField(readCount);
        const size_t copiedLength = (std::min)(len, CountReadableBytes(writeIndex, readIndex));
        if (copiedLength == 0)
            return 0;
        CopyOutOfChannel(channelBuffer, readIndex, destination, copiedLength);
        SingleShmHeader::StoreMappedField(readCount, readIndex + copiedLength);
        return copiedLength;
    }
    static void CopyIntoChannel(char* channelBuffer, size_t writeIndex, const char* source, size_t length)
    {
        assert(length <= Size);
        const size_t writePosition = writeIndex & Mask;
        const size_t headLength = (std::min)(length, Size - writePosition);
        std::memcpy(channelBuffer + writePosition, source, headLength);
        if (headLength < length)
        {
            std::memcpy(channelBuffer, source + headLength, length - headLength);
        }
    }
    static void CopyOutOfChannel(char* channelBuffer, size_t readIndex, char* destination, size_t length)
    {
        assert(length <= Size);
        const size_t readPosition = readIndex & Mask;
        const size_t headLength = (std::min)(length, Size - readPosition);
        std::memcpy(destination, channelBuffer + readPosition, headLength);
        if (headLength < length)
        {
            std::memcpy(destination + headLength, channelBuffer, length - headLength);
        }
    }

    SingleShmHeader* shmHeader_ = nullptr;
    ServerTypeType serverType_ = ServerTypeType::Client;
    char* upBuffer_ = nullptr;
    char* downBuffer_ = nullptr;
};
}
