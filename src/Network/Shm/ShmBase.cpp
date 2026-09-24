#include "Shm/ShmBase.h"
#include <Spark/Core/Logger/Logger.h>
#include <Spark/Core/Utility/TimeUtility.h>
#include <Spark/Network/Protocol/StepUtility.h>
#ifdef _WIN32
#include <Windows.h>
#endif
#ifdef __linux__
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#endif
#include <chrono>
#include <cstddef>
#include <thread>

using namespace std;
using namespace Spark::Core;

namespace Spark::Network
{
ShmBase::ShmBase(ServerTypeType serverType, const char* shmName, int milliSeconds)
    : IoBase(serverType, shmName, milliSeconds), commonShmHeader_(nullptr), shmAddr_(nullptr)
{
#ifdef _WIN32
    file_ = nullptr;
    fileMap_ = nullptr;
#endif // _WIN32

    shmName_ = address_;
    if (!StepUtility::ParseInteger(port_, maxConnectSize_))
    {
        maxConnectSize_ = 0;
    }

    semConnect_ = new Sem((shmName_ + "SemConnect").c_str(), serverType);
    for (auto i = 0u; i < maxConnectSize_; ++i)
    {
        auto sem = new Sem((shmName_ + "Sem" + to_string(i)).c_str(), serverType);
        sems_.push_back(sem);
    }
}
ShmBase::~ShmBase()
{
    if (semConnect_ != nullptr)
        delete semConnect_;
    semConnect_ = nullptr;
    for (auto sem : sems_)
    {
        delete sem;
    }
    sems_.clear();
#ifdef _WIN32
    UnmapViewOfFile(shmAddr_);
    CloseHandle(fileMap_);
    if (file_ != nullptr)
    {
        CloseHandle(file_);
        file_ = nullptr;
    }
    if (serverType_ == ServerTypeType::Server)
    {
        DeleteFileA(shmName_.c_str());
    }
#endif
#ifdef __linux__
    if (munmap(shmAddr_, GetSharedMemoryMappingSize()) < 0)
    {
        perror("shm_unlink");
        WriteLog(LogLevel::Warning, "munmap Failed. ErrNo:%d", errno);
    }
    if (serverType_ == ServerTypeType::Server)
    {
        if (shm_unlink(shmName_.c_str()) < 0)
        {
            perror("shm_unlink");
            WriteLog(LogLevel::Warning, "shm_unlink Failed. ErrNo:%d", errno);
        }
    }
#endif
}
unsigned ShmBase::GetSharedMemoryMappingSize() const
{
    return ShmBufferSize * maxConnectSize_ * 2;
}
bool ShmBase::Init()
{
    if (maxConnectSize_ < 1)
    {
        WriteLog(LogLevel::Warning, "Invalid Shm ConnectSize:%u, Address:%s", maxConnectSize_, shmName_.c_str());
        return false;
    }
    if (!semConnect_->Init())
        return false;
    for (auto i = 0u; i < maxConnectSize_; ++i)
    {
        if (!sems_[i]->Init())
            return false;
    }
#ifdef _WIN32
    if (!WindowsInit())
        return false;
#endif
#ifdef __linux__
    if (!LinuxInit())
        return false;
#endif
    commonShmHeader_ = static_cast<SingleShmHeader*>(shmAddr_);
    if (serverType_ == ServerTypeType::Server)
    {
        memset(shmAddr_, 0, GetSharedMemoryMappingSize());
        commonShmHeader_->Status = ConnectStatusType::UnConnected;
        for (auto i = 1u; i < maxConnectSize_; ++i)
        {
            auto shmHeader = commonShmHeader_ + i;
            shmHeader->Status = ConnectStatusType::UnConnected;
        }
    }
    WriteLog(LogLevel::Info, "Create Or Open Shm Successed.");
    return true;
}

void ShmBase::Send(SessionIdType sessionId, LinearBuffer<BufferSize>* buffer)
{
    auto shmConnect = static_cast<ShmConnect<ShmBufferSize>*>(GetConnect(sessionId));
    if (shmConnect == nullptr)
    {
        buffer->Deallocate();
        return;
    }
    while (buffer->GetLength() > 0)
    {
        auto len = shmConnect->GetBuffer()->Write(buffer->GetData(), buffer->GetLength());
        if (len == 0)
        {
            if (shmConnect->GetBuffer()->GetConnectStatus() != ConnectStatusType::Connected)
            {
                WriteLog(LogLevel::Warning, "Send Peer DisConnected, Drop Buffer. SessionId:%lld, Len:%zu", sessionId, buffer->GetLength());
                break;
            }
            this_thread::sleep_for(chrono::milliseconds(1));
            continue;
        }
        buffer->Shift(len);
        if (serverType_ == ServerTypeType::Server)
        {
            sems_[shmConnect->RemotePort]->UnLock();
        }
        else
        {
            sems_[0]->UnLock();
        }
    }
    buffer->Deallocate();
}

void ShmBase::HandleIoEvent()
{
    if (serverType_ == ServerTypeType::Client)
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
    auto shmConnect = static_cast<ShmConnect<ShmBufferSize>*>(connect);
    auto buffer = connect->GetNextBuffer();
    while (buffer != nullptr)
    {
        size_t len = shmConnect->GetBuffer()->Write(buffer->GetData(), buffer->GetLength());
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
    auto shmConnect = static_cast<ShmConnect<ShmBufferSize>*>(connect);

    LinearBuffer<BufferSize>* buffer = LinearBuffer<BufferSize>::Allocate();
    auto len = shmConnect->GetBuffer()->Read(buffer->GetWritePos(), BufferSize);
    buffer->SetLength(len);
    if (ioSubscriber_ != nullptr)
        ioSubscriber_->OnRecv(shmConnect->SessionId, buffer);
    else
        buffer->Deallocate();
}

bool ShmBase::WindowsInit()
{
#ifdef _WIN32
    if (serverType_ == ServerTypeType::Server)
    {
        file_ = CreateFileA(shmName_.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CREATE_NEW,
                            FILE_ATTRIBUTE_NORMAL, NULL);
        if (file_ == INVALID_HANDLE_VALUE)
        {
            WriteLog(LogLevel::Warning, "CreateFileA Failed. ErrNo:%d", GetLastError());
            return false;
        }
        fileMap_ = CreateFileMappingA(file_, NULL, PAGE_READWRITE, 0, GetSharedMemoryMappingSize(), shmName_.c_str());
    }
    else
    {
        fileMap_ = OpenFileMappingA(FILE_MAP_ALL_ACCESS, FALSE, shmName_.c_str());
    }
    if (fileMap_ == NULL)
    {
        WriteLog(LogLevel::Warning, "Create Or Open FileMapping Failed. ErrNo:%d", GetLastError());
        return false;
    }
    shmAddr_ = static_cast<char*>(MapViewOfFile(fileMap_, FILE_MAP_ALL_ACCESS, 0, 0, GetSharedMemoryMappingSize()));
    if (shmAddr_ == NULL)
    {
        WriteLog(LogLevel::Warning, "MapViewOfFile Failed. ErrNo:%d", GetLastError());
        return false;
    }
#endif
    return true;
}
bool ShmBase::LinuxInit()
{
#ifdef __linux__
    int fd;
    if (serverType_ == ServerTypeType::Server)
    {
        fd = shm_open(shmName_.c_str(), O_CREAT | O_EXCL | O_RDWR, 0666);
    }
    else
    {
        fd = shm_open(shmName_.c_str(), O_RDWR, 0666);
    }
    if (fd < 0)
    {
        WriteLog(LogLevel::Warning, "shm_open Failed. ErrNo:%d", errno);
        return false;
    }
    if (ftruncate(fd, GetSharedMemoryMappingSize()) == -1)
    {
        WriteLog(LogLevel::Warning, "ftruncate Failed. ErrNo:%d", errno);
        return false;
    }
    shmAddr_ = static_cast<char*>(mmap(nullptr, GetSharedMemoryMappingSize(), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0));
    if (shmAddr_ == MAP_FAILED)
    {
        WriteLog(LogLevel::Warning, "mmap Failed. ErrNo:%d", errno);
        return false;
    }
#endif
    return true;
}
}
