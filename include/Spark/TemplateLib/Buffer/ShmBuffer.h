#pragma once
#include <Spark/Types.h>
#include <Spark/TemplateLib/ObjectPool/ObjectPool.h>
#include <algorithm>
#include <atomic>
#include <cstring>

namespace Spark
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
	ShmBuffer()
	{
		ShmHeader = nullptr;
		UpBuffer = nullptr;
		DownBuffer = nullptr;
	}
	ShmBuffer(ServerTypeType serverType, int index, void* shmAddr, ConnectStatusType connectStatus)
	{
		ServerType = serverType;
		Index = index;
		ShmHeader = (SingleShmHeader*)shmAddr + index;
		ShmHeader->Status = connectStatus;
		UpBuffer = (char*)shmAddr + SIZE * index * 2;
		DownBuffer = (char*)shmAddr + SIZE * (index * 2 + 1);
	}
	~ShmBuffer()
	{
		ShmHeader = nullptr;
		UpBuffer = nullptr;
		DownBuffer = nullptr;
	}
	static ShmBuffer* Allocate(ServerTypeType serverType, int index, void* shmAddr, ConnectStatusType connectStatus)
	{
		return ObjectPool<ShmBuffer<SIZE>>::GetInstance().Allocate(serverType, index, shmAddr, connectStatus);
	}
	void Deallocate()
	{
		if (ShmHeader->Status != ConnectStatusType::DisConnected)
		{
			ShmHeader->Status = ConnectStatusType::DisConnected;
		}
		else
		{
			ShmHeader->Status = ConnectStatusType::UnConnected;
			ShmHeader->UpWriteCount = 0;
			ShmHeader->UpReadCount = 0;
			ShmHeader->DownWriteCount = 0;
			ShmHeader->DownReadCount = 0;
		}
		ObjectPool<ShmBuffer<SIZE>>::GetInstance().Deallocate(this);
	}

	unsigned Write(const char* data, unsigned len)
	{
		if (ServerType == ServerTypeType::Client)
			return UpWrite(data, len);
		return DownWrite(data, len);
	}
	unsigned Read(char* buff, unsigned len)
	{
		if (ServerType == ServerTypeType::Client)
			return DownRead(buff, len);
		return UpRead(buff, len);
	}

	unsigned GetWriteBufferSize()
	{
		if (ServerType == ServerTypeType::Client)
			return GetUpWriteBufferSize();
		return GetDownWriteBufferSize();
	}
	unsigned GetReadBufferSize()
	{
		if (ServerType == ServerTypeType::Client)
			return GetDownReadBufferSize();
		return GetUpReadBufferSize();
	}

	SingleShmHeader* ShmHeader;
	ServerTypeType ServerType;
	int Index;
	char* UpBuffer;
	char* DownBuffer;

private:
	unsigned GetUpWriteBufferSize()
	{
		if (ShmHeader->UpReadCount > ShmHeader->UpWriteCount)
		{
			return ShmHeader->UpReadCount - ShmHeader->UpWriteCount - 1;
		}
		return SIZE - (ShmHeader->UpWriteCount - ShmHeader->UpReadCount) - 1;
	}
	unsigned GetUpReadBufferSize()
	{
		if (ShmHeader->UpWriteCount >= ShmHeader->UpReadCount)
		{
			return ShmHeader->UpWriteCount - ShmHeader->UpReadCount;
		}
		return SIZE - (ShmHeader->UpReadCount - ShmHeader->UpWriteCount);
	}
	unsigned GetDownWriteBufferSize()
	{
		if (ShmHeader->DownReadCount > ShmHeader->DownWriteCount)
		{
			return ShmHeader->DownReadCount - ShmHeader->DownWriteCount - 1;
		}
		return SIZE - (ShmHeader->DownWriteCount - ShmHeader->DownReadCount) - 1;
	}
	unsigned GetDownReadBufferSize()
	{
		if (ShmHeader->DownWriteCount >= ShmHeader->DownReadCount)
		{
			return ShmHeader->DownWriteCount - ShmHeader->DownReadCount;
		}
		return SIZE - (ShmHeader->DownReadCount - ShmHeader->DownWriteCount);
	}

	unsigned UpWrite(const char* data, unsigned len)
	{
		if (ShmHeader->Status != ConnectStatusType::Connected)
			return 0;
		auto size = GetUpWriteBufferSize();
		unsigned int currLen = std::min<unsigned>(len, size);
		if (currLen == 0)
			return 0;
		unsigned int tailLen = std::min<unsigned>(currLen, SIZE - ShmHeader->UpWriteCount);
		memcpy(UpBuffer + ShmHeader->UpWriteCount, data, tailLen);
		if (tailLen < currLen)
		{
			memcpy(UpBuffer, data + tailLen, size_t(currLen - tailLen));
			std::atomic_thread_fence(std::memory_order_release);
			ShmHeader->UpWriteCount = currLen - tailLen;
		}
		else
		{
			std::atomic_thread_fence(std::memory_order_release);
			ShmHeader->UpWriteCount += currLen;
		}
		return currLen;
	}
	unsigned UpRead(char* buff, unsigned len)
	{
		if (ShmHeader->Status != ConnectStatusType::Connected)
			return 0;
		auto size = GetUpReadBufferSize();
		auto currLen = std::min<unsigned>(len, size);
		if (currLen == 0)
			return 0;
		auto tailLen = std::min<unsigned>(currLen, SIZE - ShmHeader->UpReadCount);
		memcpy(buff, UpBuffer + ShmHeader->UpReadCount, tailLen);
		if (tailLen < currLen)
		{
			memcpy(buff + tailLen, UpBuffer, currLen - tailLen);
			std::atomic_thread_fence(std::memory_order_release);
			ShmHeader->UpReadCount = currLen - tailLen;
		}
		else
		{
			std::atomic_thread_fence(std::memory_order_release);
			ShmHeader->UpReadCount += currLen;
		}
		return currLen;
	}
	unsigned DownWrite(const char* data, unsigned len)
	{
		if (ShmHeader->Status != ConnectStatusType::Connected)
			return 0;
		auto size = GetDownWriteBufferSize();
		unsigned int currLen = std::min<unsigned>(len, size);
		if (currLen == 0)
			return 0;
		unsigned int tailLen = std::min<unsigned>(currLen, SIZE - ShmHeader->DownWriteCount);
		memcpy(DownBuffer + ShmHeader->DownWriteCount, data, tailLen);
		if (tailLen < currLen)
		{
			memcpy(DownBuffer, data + tailLen, size_t(currLen - tailLen));
			std::atomic_thread_fence(std::memory_order_release);
			ShmHeader->DownWriteCount = currLen - tailLen;
		}
		else
		{
			std::atomic_thread_fence(std::memory_order_release);
			ShmHeader->DownWriteCount += currLen;
		}
		return currLen;
	}
	unsigned DownRead(char* buff, unsigned len)
	{
		if (ShmHeader->Status != ConnectStatusType::Connected)
			return 0;
		auto size = GetDownReadBufferSize();
		auto currLen = std::min<unsigned>(len, size);
		if (currLen == 0)
			return 0;
		auto tailLen = std::min<unsigned>(currLen, SIZE - ShmHeader->DownReadCount);
		memcpy(buff, DownBuffer + ShmHeader->DownReadCount, tailLen);
		if (tailLen < currLen)
		{
			memcpy(buff + tailLen, DownBuffer, currLen - tailLen);
			std::atomic_thread_fence(std::memory_order_release);
			ShmHeader->DownReadCount = currLen - tailLen;
		}
		else
		{
			std::atomic_thread_fence(std::memory_order_release);
			ShmHeader->DownReadCount += currLen;
		}
		return currLen;
	}
};
}

