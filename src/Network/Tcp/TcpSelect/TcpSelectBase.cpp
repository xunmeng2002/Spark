#include "Tcp/TcpSelect/TcpSelectBase.h"
#include <Spark/Core/Logger/Logger.h>
#include "Tcp/TcpUtility.h"
#include <string.h>


namespace spark::network
{
TcpSelectBase::TcpSelectBase(ServerTypeType serverType, const char* addressName, int milliSeconds)
	:TcpBase(serverType, addressName, milliSeconds)
{
	FD_ZERO(&m_ReadFds);
	FD_ZERO(&m_WriteFds);
	FD_ZERO(&m_ErrorFds);
	m_MaxID = 0;
	m_SelectSocketTimeOut.tv_sec = milliSeconds / 1000;
	m_SelectSocketTimeOut.tv_usec = (milliSeconds % 1000) * 1000;
	memcpy(&m_SelectSocketTimeOutTemp, &m_SelectSocketTimeOut, sizeof(timeval));
}
void TcpSelectBase::SetTimeOut(int milliSeconds)
{
	IOBase::SetTimeOut(milliSeconds);

	m_SelectSocketTimeOut.tv_sec = milliSeconds / 1000;
	m_SelectSocketTimeOut.tv_usec = (milliSeconds % 1000) * 1000;
	memcpy(&m_SelectSocketTimeOutTemp, &m_SelectSocketTimeOut, sizeof(timeval));
}
void TcpSelectBase::PrepareFds()
{
	FD_ZERO(&m_ReadFds);
	FD_ZERO(&m_WriteFds);
	FD_ZERO(&m_ErrorFds);
	m_MaxID = 0;
	FD_SET(m_SocketNotify->GetReadSocket(), &m_ReadFds);
	for (auto& it : m_Connects)
	{
		auto connect = (TcpConnect*)it.second;
		FD_SET(connect->SocketID, &m_ReadFds);
		FD_SET(connect->SocketID, &m_ErrorFds);
		if (!connect->Buffers.empty())
		{
			FD_SET(connect->SocketID, &m_WriteFds);
		}
		if (connect->SocketID > m_MaxID)
		{
			m_MaxID = connect->SocketID;
		}
	}
	if (m_ServerType == ServerTypeType::Server)
	{
		FD_SET(m_Socket, &m_ReadFds);
		if (m_Socket > m_MaxID)
		{
			m_MaxID = m_Socket;
		}
	}
	++m_MaxID;
}
void TcpSelectBase::HandleTcpEvent()
{
	PrepareFds();
	memcpy(&m_SelectSocketTimeOutTemp, &m_SelectSocketTimeOut, sizeof(timeval));
	::select((int)m_MaxID, &m_ReadFds, &m_WriteFds, &m_ErrorFds, &m_SelectSocketTimeOutTemp);
	if (FD_ISSET(m_SocketNotify->GetReadSocket(), &m_ReadFds))
	{
		m_SocketNotify->Consume();
	}
	for (auto& it : m_Connects)
	{
		auto connect = (TcpConnect*)it.second;
		if (FD_ISSET(connect->SocketID, &m_WriteFds))
		{
			DoSend(connect);
		}
		if (FD_ISSET(connect->SocketID, &m_ReadFds))
		{
			DoRecv(connect);
		}
	}
	for (auto& it : m_Connects)
	{
		auto connect = (TcpConnect*)it.second;
		if (FD_ISSET(connect->SocketID, &m_ErrorFds))
		{
			DisConnect(connect->SessionID);
		}
	}
	if (m_ServerType == ServerTypeType::Server)
	{
		if (FD_ISSET(m_Socket, &m_ReadFds))
		{
			DoAccept();
		}
	}
}
}
