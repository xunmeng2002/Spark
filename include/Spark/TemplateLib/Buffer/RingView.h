#pragma once
#include <Spark/Types.h>

#include <algorithm>
#include <atomic>
#include <cassert>
#include <cstddef>
#include <cstring>

namespace Spark
{
template <size_t Size>
class RingView
{
public:
    static_assert(Size > 0, "Size must be greater than 0");
    static_assert((Size & (Size - 1)) == 0, "Size must be a power of two, because the position wraps by a bit mask");
    static_assert(std::atomic_ref<size_t>::is_always_lock_free, "The ring indices must be lock free, because they are shared between processes");

    RingView(char* storage, size_t& writeIndex, size_t& readIndex) : storage_(storage), writeIndex_(writeIndex), readIndex_(readIndex) {}

    size_t Read(char* destination, size_t length)
    {
        const size_t readIndex = LoadIndex(readIndex_);
        const size_t copiedLength = (std::min)(length, CountReadableBytes(readIndex));
        if (copiedLength > 0)
        {
            CopyOutOfRing(readIndex, destination, copiedLength);
            StoreIndex(readIndex_, readIndex + copiedLength);
        }
        return copiedLength;
    }
    size_t Write(const char* source, size_t length)
    {
        const size_t writeIndex = LoadIndex(writeIndex_);
        const size_t copiedLength = (std::min)(length, CountWritableBytes(writeIndex));
        if (copiedLength > 0)
        {
            CopyIntoRing(writeIndex, source, copiedLength);
            StoreIndex(writeIndex_, writeIndex + copiedLength);
        }
        return copiedLength;
    }
    size_t Peek(char* destination, size_t length) const
    {
        const size_t readIndex = LoadIndex(readIndex_);
        const size_t copiedLength = (std::min)(length, CountReadableBytes(readIndex));
        if (copiedLength > 0)
        {
            CopyOutOfRing(readIndex, destination, copiedLength);
        }
        return copiedLength;
    }
    size_t Skip(size_t length)
    {
        const size_t readIndex = LoadIndex(readIndex_);
        const size_t skippedLength = (std::min)(length, CountReadableBytes(readIndex));
        if (skippedLength > 0)
        {
            StoreIndex(readIndex_, readIndex + skippedLength);
        }
        return skippedLength;
    }
    size_t GetReadBufferSize() const { return CountReadableBytes(LoadIndex(readIndex_)); }
    size_t GetWriteBufferSize() const { return CountWritableBytes(LoadIndex(writeIndex_)); }
    bool IsEmpty() const { return GetReadBufferSize() == 0; }
    bool IsFull() const { return GetWriteBufferSize() == 0; }

private:
    static constexpr size_t Mask = Size - 1;

    static size_t LoadIndex(size_t& index) { return std::atomic_ref<size_t>(index).load(std::memory_order_acquire); }
    static void StoreIndex(size_t& index, size_t value) { std::atomic_ref<size_t>(index).store(value, std::memory_order_release); }

    size_t CountReadableBytes(size_t readIndex) const
    {
        const size_t readableBytes = LoadIndex(writeIndex_) - readIndex;
        assert(readableBytes <= Size);
        return readableBytes;
    }
    size_t CountWritableBytes(size_t writeIndex) const
    {
        const size_t inFlightBytes = writeIndex - LoadIndex(readIndex_);
        assert(inFlightBytes <= Size);
        return Size - inFlightBytes;
    }

    void CopyIntoRing(size_t writeIndex, const char* source, size_t length) const
    {
        assert(length <= Size);
        const size_t writePosition = writeIndex & Mask;
        const size_t headLength = (std::min)(length, Size - writePosition);
        std::memcpy(storage_ + writePosition, source, headLength);
        if (headLength < length)
        {
            std::memcpy(storage_, source + headLength, length - headLength);
        }
    }
    void CopyOutOfRing(size_t readIndex, char* destination, size_t length) const
    {
        assert(length <= Size);
        const size_t readPosition = readIndex & Mask;
        const size_t headLength = (std::min)(length, Size - readPosition);
        std::memcpy(destination, storage_ + readPosition, headLength);
        if (headLength < length)
        {
            std::memcpy(destination + headLength, storage_, length - headLength);
        }
    }

    char* storage_;
    size_t& writeIndex_;
    size_t& readIndex_;
};
}
