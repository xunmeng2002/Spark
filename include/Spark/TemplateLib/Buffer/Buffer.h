#pragma once
#include <Spark/TemplateLib/ObjectPool/ObjectPool.h>
#include <cstring>
#include <assert.h>


namespace spark
{
constexpr unsigned int BuffSize = 64 * 1024;
constexpr unsigned int ShmBuffSize = 1024 * 1024;
constexpr unsigned int LogBuffSize = 1024 * 1024;

template<unsigned SIZE>
class Buffer
{
public:
	Buffer()
		:m_Buffer{0}, m_Length(0), m_ReadPos(m_Buffer)
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
		std::memcpy(m_ReadPos + m_Length, data, len);
		m_Length += len;
		return len;
	}
	char* GetData()
	{
		return m_ReadPos;
	}
	char* GetWritePos()
	{
		return m_ReadPos + m_Length;
	}
	void SetLength(unsigned len)
	{
		assert(len <= SIZE);
		m_Length = len;
	}
	unsigned GetLength()
	{
		return m_Length;
	}

	unsigned GetWriteBufferSize()
	{
		return unsigned((m_Buffer + SIZE) - (m_ReadPos + m_Length));
	}
	void Shift(unsigned len)
	{
		if (len >= m_Length)
		{
			m_ReadPos = m_Buffer;
			m_Length = 0;
		}
		else
		{
			m_ReadPos += len;
			m_Length -= len;
		}
	}
	void Reset()
	{
		m_ReadPos = m_Buffer;
		m_Length = 0;
	}
	void MemMove()
	{
		if (m_Length == 0)
			return;
		memmove(m_Buffer, m_ReadPos, m_Length);
		m_ReadPos = m_Buffer;
	}

private:
	char m_Buffer[SIZE];
	unsigned m_Length;
	char* m_ReadPos;
};
}
