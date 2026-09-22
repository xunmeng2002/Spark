#pragma once
#include <Spark/TemplateLib/ObjectPool/ObjectPool.h>

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstring>

namespace Spark
{
// 线性字节缓冲：自身不提供任何并发保护。跨线程传递必须由调用方保证所有权转移——
// 交出后即不得再访问该对象（含 Reset），否则会与池中重新分配出去的同名对象混叠。
template <size_t SIZE>
class Buffer
{
    static_assert(SIZE > 0, "SIZE must be greater than 0");

public:
    Buffer() : buffer_{0} { ClearIndices(); }
    Buffer(const Buffer&) = delete;
    Buffer& operator=(const Buffer&) = delete;
    Buffer(Buffer&&) = delete;
    Buffer& operator=(Buffer&&) = delete;

    static Buffer* Allocate() { return ObjectPool<Buffer<SIZE>>::GetInstance().Allocate(); }
    void Deallocate() { ObjectPool<Buffer<SIZE>>::GetInstance().Deallocate(this); }

    size_t Append(const char* data, size_t len)
    {
        len = (std::min)(len, GetWriteBufferSize());
        if (len > 0)
        {
            std::memcpy(buffer_ + writeIndex_, data, len);
            writeIndex_ += len;
        }
        return len;
    }
    // 登记从读位置起的有效字节数（不是写入增量），超出容量时钳到缓冲末尾；
    // 调用方传入的有符号负值会先被转成极大值，再被钳成整段容量，故 len 必须来自可信的非负长度。
    void SetLength(size_t len)
    {
        const size_t capacity = SIZE - readIndex_;
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
    // 仅在缓冲未参与任何在途读写时调用：复位会改变 GetData() 的取值。
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
    size_t GetWriteBufferSize() const { return SIZE - writeIndex_; }

private:
    void ClearIndices()
    {
        readIndex_ = 0;
        writeIndex_ = 0;
    }

    char buffer_[SIZE];
    size_t readIndex_;
    size_t writeIndex_;
};
}
