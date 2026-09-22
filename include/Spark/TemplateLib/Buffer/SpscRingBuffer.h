#pragma once
#include <algorithm>
#include <atomic>
#include <cassert>
#include <cstddef>
#include <cstring>

namespace Spark
{
template <size_t Size>
class SpscRingBuffer
{
    static_assert(Size > 0, "Size must be greater than 0");
    static_assert((Size & (Size - 1)) == 0, "Size must be a power of two, because the wrap-around uses a bit mask");
    static_assert(std::atomic<size_t>::is_always_lock_free, "size_t atomic must be lock free");

public:
    SpscRingBuffer() = default;
    SpscRingBuffer(const SpscRingBuffer&) = delete;
    SpscRingBuffer& operator=(const SpscRingBuffer&) = delete;
    SpscRingBuffer(SpscRingBuffer&&) = delete;
    SpscRingBuffer& operator=(SpscRingBuffer&&) = delete;

    size_t Read(char* destinationBuffer, size_t length)
    {
        const size_t readIndex = readIndex_.load(std::memory_order_relaxed);
        const size_t copiedLength = (std::min)(length, CountReadableBytes(readIndex));
        if (copiedLength > 0)
        {
            CopyOutOfRing(readIndex, destinationBuffer, copiedLength);
            readIndex_.store(readIndex + copiedLength, std::memory_order_release);
        }
        return copiedLength;
    }
    size_t Write(const char* sourceBuffer, size_t length)
    {
        const size_t writeIndex = writeIndex_.load(std::memory_order_relaxed);
        const size_t copiedLength = (std::min)(length, CountWritableBytes(writeIndex));
        if (copiedLength > 0)
        {
            CopyIntoRing(writeIndex, sourceBuffer, copiedLength);
            writeIndex_.store(writeIndex + copiedLength, std::memory_order_release);
        }
        return copiedLength;
    }
    size_t Peek(char* destinationBuffer, size_t length) const
    {
        const size_t readIndex = readIndex_.load(std::memory_order_relaxed);
        const size_t copiedLength = (std::min)(length, CountReadableBytes(readIndex));
        if (copiedLength > 0)
        {
            CopyOutOfRing(readIndex, destinationBuffer, copiedLength);
        }
        return copiedLength;
    }
    size_t Skip(size_t length)
    {
        const size_t readIndex = readIndex_.load(std::memory_order_relaxed);
        const size_t skippedLength = (std::min)(length, CountReadableBytes(readIndex));
        if (skippedLength > 0)
        {
            readIndex_.store(readIndex + skippedLength, std::memory_order_release);
        }
        return skippedLength;
    }
    size_t GetReadBufferSize() const { return CountReadableBytes(readIndex_.load(std::memory_order_relaxed)); }
    size_t GetWriteBufferSize() const { return CountWritableBytes(writeIndex_.load(std::memory_order_relaxed)); }
    bool IsEmpty() const { return GetReadBufferSize() == 0; }
    bool IsFull() const { return GetWriteBufferSize() == 0; }
    void ResetWhenIdle()
    {
        writeIndex_.store(0, std::memory_order_relaxed);
        readIndex_.store(0, std::memory_order_relaxed);
    }

private:
    static constexpr size_t Mask = Size - 1;

    size_t CountReadableBytes(size_t readIndex) const
    {
        return writeIndex_.load(std::memory_order_acquire) - readIndex;
    }
    size_t CountWritableBytes(size_t writeIndex) const
    {
        return Size - (writeIndex - readIndex_.load(std::memory_order_acquire));
    }
    void CopyIntoRing(size_t writeIndex, const char* sourceBuffer, size_t length)
    {
        assert(length <= Size);
        const size_t writePosition = writeIndex & Mask;
        const size_t headLength = (std::min)(length, Size - writePosition);
        std::memcpy(buffer_ + writePosition, sourceBuffer, headLength);
        if (headLength < length)
        {
            std::memcpy(buffer_, sourceBuffer + headLength, length - headLength);
        }
    }
    void CopyOutOfRing(size_t readIndex, char* destinationBuffer, size_t length) const
    {
        assert(length <= Size);
        const size_t readPosition = readIndex & Mask;
        const size_t headLength = (std::min)(length, Size - readPosition);
        std::memcpy(destinationBuffer, buffer_ + readPosition, headLength);
        if (headLength < length)
        {
            std::memcpy(destinationBuffer + headLength, buffer_, length - headLength);
        }
    }

    alignas(64) std::atomic<size_t> writeIndex_{0};
    alignas(64) std::atomic<size_t> readIndex_{0};
    alignas(64) char buffer_[Size];
};
}
