#include "ShmSubscriber/ShmSubscriberImpl.h"
#include "TestUtility/TestUtility.h"
#include <Spark/Core/Logger/Logger.h>
#include <string.h>
#include <assert.h>

using namespace spark;
using namespace spark::core;
using namespace spark::network;

static int g_Count = 0;
ShmSubscriberImpl::ShmSubscriberImpl(IOBase* io, ServerTypeType serverType)
	:m_IO(io), m_ServerType(serverType), m_Connected(false), m_SessionID(0LL)
{
	m_Buff = new char[BuffSize];
	m_Length = 0;

	m_IO->Subscribe(this);
}
void ShmSubscriberImpl::OnConnect(SessionIDType sessionID, const char* ip, int port)
{
	WriteLog(LogLevel::Info, "OnConnect sessionID:%lld, ip:%s, port:%d", sessionID, ip, port);
	m_Connected = true;
	m_SessionID = sessionID;
}
void ShmSubscriberImpl::OnDisConnect(SessionIDType sessionID, const char* ip, int port)
{
	WriteLog(LogLevel::Info, "OnDisConnect sessionID:%lld, ip:%s, port:%d", sessionID, ip, port);
	m_Connected = false;
}
void ShmSubscriberImpl::OnRecv(SessionIDType sessionID, Buffer<BuffSize>* buffer)
{
	while (buffer->GetLength() > 0)
	{
		ShmPackage* shmPackage = nullptr;
		if (m_Length > 0)
		{
			unsigned len = sizeof(ShmPackage) - m_Length;
			len = std::min(len, buffer->GetLength());
			memcpy(m_Buff + m_Length, buffer->GetData(), len);
			m_Length += len;
			if (m_Length == sizeof(ShmPackage))
			{
				shmPackage = (ShmPackage*)m_Buff;
				m_Length = 0;
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
				memcpy(m_Buff, buffer->GetData(), buffer->GetLength());
				m_Length += buffer->GetLength();
				buffer->Shift(buffer->GetLength());
			}
		}
		if (shmPackage != nullptr)
		{
			WriteLog(LogLevel::Info, "ShmSubscriberImpl::OnRecv ShmType[%d], Count[%d], Data[%s]", shmPackage->ShmType, shmPackage->Count, shmPackage->Data);
			if (m_ServerType == ServerTypeType::Server)
			{
				shmPackage->ShmType = (int)ServerTypeType::Server;
				auto sendBuff = new Buffer<BuffSize>();
				sendBuff->Append((char*)shmPackage, sizeof(ShmPackage));
				m_IO->Send(sessionID, sendBuff);
			}
		}
	}
}
