#pragma once
#include <Spark/Types.h>
#include <Spark/TemplateLib/ObjectPool/ObjectPool.h>
#include <algorithm>
#include <atomic>
#include <cstring>

namespace spark
{
struct SingleShmHeader
{
	volatile ConnectStatusType Status;
	volatile unsigned UpWriteCount;
	volatile unsigned UpReadCount;
	volatile unsigned DownWriteCount;
	volatile unsigned DownReadCount;
};


template<unsigned SIZE>
class ShmBuffer
{
public:
	SingleShmHeader* m_ShmHeader;
	ServerTypeType m_ServerType;
	int m_Index;
	char* m_UpBuffer;
	char* m_DownBuffer;

	ShmBuffer()
	{
		m_ShmHeader = nullptr;
		m_UpBuffer = nullptr;
		m_DownBuffer = nullptr;
	}
	ShmBuffer(ServerTypeType serverType, int index, void* shmAddr, ConnectStatusType connectStatus)
	{
		m_ServerType = serverType;
		m_Index = index;
		m_ShmHeader = (SingleShmHeader*)shmAddr + index;
		m_ShmHeader->Status = connectStatus;
		m_UpBuffer = (char*)shmAddr + SIZE * index * 2;
		m_DownBuffer = (char*)shmAddr + SIZE * (index * 2 + 1);
	}
	~ShmBuffer()
	{
		m_ShmHeader = nullptr;
		m_UpBuffer = nullptr;
		m_DownBuffer = nullptr;
	}
	static ShmBuffer* Allocate(ServerTypeType serverType, int index, void* shmAddr, ConnectStatusType connectStatus)
	{
		return ObjectPool<ShmBuffer<SIZE>>::GetInstance().Allocate(serverType, index, shmAddr, connectStatus);
	}
	void Deallocate()
	{
		if (m_ShmHeader->Status != ConnectStatusType::DisConnected)
		{
			m_ShmHeader->Status = ConnectStatusType::DisConnected;
		}
		else
		{
			m_ShmHeader->Status = ConnectStatusType::UnConnected;
			m_ShmHeader->UpWriteCount = 0;
			m_ShmHeader->UpReadCount = 0;
			m_ShmHeader->DownWriteCount = 0;
			m_ShmHeader->DownReadCount = 0;
		}
		ObjectPool<ShmBuffer<SIZE>>::GetInstance().Deallocate(this);
	}

	unsigned Write(const char* data, unsigned len)
	{
		if (m_ServerType == ServerTypeType::Client)
			return UpWrite(data, len);
		return DownWrite(data, len);
	}
	unsigned Read(char* buff, unsigned len)
	{
		if (m_ServerType == ServerTypeType::Client)
			return DownRead(buff, len);
		return UpRead(buff, len);
	}

	unsigned GetWriteBufferSize()
	{
		if (m_ServerType == ServerTypeType::Client)
			return GetUpWriteBufferSize();
		return GetDownWriteBufferSize();
	}
	unsigned GetReadBufferSize()
	{
		if (m_ServerType == ServerTypeType::Client)
			return GetDownReadBufferSize();
		return GetUpReadBufferSize();
	}

private:
	unsigned GetUpWriteBufferSize()
	{
		if (m_ShmHeader->UpReadCount > m_ShmHeader->UpWriteCount)
		{
			return m_ShmHeader->UpReadCount - m_ShmHeader->UpWriteCount - 1;
		}
		return SIZE - (m_ShmHeader->UpWriteCount - m_ShmHeader->UpReadCount) - 1;
	}
	unsigned GetUpReadBufferSize()
	{
		if (m_ShmHeader->UpWriteCount >= m_ShmHeader->UpReadCount)
		{
			return m_ShmHeader->UpWriteCount - m_ShmHeader->UpReadCount;
		}
		return SIZE - (m_ShmHeader->UpReadCount - m_ShmHeader->UpWriteCount);
	}
	unsigned GetDownWriteBufferSize()
	{
		if (m_ShmHeader->DownReadCount > m_ShmHeader->DownWriteCount)
		{
			return m_ShmHeader->DownReadCount - m_ShmHeader->DownWriteCount - 1;
		}
		return SIZE - (m_ShmHeader->DownWriteCount - m_ShmHeader->DownReadCount) - 1;
	}
	unsigned GetDownReadBufferSize()
	{
		if (m_ShmHeader->DownWriteCount >= m_ShmHeader->DownReadCount)
		{
			return m_ShmHeader->DownWriteCount - m_ShmHeader->DownReadCount;
		}
		return SIZE - (m_ShmHeader->DownReadCount - m_ShmHeader->DownWriteCount);
	}

	unsigned UpWrite(const char* data, unsigned len)
	{
		if (m_ShmHeader->Status != ConnectStatusType::Connected)
			return 0;
		auto size = GetUpWriteBufferSize();
		unsigned int currLen = std::min<unsigned>(len, size);
		if (currLen == 0)
			return 0;
		unsigned int tailLen = std::min<unsigned>(currLen, SIZE - m_ShmHeader->UpWriteCount);
		memcpy(m_UpBuffer + m_ShmHeader->UpWriteCount, data, tailLen);
		if (tailLen < currLen)
		{
			memcpy(m_UpBuffer, data + tailLen, size_t(currLen - tailLen));
			std::atomic_thread_fence(std::memory_order_release);
			m_ShmHeader->UpWriteCount = currLen - tailLen;
		}
		else
		{
			std::atomic_thread_fence(std::memory_order_release);
			m_ShmHeader->UpWriteCount += currLen;
		}
		return currLen;
	}
	unsigned UpRead(char* buff, unsigned len)
	{
		if (m_ShmHeader->Status != ConnectStatusType::Connected)
			return 0;
		auto size = GetUpReadBufferSize();
		auto currLen = std::min<unsigned>(len, size);
		if (currLen == 0)
			return 0;
		auto tailLen = std::min<unsigned>(currLen, SIZE - m_ShmHeader->UpReadCount);
		memcpy(buff, m_UpBuffer + m_ShmHeader->UpReadCount, tailLen);
		if (tailLen < currLen)
		{
			memcpy(buff + tailLen, m_UpBuffer, currLen - tailLen);
			std::atomic_thread_fence(std::memory_order_release);
			m_ShmHeader->UpReadCount = currLen - tailLen;
		}
		else
		{
			std::atomic_thread_fence(std::memory_order_release);
			m_ShmHeader->UpReadCount += currLen;
		}
		return currLen;
	}
	unsigned DownWrite(const char* data, unsigned len)
	{
		if (m_ShmHeader->Status != ConnectStatusType::Connected)
			return 0;
		auto size = GetDownWriteBufferSize();
		unsigned int currLen = std::min<unsigned>(len, size);
		if (currLen == 0)
			return 0;
		unsigned int tailLen = std::min<unsigned>(currLen, SIZE - m_ShmHeader->DownWriteCount);
		memcpy(m_DownBuffer + m_ShmHeader->DownWriteCount, data, tailLen);
		if (tailLen < currLen)
		{
			memcpy(m_DownBuffer, data + tailLen, size_t(currLen - tailLen));
			std::atomic_thread_fence(std::memory_order_release);
			m_ShmHeader->DownWriteCount = currLen - tailLen;
		}
		else
		{
			std::atomic_thread_fence(std::memory_order_release);
			m_ShmHeader->DownWriteCount += currLen;
		}
		return currLen;
	}
	unsigned DownRead(char* buff, unsigned len)
	{
		if (m_ShmHeader->Status != ConnectStatusType::Connected)
			return 0;
		auto size = GetDownReadBufferSize();
		auto currLen = std::min<unsigned>(len, size);
		if (currLen == 0)
			return 0;
		auto tailLen = std::min<unsigned>(currLen, SIZE - m_ShmHeader->DownReadCount);
		memcpy(buff, m_DownBuffer + m_ShmHeader->DownReadCount, tailLen);
		if (tailLen < currLen)
		{
			memcpy(buff + tailLen, m_DownBuffer, currLen - tailLen);
			std::atomic_thread_fence(std::memory_order_release);
			m_ShmHeader->DownReadCount = currLen - tailLen;
		}
		else
		{
			std::atomic_thread_fence(std::memory_order_release);
			m_ShmHeader->DownReadCount += currLen;
		}
		return currLen;
	}
};
}

