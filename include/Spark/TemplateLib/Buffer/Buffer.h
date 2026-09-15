#pragma once
#include <Spark/TemplateLib/ObjectPool/ObjectPool.h>
#include <cstring>
#include <assert.h>


namespace Spark
{
constexpr unsigned int BuffSize = 64 * 1024;
constexpr unsigned int ShmBuffSize = 1024 * 1024;
constexpr unsigned int LogBuffSize = 1024 * 1024;

template<unsigned SIZE>
class Buffer
{
public:
	Buffer()
		:buffer_{0}, length_(0), readPos_(buffer_)
	{}
	static Buffer* Allocate()
	{
		return ObjectPool<Buffer<SIZE>>::GetInstance().Allocate();
	}
	void Deallocate()
	{
		ObjectPool<Buffer<SIZE>>::GetInstance().Deallocate(this);
	}
	unsigned Append(const char* data, unsigned len)
	{
		auto size = GetWriteBufferSize();
		len = len > size ? size : len;
		std::memcpy(readPos_ + length_, data, len);
		length_ += len;
		return len;
	}
	char* GetData()
	{
		return readPos_;
	}
	char* GetWritePos()
	{
		return readPos_ + length_;
	}
	void SetLength(unsigned len)
	{
		assert(len <= SIZE);
		length_ = len;
	}
	unsigned GetLength()
	{
		return length_;
	}

	unsigned GetWriteBufferSize()
	{
		return unsigned((buffer_ + SIZE) - (readPos_ + length_));
	}
	void Shift(unsigned len)
	{
		if (len >= length_)
		{
			readPos_ = buffer_;
			length_ = 0;
		}
		else
		{
			readPos_ += len;
			length_ -= len;
		}
	}
	void Reset()
	{
		readPos_ = buffer_;
		length_ = 0;
	}
	void MemMove()
	{
		if (length_ == 0)
			return;
		memmove(buffer_, readPos_, length_);
		readPos_ = buffer_;
	}

private:
	char buffer_[SIZE];
	unsigned length_;
	char* readPos_;
};
}
