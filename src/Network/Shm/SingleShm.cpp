#include "Shm/SingleShm.h"
#include <Spark/Core/Logger/Logger.h>
#include <Spark/Core/Utility/TimeUtility.h>
#include <string.h>
#include <assert.h>
#ifdef __linux__
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#endif
#ifdef _WIN32
#include <Windows.h>
#endif

using namespace std;
using namespace spark::core;

namespace spark::network
{
SingleShm::SingleShm(ServerTypeType shmType, const char* shmName)
	:IOBase(shmType, shmName, 0), m_ShmName(shmName), m_Connected(false), m_SessionID(0LL),
	m_ShmAddr(nullptr)
{
	m_ShmBuffer = new ShmBuffer<ShmBuffSize>();
#ifdef _WIN32
	m_File = nullptr;
	m_FileMap = nullptr;
#endif // _WIN32
}
SingleShm::~SingleShm()
{
	bool isLast = false;
	if (m_ShmBuffer->m_ShmHeader->Status == ConnectStatusType::DisConnected)
	{
		isLast = true;
	}
	else
	{
		m_ShmBuffer->m_ShmHeader->Status = ConnectStatusType::DisConnected;
	}
#ifdef _WIN32
	UnmapViewOfFile(m_ShmAddr);
	CloseHandle(m_FileMap);
	if (m_File != nullptr)
	{
		CloseHandle(m_File);
		m_File = nullptr;
	}
	if (isLast)
	{
		DeleteFileA(m_ShmName.c_str());
	}
#endif
#ifdef __linux__
	if (munmap(m_ShmAddr, sizeof(SingleShmHeader) + 2 * ShmBuffSize) < 0)
	{
		perror("shm_unlink");
		WriteLog(LogLevel::Warning, "munmap Failed. ErrNo:%d", errno);
	}
	if (isLast)
	{
		if (shm_unlink(m_ShmName.c_str()) < 0)
		{
			perror("shm_unlink");
			WriteLog(LogLevel::Warning, "shm_unlink Failed. ErrNo:%d", errno);
		}
	}
#endif
}
bool SingleShm::Init()
{
	bool firstOpen = true;
#ifdef _WIN32
	m_File = CreateFileA(m_ShmName.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
	if (m_File == INVALID_HANDLE_VALUE)
	{
		firstOpen = false;
		m_FileMap = OpenFileMappingA(FILE_MAP_ALL_ACCESS, FALSE, m_ShmName.c_str());
	}
	else
	{
		m_FileMap = CreateFileMappingA(m_File, NULL, PAGE_READWRITE, 0, sizeof(SingleShmHeader) + 2 * ShmBuffSize, m_ShmName.c_str());
	}
	if (m_FileMap == NULL)
	{
		WriteLog(LogLevel::Warning, "Create Or Open FileMapping Failed. ErrNo:%d", GetLastError());
		return false;
	}
	m_ShmAddr = (char*)MapViewOfFile(m_FileMap, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(SingleShmHeader) + 2 * ShmBuffSize);
	if (m_ShmAddr == NULL)
	{
		WriteLog(LogLevel::Warning, "MapViewOfFile Failed. ErrNo:%d", GetLastError());
		return false;
	}
#endif
#ifdef __linux__
	int fd = shm_open(m_ShmName.c_str(), O_CREAT | O_EXCL | O_RDWR, 0666);
	if (fd < 0)
	{
		firstOpen = false;
		fd = shm_open(m_ShmName.c_str(), O_EXCL | O_RDWR, 0666);
		if (fd < 0)
		{
			WriteLog(LogLevel::Warning, "shm_open Failed. ErrNo:%d", errno);
			return false;
		}
	}
	if (ftruncate(fd, sizeof(SingleShmHeader) + 2 * ShmBuffSize) == -1)
	{
		WriteLog(LogLevel::Warning, "ftruncate Failed. ErrNo:%d", errno);
		return false;
	}
	m_ShmAddr = (char*)mmap(nullptr, sizeof(SingleShmHeader) + 2 * ShmBuffSize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (m_ShmAddr == MAP_FAILED)
	{
		WriteLog(LogLevel::Warning, "mmap Failed. ErrNo:%d", errno);
		return false;
	}
#endif
	m_ShmBuffer->m_ShmHeader = (SingleShmHeader*)m_ShmAddr;
	m_ShmBuffer->m_ServerType = m_ServerType;
	m_ShmBuffer->m_UpBuffer = (char*)m_ShmAddr + sizeof(SingleShmHeader);
	m_ShmBuffer->m_DownBuffer = (char*)m_ShmAddr + sizeof(SingleShmHeader) + ShmBuffSize;
	if (firstOpen)
	{
		m_ShmBuffer->m_ShmHeader->Status = ConnectStatusType::UnConnected;
		m_ShmBuffer->m_ShmHeader->UpWriteCount = 0;
		m_ShmBuffer->m_ShmHeader->UpReadCount = 0;
		m_ShmBuffer->m_ShmHeader->DownWriteCount = 0;
		m_ShmBuffer->m_ShmHeader->DownReadCount = 0;
	}
	else
	{
		m_ShmBuffer->m_ShmHeader->Status = ConnectStatusType::Connected;
	}
	WriteLog(LogLevel::Info, "Create Or Open FileMapping Successed. Status:%d", m_ShmBuffer->m_ShmHeader->Status);
	return true;
}

void SingleShm::Send(SessionIDType sessionID, Buffer<BuffSize>* buffer)
{
	m_ShmBuffer->Write(buffer->GetData(), buffer->GetLength());
}
void SingleShm::DoRecv(Connect* connect)
{
	Buffer<BuffSize>* buffer = Buffer<BuffSize>::Allocate();
	auto len = m_ShmBuffer->Read(buffer->GetWritePos(), BuffSize);
	buffer->SetLength(len);

	if (m_IOSubscriber != nullptr)
		m_IOSubscriber->OnRecv(m_SessionID, buffer);
	else
		buffer->Deallocate();
}
void SingleShm::HandleIOEvent()
{
	CheckConnectStatus();
	CheckEvent();
	HandleEvent();
}

void SingleShm::CheckConnectStatus()
{
	if (!m_Connected && m_ShmBuffer->m_ShmHeader->Status == ConnectStatusType::Connected && m_IOSubscriber != nullptr)
	{
		m_Connected = true;
		m_SessionID = GetSessionID();
		m_IOSubscriber->OnConnect(m_SessionID, m_ShmName.c_str(), 0);
	}
	if (m_Connected && m_ShmBuffer->m_ShmHeader->Status == ConnectStatusType::UnConnected && m_IOSubscriber != nullptr)
	{
		m_Connected = false;
		m_IOSubscriber->OnDisConnect(m_SessionID, m_ShmName.c_str(), 0);
	}
}
void SingleShm::CheckEvent()
{

}
void SingleShm::HandleEvent()
{
	if (m_Connected)
	{
		if (m_ShmBuffer->GetReadBufferSize() > 0)
		{
			DoRecv(nullptr);
		}
	}
}
}
