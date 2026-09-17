#include "Tcp/TcpSelect/TcpSelectBase.h"
#include <Spark/Core/Logger/Logger.h>
#include "Tcp/TcpUtility.h"
#include <string.h>


namespace Spark::Network
{
TcpSelectBase::TcpSelectBase(ServerTypeType serverType, const char* addressName, int milliSeconds)
    :TcpBase(serverType, addressName, milliSeconds)
{
    FD_ZERO(&readFds_);
    FD_ZERO(&writeFds_);
    FD_ZERO(&errorFds_);
    maxId_ = 0;
    selectSocketTimeOut_.tv_sec = milliSeconds / 1000;
    selectSocketTimeOut_.tv_usec = (milliSeconds % 1000) * 1000;
    memcpy(&selectSocketTimeOutTemp_, &selectSocketTimeOut_, sizeof(timeval));
}
void TcpSelectBase::SetTimeOut(int milliSeconds)
{
    IoBase::SetTimeOut(milliSeconds);

    selectSocketTimeOut_.tv_sec = milliSeconds / 1000;
    selectSocketTimeOut_.tv_usec = (milliSeconds % 1000) * 1000;
    memcpy(&selectSocketTimeOutTemp_, &selectSocketTimeOut_, sizeof(timeval));
}
void TcpSelectBase::PrepareFds()
{
    FD_ZERO(&readFds_);
    FD_ZERO(&writeFds_);
    FD_ZERO(&errorFds_);
    maxId_ = 0;
    FD_SET(socketNotify_->GetReadSocket(), &readFds_);
    for (auto& it : connects_)
    {
        auto connect = (TcpConnect*)it.second;
        FD_SET(connect->SocketId, &readFds_);
        FD_SET(connect->SocketId, &errorFds_);
        if (!connect->Buffers.empty())
        {
            FD_SET(connect->SocketId, &writeFds_);
        }
        if (connect->SocketId > maxId_)
        {
            maxId_ = connect->SocketId;
        }
    }
    if (serverType_ == ServerTypeType::Server)
    {
        FD_SET(socket_, &readFds_);
        if (socket_ > maxId_)
        {
            maxId_ = socket_;
        }
    }
    ++maxId_;
}
void TcpSelectBase::HandleTcpEvent()
{
    PrepareFds();
    memcpy(&selectSocketTimeOutTemp_, &selectSocketTimeOut_, sizeof(timeval));
    ::select((int)maxId_, &readFds_, &writeFds_, &errorFds_, &selectSocketTimeOutTemp_);
    if (FD_ISSET(socketNotify_->GetReadSocket(), &readFds_))
    {
        socketNotify_->Consume();
    }
    for (auto& it : connects_)
    {
        auto connect = (TcpConnect*)it.second;
        if (FD_ISSET(connect->SocketId, &writeFds_))
        {
            DoSend(connect);
        }
        if (FD_ISSET(connect->SocketId, &readFds_))
        {
            DoRecv(connect);
        }
    }
    for (auto& it : connects_)
    {
        auto connect = (TcpConnect*)it.second;
        if (FD_ISSET(connect->SocketId, &errorFds_))
        {
            DisConnect(connect->SessionId);
        }
    }
    if (serverType_ == ServerTypeType::Server)
    {
        if (FD_ISSET(socket_, &readFds_))
        {
            DoAccept();
        }
    }
}
}
