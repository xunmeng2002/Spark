#include "Shm/ShmServer.h"
#include <Spark/Core/Utility/TimeUtility.h>
#include <string.h>

using namespace std;

namespace Spark::Network
{
ShmServer::ShmServer(const char* shmName, int milliSeconds)
	:ShmBase(ServerTypeType::Server, shmName, milliSeconds), connectCount_(0)
{
}
ShmServer::~ShmServer()
{
}
void ShmServer::Accept()
{
	switch (commonShmHeader_->Status)
	{
	case ConnectStatusType::UnConnected:
		break;
	case ConnectStatusType::Connecting:
	{
		if (semConnect_->Lock())
		{
			if (connectCount_ >= maxConnectSize_ - 1)
			{
				commonShmHeader_->Status = ConnectStatusType::Rejected;
			}
			else
			{
				for (auto i = 1U; i < maxConnectSize_; ++i)
				{
					auto shmHeader = commonShmHeader_ + i;
					if (shmHeader->Status == ConnectStatusType::UnConnected)
					{
						ShmConnect<ShmBuffSize>* shmConnect = ShmConnect<ShmBuffSize>::Allocate(GetSessionID(), address_.c_str(), i, serverType_, shmAddr_, ConnectStatusType::Accepted);
						AddConnect(shmConnect);

						commonShmHeader_->Status = ConnectStatusType::Accepted;
						commonShmHeader_->DownWriteCount = i;
						++connectCount_;
						break;
					}
				}
			}
			lastWriteTimePoint_ = chrono::system_clock::now();
			semConnect_->UnLock();
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
		auto t = chrono::duration_cast<chrono::seconds>(currTimePoint - lastWriteTimePoint_);
		if (t.count() >= 5)
		{
			if (semConnect_->Lock())
			{
				if (commonShmHeader_->Status == ConnectStatusType::Accepted || commonShmHeader_->Status == ConnectStatusType::Rejected)
				{
					printf("Reset Connect From Server,  Status:%d\n", (int)commonShmHeader_->Status);
					if (commonShmHeader_->Status == ConnectStatusType::Accepted)
					{
						auto index = commonShmHeader_->DownWriteCount;
						auto shmHeader = commonShmHeader_ + index;
						memset(shmHeader, 0, sizeof(SingleShmHeader));
					}
					commonShmHeader_->Status = ConnectStatusType::UnConnected;
				}
				semConnect_->UnLock();
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
	for (auto& it : connects_)
	{
		auto shmConnect = (ShmConnect<ShmBuffSize>*)it.second;
		if (shmConnect->GetBuffer()->ShmHeader->Status == ConnectStatusType::DisConnected)
		{
			lock_guard<mutex> guard(disConnectSessionIdsMutex_);
			disConnectSessionIds_.push_back(shmConnect->SessionID);
		}
	}
}
void ShmServer::CheckData()
{
	for (auto& it : connects_)
	{
		auto shmConnect = (ShmConnect<ShmBuffSize>*)it.second;
		if (shmConnect->GetBuffer()->GetReadBufferSize() > 0)
			return;
	}
	sems_[0]->Lock();
}
void ShmServer::HandleData()
{
	for (auto& it : connects_)
	{
		auto shmConnect = (ShmConnect<ShmBuffSize>*)it.second;
		if (shmConnect->GetBuffer()->GetReadBufferSize() > 0)
		{
			DoRecv(shmConnect);
		}
	}
}

void ShmServer::RemoveConnect(Connect* connect)
{
	ShmBase::RemoveConnect(connect);
	--connectCount_;
}
}
