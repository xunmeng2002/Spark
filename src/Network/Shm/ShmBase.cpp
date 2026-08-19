#include "Shm/ShmBase.h"
#include <Spark/Core/Logger/Logger.h>
#include <Spark/Core/Utility/TimeUtility.h>
#ifdef WINDOWS
#include <Windows.h>
#endif
#ifdef LINUX
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#endif

using namespace std;
using namespace spark::core;

namespace spark::network
{
ShmBase::ShmBase(ServerTypeType serverType, const char* shmName, int milliSeconds)
	:IOBase(serverType, shmName, milliSeconds), m_CommonShmHeader(nullptr), m_ShmAddr(nullptr)
{
#ifdef WINDOWS
	m_File = nullptr;
	m_FileMap = nullptr;
#endif // WINDOWS

	m_ShmName = m_Address;
	m_MaxConnectSize = atoi(m_Port.c_str());

	m_SemConnect = new Sem((m_ShmName + "SemConnect").c_str(), serverType);
	for (auto i = 0; i < m_MaxConnectSize; ++i)
	{
		auto sem = new Sem((m_ShmName + "Sem" + to_string(i)).c_str(), serverType);
		m_Sems.push_back(sem);
	}
}
ShmBase::~ShmBase()
{
	if (m_SemConnect != nullptr)
		delete m_SemConnect;
	m_SemConnect = nullptr;
	for (auto sem : m_Sems)
	{
		delete sem;
	}
	m_Sems.clear();
#ifdef WINDOWS
	UnmapViewOfFile(m_ShmAddr);
	CloseHandle(m_FileMap);
	if (m_File != nullptr)
	{
		CloseHandle(m_File);
		m_File = nullptr;
	}
	if (m_ServerType == ServerTypeType::Server)
	{
		DeleteFileA(m_ShmName.c_str());
	}
#endif
#ifdef LINUX
	if (munmap(m_ShmAddr, ShmBuffSize * m_MaxConnectSize * 2) < 0)
	{
		perror("shm_unlink");
		WriteLog(LogLevel::Warning, "munmap Failed. ErrNo:%d", errno);
	}
	if (m_ServerType == ServerTypeType::Server)
	{
		if (shm_unlink(m_ShmName.c_str()) < 0)
		{
			perror("shm_unlink");
			WriteLog(LogLevel::Warning, "shm_unlink Failed. ErrNo:%d", errno);
		}
	}
#endif
}
bool ShmBase::Init()
{
	if (!m_SemConnect->Init())
		return false;
	for (auto i = 0; i < m_MaxConnectSize; ++i)
	{
		if (!m_Sems[i]->Init())
			return false;
	}
#ifdef WINDOWS
	if (!WindowsInit())
		return false;
#endif
#ifdef LINUX
	if (!LinuxInit())
		return false;
#endif
	m_CommonShmHeader = (SingleShmHeader*)m_ShmAddr;
	if (m_ServerType == ServerTypeType::Server)
	{
		memset(m_ShmAddr, 0, ShmBuffSize * m_MaxConnectSize * 2);
		m_CommonShmHeader->Status = ConnectStatusType::UnConnected;
		for (auto i = 1u; i < m_MaxConnectSize; ++i)
		{
			auto shmHeader = m_CommonShmHeader + i;
			shmHeader->Status = ConnectStatusType::UnConnected;
		}
	}
	WriteLog(LogLevel::Info, "Create Or Open Shm Successed.");
	return true;
}

void ShmBase::Send(SessionIDType sessionID, Buffer<BuffSize>* buffer)
{
	auto shmConnect = (ShmConnect<ShmBuffSize>*)GetConnect(sessionID);
	if (shmConnect == nullptr)
	{
		buffer->Deallocate();
		return;
	}
	while (buffer->GetLength() > 0)
	{
		auto len = shmConnect->m_ShmBuffer->Write(buffer->GetData(), buffer->GetLength());
		if (len > 0)
		{
			buffer->Shift(len);
			if (m_ServerType == ServerTypeType::Server)
			{
				m_Sems[shmConnect->RemotePort]->UnLock();
			}
			else
			{
				m_Sems[0]->UnLock();
			}
		}
	}
	buffer->Deallocate();
}

void ShmBase::HandleIOEvent()
{
	if (m_ServerType == ServerTypeType::Client)
	{
		ConnectToServer();
	}
	else
	{
		Accept();
	}
	CheckConnect();
	DoDisConnect();
	CheckData();
	HandleData();
}
void ShmBase::DoSend(Connect* connect)
{
	auto shmConnect = (ShmConnect<ShmBuffSize>*)connect;
	auto buffer = connect->GetNextBuffer();
	while (buffer != nullptr)
	{
		int len = shmConnect->m_ShmBuffer->Write(buffer->GetData(), buffer->GetLength());
		buffer->Shift(len);
		if (buffer->GetLength() == 0)
		{
			buffer->Deallocate();
			buffer = connect->GetNextBuffer();
		}
		else
		{
			connect->PushFront(buffer);
			break;
		}
	}
}
void ShmBase::DoRecv(Connect* connect)
{
	auto shmConnect = (ShmConnect<ShmBuffSize>*)connect;

	Buffer<BuffSize>* buffer = Buffer<BuffSize>::Allocate();
	auto len = shmConnect->m_ShmBuffer->Read(buffer->GetWritePos(), BuffSize);
	buffer->SetLength(len);
	if (m_IOSubscriber != nullptr)
		m_IOSubscriber->OnRecv(shmConnect->SessionID, buffer);
	else
		buffer->Deallocate();
}


bool ShmBase::WindowsInit()
{
#ifdef WINDOWS
	if (m_ServerType == ServerTypeType::Server)
	{
		m_File = CreateFileA(m_ShmName.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
		if (m_File == INVALID_HANDLE_VALUE)
		{
			WriteLog(LogLevel::Warning, "CreateFileA Failed. ErrNo:%d", GetLastError());
			return false;
		}
		m_FileMap = CreateFileMappingA(m_File, NULL, PAGE_READWRITE, 0, ShmBuffSize * m_MaxConnectSize * 2, m_ShmName.c_str());
	}
	else
	{
		m_FileMap = OpenFileMappingA(FILE_MAP_ALL_ACCESS, FALSE, m_ShmName.c_str());
	}
	if (m_FileMap == NULL)
	{
		WriteLog(LogLevel::Warning, "Create Or Open FileMapping Failed. ErrNo:%d", GetLastError());
		return false;
	}
	m_ShmAddr = (char*)MapViewOfFile(m_FileMap, FILE_MAP_ALL_ACCESS, 0, 0, ShmBuffSize * m_MaxConnectSize * 2);
	if (m_ShmAddr == NULL)
	{
		WriteLog(LogLevel::Warning, "MapViewOfFile Failed. ErrNo:%d", GetLastError());
		return false;
	}
#endif
	return true;
}
bool ShmBase::LinuxInit()
{
#ifdef LINUX
	int fd;
	if (m_ServerType == ServerTypeType::Server)
	{
		fd = shm_open(m_ShmName.c_str(), O_CREAT | O_EXCL | O_RDWR, 0666);
	}
	else
	{
		fd = shm_open(m_ShmName.c_str(), O_RDWR, 0666);
	}
	if (fd < 0)
	{
		WriteLog(LogLevel::Warning, "shm_open Failed. ErrNo:%d", errno);
		return false;
	}
	if (ftruncate(fd, ShmBuffSize * m_MaxConnectSize * 2) == -1)
	{
		WriteLog(LogLevel::Warning, "ftruncate Failed. ErrNo:%d", errno);
		return false;
	}
	m_ShmAddr = (char*)mmap(nullptr, ShmBuffSize * m_MaxConnectSize * 2, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (m_ShmAddr == MAP_FAILED)
	{
		WriteLog(LogLevel::Warning, "mmap Failed. ErrNo:%d", errno);
		return false;
	}
#endif
	return true;
}
}

