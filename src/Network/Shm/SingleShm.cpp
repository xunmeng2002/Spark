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
using namespace Spark::Core;

namespace Spark::Network
{
SingleShm::SingleShm(ServerTypeType shmType, const char* shmName)
	:IoBase(shmType, shmName, 0), shmName_(shmName), connected_(false), sessionId_(0LL),
	shmAddr_(nullptr)
{
    shmBuffer_ = new ShmBuffer<ShmBuffSize>();
#ifdef _WIN32
	file_ = nullptr;
	fileMap_ = nullptr;
#endif // _WIN32
}
SingleShm::~SingleShm()
{
	bool isLast = false;
	if (shmBuffer_->ShmHeader->Status == ConnectStatusType::DisConnected)
	{
		isLast = true;
	}
	else
	{
        shmBuffer_->ShmHeader->Status = ConnectStatusType::DisConnected;
	}
#ifdef _WIN32
	UnmapViewOfFile(shmAddr_);
	CloseHandle(fileMap_);
	if (file_ != nullptr)
	{
		CloseHandle(file_);
		file_ = nullptr;
	}
	if (isLast)
	{
		DeleteFileA(shmName_.c_str());
	}
#endif
#ifdef __linux__
	if (munmap(shmAddr_, sizeof(SingleShmHeader) + 2 * ShmBuffSize) < 0)
	{
		perror("shm_unlink");
		WriteLog(LogLevel::Warning, "munmap Failed. ErrNo:%d", errno);
	}
	if (isLast)
	{
		if (shm_unlink(shmName_.c_str()) < 0)
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
	file_ = CreateFileA(shmName_.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
	if (file_ == INVALID_HANDLE_VALUE)
	{
		firstOpen = false;
		fileMap_ = OpenFileMappingA(FILE_MAP_ALL_ACCESS, FALSE, shmName_.c_str());
	}
	else
	{
		fileMap_ = CreateFileMappingA(file_, NULL, PAGE_READWRITE, 0, sizeof(SingleShmHeader) + 2 * ShmBuffSize, shmName_.c_str());
	}
	if (fileMap_ == NULL)
	{
		WriteLog(LogLevel::Warning, "Create Or Open FileMapping Failed. ErrNo:%d", GetLastError());
		return false;
	}
	shmAddr_ = (char*)MapViewOfFile(fileMap_, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(SingleShmHeader) + 2 * ShmBuffSize);
	if (shmAddr_ == NULL)
	{
		WriteLog(LogLevel::Warning, "MapViewOfFile Failed. ErrNo:%d", GetLastError());
		return false;
	}
#endif
#ifdef __linux__
	int fd = shm_open(shmName_.c_str(), O_CREAT | O_EXCL | O_RDWR, 0666);
	if (fd < 0)
	{
		firstOpen = false;
		fd = shm_open(shmName_.c_str(), O_EXCL | O_RDWR, 0666);
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
	shmAddr_ = (char*)mmap(nullptr, sizeof(SingleShmHeader) + 2 * ShmBuffSize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (shmAddr_ == MAP_FAILED)
	{
		WriteLog(LogLevel::Warning, "mmap Failed. ErrNo:%d", errno);
		return false;
	}
#endif
	shmBuffer_->ShmHeader = (SingleShmHeader*)shmAddr_;
	shmBuffer_->ServerType = serverType_;
	shmBuffer_->UpBuffer = (char*)shmAddr_ + sizeof(SingleShmHeader);
	shmBuffer_->DownBuffer = (char*)shmAddr_ + sizeof(SingleShmHeader) + ShmBuffSize;
	if (firstOpen)
	{
		shmBuffer_->ShmHeader->Status = ConnectStatusType::UnConnected;
		shmBuffer_->ShmHeader->UpWriteCount = 0;
		shmBuffer_->ShmHeader->UpReadCount = 0;
		shmBuffer_->ShmHeader->DownWriteCount = 0;
		shmBuffer_->ShmHeader->DownReadCount = 0;
	}
	else
	{
		shmBuffer_->ShmHeader->Status = ConnectStatusType::Connected;
	}
	WriteLog(LogLevel::Info, "Create Or Open FileMapping Successed. Status:%d", shmBuffer_->ShmHeader->Status);
	return true;
}

void SingleShm::Send(SessionIdType sessionId, Buffer<BuffSize>* buffer)
{
	shmBuffer_->Write(buffer->GetData(), buffer->GetLength());
}
void SingleShm::DoRecv(Connect* connect)
{
	Buffer<BuffSize>* buffer = Buffer<BuffSize>::Allocate();
	auto len = shmBuffer_->Read(buffer->GetWritePos(), BuffSize);
	buffer->SetLength(len);

	if (ioSubscriber_ != nullptr)
		ioSubscriber_->OnRecv(sessionId_, buffer);
	else
		buffer->Deallocate();
}
void SingleShm::HandleIoEvent()
{
	CheckConnectStatus();
	CheckEvent();
	HandleEvent();
}

void SingleShm::CheckConnectStatus()
{
	if (!connected_ && shmBuffer_->ShmHeader->Status == ConnectStatusType::Connected && ioSubscriber_ != nullptr)
	{
		connected_ = true;
		sessionId_ = GetSessionId();
		ioSubscriber_->OnConnect(sessionId_, shmName_.c_str(), 0);
	}
	if (connected_ && shmBuffer_->ShmHeader->Status == ConnectStatusType::UnConnected && ioSubscriber_ != nullptr)
	{
		connected_ = false;
		ioSubscriber_->OnDisConnect(sessionId_, shmName_.c_str(), 0);
	}
}
void SingleShm::CheckEvent()
{

}
void SingleShm::HandleEvent()
{
	if (connected_)
	{
		if (shmBuffer_->GetReadBufferSize() > 0)
		{
			DoRecv(nullptr);
		}
	}
}
}
