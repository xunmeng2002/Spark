#pragma once
#include "Shm/ShmConnect.h"
#include <Spark/Types.h>
#include <Spark/TemplateLib/Buffer/Buffer.h>
#include <Spark/Network/Io/IoBase.h>
#include <string>


namespace Spark::Network
{
class SingleShm : public IoBase
{
public:
	SingleShm(ServerTypeType shmType, const char* shmName);
	virtual ~SingleShm();
	virtual bool Init() override;
	
	virtual void Send(SessionIdType sessionId, Buffer<BuffSize>* buffer) override;
	virtual void DoRecv(Connect* connect) override;
	virtual void HandleIoEvent() override;

protected:
	virtual void CheckConnectStatus();
	virtual void CheckEvent();
	virtual void HandleEvent();

public:
	std::string shmName_;
protected:
	bool connected_;
	SessionIdType sessionId_;
	void* shmAddr_;
	ShmBuffer<ShmBuffSize>* shmBuffer_;
#ifdef _WIN32
	void* file_;
	void* fileMap_;
#endif // _WIN32
};
}


