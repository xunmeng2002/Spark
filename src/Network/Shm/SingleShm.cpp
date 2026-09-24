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
    : IoBase(shmType, shmName, 0), shmName_(shmName), connected_(false), sessionId_(0LL), shmAddr_(nullptr)
{
    shmBuffer_ = new ShmBuffer<ShmBufferSize>();
#ifdef _WIN32
    file_ = nullptr;
    fileMap_ = nullptr;
#endif // _WIN32
}
SingleShm::~SingleShm()
{
    const bool isLastOwner = shmBuffer_ != nullptr && shmBuffer_->MarkDisconnected();
    delete shmBuffer_;
    shmBuffer_ = nullptr;
#ifdef _WIN32
    if (shmAddr_ != nullptr)
    {
        UnmapViewOfFile(shmAddr_);
    }
    CloseHandle(fileMap_);
    if (file_ != nullptr)
    {
        CloseHandle(file_);
        file_ = nullptr;
    }
    if (isLastOwner)
    {
        DeleteFileA(shmName_.c_str());
    }
#endif
#ifdef __linux__
    if (shmAddr_ != nullptr && munmap(shmAddr_, sizeof(SingleShmHeader) + 2 * ShmBufferSize) < 0)
    {
        perror("shm_unlink");
        WriteLog(LogLevel::Warning, "munmap Failed. ErrNo:%d", errno);
    }
    if (isLastOwner)
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
    file_ = CreateFileA(shmName_.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL,
                        NULL);
    if (file_ == INVALID_HANDLE_VALUE)
    {
        firstOpen = false;
        fileMap_ = OpenFileMappingA(FILE_MAP_ALL_ACCESS, FALSE, shmName_.c_str());
    }
    else
    {
        fileMap_ = CreateFileMappingA(file_, NULL, PAGE_READWRITE, 0, sizeof(SingleShmHeader) + 2 * ShmBufferSize, shmName_.c_str());
    }
    if (fileMap_ == NULL)
    {
        WriteLog(LogLevel::Warning, "Create Or Open FileMapping Failed. ErrNo:%d", GetLastError());
        return false;
    }
    shmAddr_ = static_cast<char*>(MapViewOfFile(fileMap_, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(SingleShmHeader) + 2 * ShmBufferSize));
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
    if (ftruncate(fd, sizeof(SingleShmHeader) + 2 * ShmBufferSize) == -1)
    {
        WriteLog(LogLevel::Warning, "ftruncate Failed. ErrNo:%d", errno);
        return false;
    }
    shmAddr_ = static_cast<char*>(mmap(nullptr, sizeof(SingleShmHeader) + 2 * ShmBufferSize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0));
    if (shmAddr_ == MAP_FAILED)
    {
        WriteLog(LogLevel::Warning, "mmap Failed. ErrNo:%d", errno);
        return false;
    }
#endif
    shmBuffer_->AttachSingleConnectionSharedMemory(shmAddr_, serverType_);
    if (firstOpen)
    {
        shmBuffer_->ResetSharedHeader();
    }
    else
    {
        shmBuffer_->SetConnectStatus(ConnectStatusType::Connected);
    }
    WriteLog(LogLevel::Info, "Create Or Open FileMapping Successed. Status:%d", static_cast<int>(shmBuffer_->GetConnectStatus()));
    return true;
}

void SingleShm::Send(SessionIdType sessionId, LinearBuffer<BufferSize>* buffer)
{
    shmBuffer_->Write(buffer->GetData(), static_cast<unsigned>(buffer->GetLength()));
}
void SingleShm::DoRecv(Connect* connect)
{
    LinearBuffer<BufferSize>* buffer = LinearBuffer<BufferSize>::Allocate();
    auto len = shmBuffer_->Read(buffer->GetWritePos(), BufferSize);
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
    if (!connected_ && shmBuffer_->GetConnectStatus() == ConnectStatusType::Connected && ioSubscriber_ != nullptr)
    {
        connected_ = true;
        sessionId_ = GetSessionId();
        ioSubscriber_->OnConnect(sessionId_, shmName_.c_str(), 0);
    }
    if (connected_ && shmBuffer_->GetConnectStatus() == ConnectStatusType::UnConnected && ioSubscriber_ != nullptr)
    {
        connected_ = false;
        ioSubscriber_->OnDisConnect(sessionId_, shmName_.c_str(), 0);
    }
}
void SingleShm::CheckEvent() {}
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
