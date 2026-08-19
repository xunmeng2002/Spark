#pragma once
#include <Spark/TemplateLib/ObjectPool/ObjectPool.h>
#include <string.h>
#include <assert.h>


namespace spark
{
template<unsigned SIZE>
class RingBuffer
{
public:
	RingBuffer()
		:m_Buffer{ 0 }, m_ReadPos(m_Buffer), m_WritePos(m_Buffer), m_Length(0)
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
		return m_Length;
	}
	inline unsigned GetWriteBufferSize()
	{
		return SIZE - m_Length;
	}
	inline bool IsEmpty()
	{
		return m_Length == 0;
	}
	inline bool IsFull()
	{
		return m_Length == SIZE;
	}
	inline void Reset()
	{
		m_Length = 0;
		m_ReadPos = m_Buffer;
		m_WritePos = m_Buffer;
	}

private:
	unsigned CopyToBuffer(char* buff, unsigned len, bool consume)
	{
		if (len == 0)
			return 0;
		unsigned tailLen = (std::min)(len, unsigned((m_Buffer + SIZE) - m_ReadPos));
		if (buff != nullptr)
		{
			memcpy(buff, m_ReadPos, tailLen);
			if (tailLen < len)
			{
				memcpy(buff + tailLen, m_Buffer, size_t(len - tailLen));
			}
		}
		if (consume)
		{
			if (m_ReadPos + len < m_Buffer + SIZE)
			{
				m_ReadPos += len;
			}
			else
			{
				m_ReadPos = m_Buffer + len - tailLen;
			}
			m_Length -= len;
		}
		return len;
	}
	unsigned CopyFromBuffer(const char* data, unsigned len)
	{
		if (len == 0)
			return 0;
		unsigned tailLen = (std::min)(len, unsigned((m_Buffer + SIZE) - m_WritePos));
		memcpy(m_WritePos, data, tailLen);
		if (tailLen < len)
		{
			memcpy(m_Buffer, data + tailLen, size_t(len - tailLen));
			m_WritePos = m_Buffer + len - tailLen;
		}
		else
		{
			m_WritePos += tailLen;
		}
		m_Length += len;
		return len;
	}
private:
	char* m_ReadPos;
	char* m_WritePos;
	char m_Buffer[SIZE];
	unsigned m_Length;
};
}
