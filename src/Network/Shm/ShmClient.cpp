#include "Shm/ShmClient.h"
#include <Spark/Core/Logger/Logger.h>
#include <string.h>

using namespace std;

using namespace spark::core;
namespace spark::network
{
ShmClient::ShmClient(const char* shmName, int milliSeconds)
	:ShmBase(ServerTypeType::Client, shmName, milliSeconds), m_Connected(false), m_HasSendConnect(false), m_ShmConnect(nullptr)
{
}
ShmClient::~ShmClient()
{
	m_ShmConnect = nullptr;
}
bool ShmClient::ConnectToServer(const char* addressName)
{
	string address, port;
	ParseAddress(addressName, address, port);
	if (address == m_Address && m_Connected)
	{
		return true;
	}
	if (address != m_Address)
	{
		RemoveConnect(m_ShmConnect);
		m_Address = address;
		m_Port = port;
		m_Connected = false;
		m_HasSendConnect = false;
		m_ShmConnect = nullptr;
		WriteLog(LogLevel::Info, "Address Changed. Address:%s Port:%s\n", m_Address.c_str(), m_Port.c_str());
		if (!Init())
			return false;
	}
	ConnectToServer();
	while (m_HasSendConnect && m_CommonShmHeader->Status == ConnectStatusType::Connecting)
	{
		this_thread::sleep_for(std::chrono::milliseconds(10));
	}
	return m_Connected;
}
void ShmClient::ConnectToServer()
{
	if (m_Connected)
		return;
	if (!m_HasSendConnect && m_CommonShmHeader->Status == ConnectStatusType::UnConnected)
	{
		SendConnect();
	}
	else if (m_HasSendConnect && m_CommonShmHeader->Status != ConnectStatusType::Connecting)
	{
		CheckConnectResult();
	}
}
void ShmClient::CheckConnect()
{
	if (!m_Connected)
		return;
	if (m_ShmConnect->m_ShmBuffer->m_ShmHeader->Status == ConnectStatusType::DisConnected)
	{
		RemoveConnect(m_ShmConnect);
	}
}
void ShmClient::CheckData()
{
	if (!m_Connected)
		return;
	if (m_ShmConnect->m_ShmBuffer->GetReadBufferSize() > 0)
		return;
	m_Sems[m_ShmConnect->RemotePort]->Lock();
}
void ShmClient::HandleData()
{
	if (!m_Connected)
		return;
	if (m_ShmConnect->m_ShmBuffer->GetReadBufferSize() > 0)
	{
		DoRecv(m_ShmConnect);
	}
}


void ShmClient::SendConnect()
{
	if (m_SemConnect->Lock())
	{
		if (m_CommonShmHeader->Status == ConnectStatusType::UnConnected)
		{
			m_CommonShmHeader->Status = ConnectStatusType::Connecting;
			m_HasSendConnect = true;
		}
		m_SemConnect->UnLock();
	}
	else
	{
		WriteLog(LogLevel::Info, "Sem Lock Failed. Sleep 10ms\n");
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
}
void ShmClient::CheckConnectResult()
{
	if (m_SemConnect->Lock())
	{
		m_HasSendConnect = false;
		if (m_CommonShmHeader->Status == ConnectStatusType::Accepted)
		{
			auto index = m_CommonShmHeader->DownWriteCount;
			m_ShmConnect = ShmConnect<ShmBuffSize>::Allocate(GetSessionID(), m_Address.c_str(), index, m_ServerType, m_ShmAddr, ConnectStatusType::Connected);
			AddConnect(m_ShmConnect);
			m_Connected = true;
			m_CommonShmHeader->Status = ConnectStatusType::UnConnected;
		}
		else if (m_CommonShmHeader->Status == ConnectStatusType::Rejected)
		{
			m_CommonShmHeader->Status = ConnectStatusType::UnConnected;
		}
		else
		{
			WriteLog(LogLevel::Info, "UnExpected Status:%d\n", (int)m_CommonShmHeader->Status);
		}
		m_SemConnect->UnLock();
		if (!m_Connected)
		{
			WriteLog(LogLevel::Info, "Connect Failed. Sleep 1s\n");
			std::this_thread::sleep_for(std::chrono::seconds(1));
		}
	}
	else
	{
		WriteLog(LogLevel::Info, "Sem Lock Failed. Sleep 10ms\n");
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
}
void ShmClient::RemoveConnect(Connect* connect)
{
	ShmBase::RemoveConnect(connect);
	m_Connected = false;
	m_HasSendConnect = false;
	m_ShmConnect = nullptr;
}
}
