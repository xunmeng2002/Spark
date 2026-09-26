#pragma once
#include <Spark/TemplateLib/ObjectPool/ObjectPool.h>

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstring>

namespace Spark
{
template <size_t Size>
class LinearBuffer
{
public:
    static_assert(Size > 0, "Size must be greater than 0");

    LinearBuffer() : buffer_{0} { ClearIndices(); }
    LinearBuffer(const LinearBuffer&) = delete;
    LinearBuffer& operator=(const LinearBuffer&) = delete;
    LinearBuffer(LinearBuffer&&) = delete;
    LinearBuffer& operator=(LinearBuffer&&) = delete;

    static LinearBuffer* Allocate() { return ObjectPool<LinearBuffer<Size>>::GetInstance().Allocate(); }
    void Deallocate() { ObjectPool<LinearBuffer<Size>>::GetInstance().Deallocate(this); }

    size_t Append(const char* source, size_t len)
    {
        len = (std::min)(len, GetWriteBufferSize());
        if (len > 0)
        {
            std::memcpy(buffer_ + writeIndex_, source, len);
            writeIndex_ += len;
        }
        return len;
    }
    void SetLength(size_t len)
    {
        const size_t capacity = Size - readIndex_;
        assert(len <= capacity);
        writeIndex_ = readIndex_ + (std::min)(len, capacity);
    }
    void Shift(size_t len)
    {
        if (len >= GetLength())
        {
            ClearIndices();
        }
        else
        {
            readIndex_ += len;
        }
    }
    void Reset()
    {
        ClearIndices();
    }
    void MemMove()
    {
        const size_t length = GetLength();
        if (length == 0)
        {
            return;
        }
        std::memmove(buffer_, buffer_ + readIndex_, length);
        readIndex_ = 0;
        writeIndex_ = length;
    }

    char* GetData() { return buffer_ + readIndex_; }
    const char* GetData() const { return buffer_ + readIndex_; }
    char* GetWritePos() { return buffer_ + writeIndex_; }
    const char* GetWritePos() const { return buffer_ + writeIndex_; }
    size_t GetLength() const { return writeIndex_ - readIndex_; }
    size_t GetWriteBufferSize() const { return Size - writeIndex_; }

private:
    void ClearIndices()
    {
        readIndex_ = 0;
        writeIndex_ = 0;
    }

    char buffer_[Size];
    size_t readIndex_;
    size_t writeIndex_;
};
}
