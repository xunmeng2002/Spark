#include "Shm/ShmServer.h"
#include <Spark/Core/Utility/TimeUtility.h>
#include <string.h>

using namespace std;

namespace spark::network
{
ShmServer::ShmServer(const char* shmName, int milliSeconds)
	:ShmBase(ServerTypeType::Server, shmName, milliSeconds), m_ConnectCount(0)
{
}
ShmServer::~ShmServer()
{
}
void ShmServer::Accept()
{
	switch (m_CommonShmHeader->Status)
	{
	case ConnectStatusType::UnConnected:
		break;
	case ConnectStatusType::Connecting:
	{
		if (m_SemConnect->Lock())
		{
			if (m_ConnectCount >= m_MaxConnectSize - 1)
			{
				m_CommonShmHeader->Status = ConnectStatusType::Rejected;
			}
			else
			{
				for (auto i = 1U; i < m_MaxConnectSize; ++i)
				{
					auto shmHeader = m_CommonShmHeader + i;
					if (shmHeader->Status == ConnectStatusType::UnConnected)
					{
						ShmConnect<ShmBuffSize>* shmConnect = ShmConnect<ShmBuffSize>::Allocate(GetSessionID(), m_Address.c_str(), i, m_ServerType, m_ShmAddr, ConnectStatusType::Accepted);
						AddConnect(shmConnect);

						m_CommonShmHeader->Status = ConnectStatusType::Accepted;
						m_CommonShmHeader->DownWriteCount = i;
						++m_ConnectCount;
						break;
					}
				}
			}
			m_LastWriteTimePoint = chrono::system_clock::now();
			m_SemConnect->UnLock();
		}
		else
		{
			printf("Sem Lock Failed.\n");
		}
		break;
	}
	case ConnectStatusType::Accepted:
	case ConnectStatusType::Rejected:
	{
		auto currTimePoint = chrono::system_clock::now();
		auto t = chrono::duration_cast<chrono::seconds>(currTimePoint - m_LastWriteTimePoint);
		if (t.count() >= 5)
		{
			if (m_SemConnect->Lock())
			{
				if (m_CommonShmHeader->Status == ConnectStatusType::Accepted || m_CommonShmHeader->Status == ConnectStatusType::Rejected)
				{
					printf("Reset Connect From Server,  Status:%d\n", (int)m_CommonShmHeader->Status);
					if (m_CommonShmHeader->Status == ConnectStatusType::Accepted)
					{
						auto index = m_CommonShmHeader->DownWriteCount;
						auto shmHeader = m_CommonShmHeader + index;
						memset(shmHeader, 0, sizeof(SingleShmHeader));
					}
					m_CommonShmHeader->Status = ConnectStatusType::UnConnected;
				}
				m_SemConnect->UnLock();
			}
			else
			{
				printf("Sem Lock Failed.\n");
			}
		}
	}
		break;
	case ConnectStatusType::Connected:
	case ConnectStatusType::DisConnected:
		break;
	default:
		break;
	}
}
void ShmServer::CheckConnect()
{
	for (auto& it : m_Connects)
	{
		auto shmConnect = (ShmConnect<ShmBuffSize>*)it.second;
		if (shmConnect->m_ShmBuffer->m_ShmHeader->Status == ConnectStatusType::DisConnected)
		{
			lock_guard<mutex> guard(m_DisConnectSessionIDsMutex);
			m_DisConnectSessionIDs.push_back(shmConnect->SessionID);
		}
	}
}
void ShmServer::CheckData()
{
	for (auto& it : m_Connects)
	{
		auto shmConnect = (ShmConnect<ShmBuffSize>*)it.second;
		if (shmConnect->m_ShmBuffer->GetReadBufferSize() > 0)
			return;
	}
	m_Sems[0]->Lock();
}
void ShmServer::HandleData()
{
	for (auto& it : m_Connects)
	{
		auto shmConnect = (ShmConnect<ShmBuffSize>*)it.second;
		if (shmConnect->m_ShmBuffer->GetReadBufferSize() > 0)
		{
			DoRecv(shmConnect);
		}
	}
}

void ShmServer::RemoveConnect(Connect* connect)
{
	ShmBase::RemoveConnect(connect);
	--m_ConnectCount;
}
}
