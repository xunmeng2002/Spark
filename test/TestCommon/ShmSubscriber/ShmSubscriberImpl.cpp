#include "ShmSubscriber/ShmSubscriberImpl.h"
#include "TestUtility/TestUtility.h"
#include <Spark/Core/Logger/Logger.h>
#include <string.h>
#include <assert.h>

using namespace Spark;
using namespace Spark::Core;
using namespace Spark::Network;

static int g_Count = 0;
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
void ShmSubscriberImpl::OnRecv(SessionIdType sessionId, LinearBuffer<BufferSize>* buffer)
{
    while (buffer->GetLength() > 0)
    {
        ShmPackage* shmPackage = nullptr;
        if (length_ > 0)
        {
            size_t len = sizeof(ShmPackage) - length_;
            len = std::min(len, buffer->GetLength());
            memcpy(buff_ + length_, buffer->GetData(), len);
            length_ += len;
            if (length_ == sizeof(ShmPackage))
            {
                shmPackage = (ShmPackage*)buff_;
                length_ = 0;
            }
            buffer->Shift(len);
        }
        else
        {
            if (buffer->GetLength() >= sizeof(ShmPackage))
            {
                shmPackage = (ShmPackage*)buffer->GetData();
                buffer->Shift(sizeof(ShmPackage));
            }
            else
            {
                memcpy(buff_, buffer->GetData(), buffer->GetLength());
                length_ += buffer->GetLength();
                buffer->Shift(buffer->GetLength());
            }
        }
        if (shmPackage != nullptr)
        {
            WriteLog(LogLevel::Info, "ShmSubscriberImpl::OnRecv ShmType[%d], Count[%d], Data[%s]", shmPackage->ShmType, shmPackage->Count,
                     shmPackage->Data);
            if (serverType_ == ServerTypeType::Server)
            {
                shmPackage->ShmType = (int)ServerTypeType::Server;
                auto sendBuff = new LinearBuffer<BufferSize>();
                sendBuff->Append((char*)shmPackage, sizeof(ShmPackage));
                io_->Send(sessionId, sendBuff);
            }
        }
    }
}
