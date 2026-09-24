#include <Spark/Network/Io/IoFactory.h>
#include "Tcp/SocketInit.h"
#ifdef __linux__
#include "Tcp/TcpEpoll/TcpEpollClient.h"
#include "Tcp/TcpEpoll/TcpEpollServer.h"
#elif defined _WIN32
#include "Tcp/TcpIocp/TcpIocpClient.h"
#include "Tcp/TcpIocp/TcpIocpServer.h"
#endif
#include "Tcp/TcpSelect/TcpSelectClient.h"
#include "Tcp/TcpSelect/TcpSelectServer.h"
#include "Shm/ShmClient.h"
#include "Shm/ShmServer.h"
#include <Spark/Core/Core.h>
#include <Spark/EnumString.h>
#include <format>
#include <stdexcept>

using namespace Spark;
using namespace Spark::Core;

namespace Spark::Network
{
IoBase* IoFactory::CreateIo(ServerTypeType serverType, const char* address, IoModelType ioModel, int milliSeconds)
{
    IoTypeType ioType;
    if (strncmp(address, "tcp", 3) == 0)
    {
        ioType = IoTypeType::Tcp;
    }
    else if (strncmp(address, "udp", 3) == 0)
    {
        ioType = IoTypeType::Udp;
    }
    else if (strncmp(address, "shm", 3) == 0)
    {
        ioType = IoTypeType::Shm;
    }
    else
    {
        throw std::logic_error(std::format("Invalid Address:{}", address));
    }
    auto addressName = address + 6;
    WriteLog(LogLevel::Info, "CreateIo ServerType:%s, Address:%s, IoType:%s, IoModel:%s", GetServerTypeString(serverType), address,
             GetIoTypeString(ioType), GetIoModelString(ioModel));
    if (serverType == ServerTypeType::Client)
    {
        switch (ioType)
        {
        case IoTypeType::Tcp:
        {
            switch (ioModel)
            {
            case IoModelType::Select:
                return new TcpSelectClient(addressName, milliSeconds);
#ifdef __linux__
            case IoModelType::Epoll:
                return new TcpEpollClient(addressName, milliSeconds);
#endif
#if defined _WIN32
            case IoModelType::Iocp:
                return new TcpIocpClient(addressName, milliSeconds);
#endif
            default:
                return new TcpSelectClient(addressName, milliSeconds);
            }
        }
        case IoTypeType::Udp:
            break;
        case IoTypeType::Shm:
            return new ShmClient(addressName, milliSeconds);
        default:
            break;
        }
    }
    else
    {
        switch (ioType)
        {
        case IoTypeType::Tcp:
        {
            switch (ioModel)
            {
            case IoModelType::Select:
                return new TcpSelectServer(addressName, milliSeconds);
#ifdef __linux__
            case IoModelType::Epoll:
                return new TcpEpollServer(addressName, milliSeconds);
#endif
#if defined _WIN32
            case IoModelType::Iocp:
                return new TcpIocpServer(addressName, milliSeconds);
#endif
            default:
                return new TcpSelectServer(addressName, milliSeconds);
            }
        }
        case IoTypeType::Udp:
            break;
        case IoTypeType::Shm:
            return new ShmServer(addressName, milliSeconds);
        default:
            break;
        }
    }
    return nullptr;
}
}
