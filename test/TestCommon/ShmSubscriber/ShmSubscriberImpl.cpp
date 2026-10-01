#include "ShmSubscriber/ShmSubscriberImpl.h"
#include "TestUtility/TestUtility.h"
#include <Spark/Core/Logger/Logger.h>
#include <string.h>
#include <assert.h>

using namespace Spark;
using namespace Spark::Core;
using namespace Spark::Network;

ShmSubscriberImpl::ShmSubscriberImpl(IoBase* io, ServerTypeType serverType) : connected_(false), sessionId_(0LL), io_(io), serverType_(serverType)
{
    buff_ = new char[BufferSize];
    length_ = 0;

    io_->Subscribe(this);
}
void ShmSubscriberImpl::OnConnect(SessionIdType sessionId, const char* ip, int port)
{
    WriteLog(LogLevel::Info, "OnConnect sessionId:%lld, ip:%s, port:%d", sessionId, ip, port);
    connected_ = true;
    sessionId_ = sessionId;
}
void ShmSubscriberImpl::OnDisConnect(SessionIdType sessionId, const char* ip, int port)
{
    WriteLog(LogLevel::Info, "OnDisConnect sessionId:%lld, ip:%s, port:%d", sessionId, ip, port);
    connected_ = false;
}
void ShmSubscriberImpl::OnRecv(SessionIdType sessionId, const char* data, size_t length)
{
    size_t offset = 0;
    while (offset < length)
    {
        ShmPackage* shmPackage = nullptr;
        if (length_ > 0)
        {
            size_t len = sizeof(ShmPackage) - length_;
            len = std::min(len, length - offset);
            memcpy(buff_ + length_, data + offset, len);
            length_ += len;
            offset += len;
            if (length_ == sizeof(ShmPackage))
            {
                shmPackage = reinterpret_cast<ShmPackage*>(buff_);
                length_ = 0;
            }
        }
        else
        {
            if (length - offset >= sizeof(ShmPackage))
            {
                memcpy(buff_, data + offset, sizeof(ShmPackage));
                shmPackage = reinterpret_cast<ShmPackage*>(buff_);
                offset += sizeof(ShmPackage);
            }
            else
            {
                memcpy(buff_, data + offset, length - offset);
                length_ += length - offset;
                offset = length;
            }
        }
        if (shmPackage != nullptr)
        {
            WriteLog(LogLevel::Info, "ShmSubscriberImpl::OnRecv ShmType[%d], Count[%d], Data[%s]", shmPackage->ShmType, shmPackage->Count,
                     shmPackage->Data);
            if (serverType_ == ServerTypeType::Server)
            {
                shmPackage->ShmType = static_cast<int>(ServerTypeType::Server);
                auto sendBuff = io_->AllocateSendBuffer();
                sendBuff->Append(reinterpret_cast<char*>(shmPackage), sizeof(ShmPackage));
                io_->Send(sessionId, sendBuff);
            }
        }
    }
}
