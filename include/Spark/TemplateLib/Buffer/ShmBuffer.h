#pragma once
#include <Spark/TemplateLib/Buffer/RingView.h>
#include <Spark/Types.h>

#include <atomic>
#include <cassert>
#include <cstddef>

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
    static bool ChangeStatusIfEqualTo(SingleShmHeader* header, ConnectStatusType expectedStatus, ConnectStatusType targetStatus)
    {
        return StatusReference(header).compare_exchange_strong(expectedStatus, targetStatus, std::memory_order_acq_rel, std::memory_order_acquire);
    }
    static bool ConfirmAcceptedConnection(SingleShmHeader* channelHeader)
    {
        return ChangeStatusIfEqualTo(channelHeader, ConnectStatusType::Accepted, ConnectStatusType::Connected);
    }
    static bool RevokeConfirmedConnection(SingleShmHeader* channelHeader)
    {
        return ChangeStatusIfEqualTo(channelHeader, ConnectStatusType::Connected, ConnectStatusType::DisConnected);
    }
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
        return SingleShmHeader::ChangeStatusIfEqualTo(shmHeader_, ConnectStatusType::Accepted, ConnectStatusType::DisConnected);
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
        return IsConnectionIndexWithinMapping(static_cast<size_t>(connectionIndex), connectionCount);
    }
    bool IsAttached() const { return shmHeader_ != nullptr; }
    bool IsChannelConnected() const { return SingleShmHeader::LoadStatus(shmHeader_) == ConnectStatusType::Connected; }

    RingView<Size> UpRing() const { return RingView<Size>(upBuffer_, shmHeader_->UpWriteCount, shmHeader_->UpReadCount); }
    RingView<Size> DownRing() const { return RingView<Size>(downBuffer_, shmHeader_->DownWriteCount, shmHeader_->DownReadCount); }

    size_t GetUpWriteBufferSize() const { return UpRing().GetWriteBufferSize(); }
    size_t GetUpReadBufferSize() const { return UpRing().GetReadBufferSize(); }
    size_t GetDownWriteBufferSize() const { return DownRing().GetWriteBufferSize(); }
    size_t GetDownReadBufferSize() const { return DownRing().GetReadBufferSize(); }

    size_t UpWrite(const char* source, size_t len) { return IsChannelConnected() ? UpRing().Write(source, len) : 0; }
    size_t UpRead(char* destination, size_t len) { return IsChannelConnected() ? UpRing().Read(destination, len) : 0; }
    size_t DownWrite(const char* source, size_t len) { return IsChannelConnected() ? DownRing().Write(source, len) : 0; }
    size_t DownRead(char* destination, size_t len) { return IsChannelConnected() ? DownRing().Read(destination, len) : 0; }

    SingleShmHeader* shmHeader_ = nullptr;
    ServerTypeType serverType_ = ServerTypeType::Client;
    char* upBuffer_ = nullptr;
    char* downBuffer_ = nullptr;
};
}
