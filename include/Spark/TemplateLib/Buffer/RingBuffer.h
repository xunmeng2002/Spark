#pragma once
#include <Spark/TemplateLib/ObjectPool/ObjectPool.h>
#include <string.h>
#include <assert.h>


namespace Spark
{
template<unsigned SIZE>
class RingBuffer
{
public:
	RingBuffer()
		:buffer_{ 0 }, readPos_(buffer_), writePos_(buffer_), length_(0)
	{}
	static RingBuffer* Allocate()
	{
		return ObjectPool<RingBuffer<SIZE>>::GetInstance().Allocate();
	}
	void Deallocate()
	{
		ObjectPool<RingBuffer<SIZE>>::GetInstance().Deallocate(this);
	}
	unsigned Write(const char* data, unsigned len)
	{
		unsigned size = GetWriteBufferSize();
		len = (std::min)(len, size);
		return CopyFromBuffer(data, len);
	}
	unsigned Read(char* buff, unsigned len)
	{
		unsigned size = GetReadBufferSize();
		len = (std::min)(len, size);
		return CopyToBuffer(buff, len, true);
	}
	unsigned Peek(char* buff, unsigned len)
	{
		unsigned size = GetReadBufferSize();
		len = (std::min)(len, size);
		return CopyToBuffer(buff, len, false);
	}
	unsigned Skip(unsigned len)
	{
		unsigned size = GetReadBufferSize();
		len = (std::min)(len, size);
		return CopyToBuffer(nullptr, len, true);
	}
	inline unsigned GetReadBufferSize()
	{
		return length_;
	}
	inline unsigned GetWriteBufferSize()
	{
		return SIZE - length_;
	}
	inline bool IsEmpty()
	{
		return length_ == 0;
	}
	inline bool IsFull()
	{
		return length_ == SIZE;
	}
	inline void Reset()
	{
		length_ = 0;
		readPos_ = buffer_;
		writePos_ = buffer_;
	}

private:
	unsigned CopyToBuffer(char* buff, unsigned len, bool consume)
	{
		if (len == 0)
			return 0;
		unsigned tailLen = (std::min)(len, unsigned((buffer_ + SIZE) - readPos_));
		if (buff != nullptr)
		{
			memcpy(buff, readPos_, tailLen);
			if (tailLen < len)
			{
				memcpy(buff + tailLen, buffer_, size_t(len - tailLen));
			}
		}
		if (consume)
		{
			if (readPos_ + len < buffer_ + SIZE)
			{
				readPos_ += len;
			}
			else
			{
				readPos_ = buffer_ + len - tailLen;
			}
			length_ -= len;
		}
		return len;
	}
	unsigned CopyFromBuffer(const char* data, unsigned len)
	{
		if (len == 0)
			return 0;
		unsigned tailLen = (std::min)(len, unsigned((buffer_ + SIZE) - writePos_));
		memcpy(writePos_, data, tailLen);
		if (tailLen < len)
		{
			memcpy(buffer_, data + tailLen, size_t(len - tailLen));
			writePos_ = buffer_ + len - tailLen;
		}
		else
		{
			writePos_ += tailLen;
		}
		length_ += len;
		return len;
	}
private:
	char* readPos_;
	char* writePos_;
	char buffer_[SIZE];
	unsigned length_;
};
}
