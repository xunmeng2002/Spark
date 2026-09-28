#pragma once
#include <Spark/TemplateLib/Buffer/RingView.h>

#include <atomic>
#include <cstddef>

namespace Spark
{
template <size_t Size>
class SpscRingBuffer
{
public:
    static_assert(Size > 0, "Size must be greater than 0");
    static_assert((Size & (Size - 1)) == 0, "Size must be a power of two, because the position wraps by a bit mask");

    SpscRingBuffer() = default;
    SpscRingBuffer(const SpscRingBuffer&) = delete;
    SpscRingBuffer& operator=(const SpscRingBuffer&) = delete;
    SpscRingBuffer(SpscRingBuffer&&) = delete;
    SpscRingBuffer& operator=(SpscRingBuffer&&) = delete;

    size_t Read(char* destinationBuffer, size_t length) { return ring_.Read(destinationBuffer, length); }
    size_t Write(const char* sourceBuffer, size_t length) { return ring_.Write(sourceBuffer, length); }
    size_t Peek(char* destinationBuffer, size_t length) const { return ring_.Peek(destinationBuffer, length); }
    size_t Skip(size_t length) { return ring_.Skip(length); }
    size_t GetReadBufferSize() const { return ring_.GetReadBufferSize(); }
    size_t GetWriteBufferSize() const { return ring_.GetWriteBufferSize(); }
    bool IsEmpty() const { return ring_.IsEmpty(); }
    bool IsFull() const { return ring_.IsFull(); }
    void ResetWhenIdle()
    {
        std::atomic_ref<size_t>(writeIndex_).store(0, std::memory_order_relaxed);
        std::atomic_ref<size_t>(readIndex_).store(0, std::memory_order_relaxed);
    }

private:
    alignas(64) size_t writeIndex_{0};
    alignas(64) size_t readIndex_{0};
    alignas(64) char buffer_[Size];
    RingView<Size> ring_{buffer_, writeIndex_, readIndex_};
};
}
