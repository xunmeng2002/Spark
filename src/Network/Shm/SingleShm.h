#pragma once
#include "Shm/ShmConnect.h"
#include <Spark/Types.h>
#include <Spark/TemplateLib/Buffer/Buffer.h>
#include <Spark/Network/IO/IOBase.h>
#include <string>


namespace spark::network
{
class SingleShm : public IOBase
{
public:
	SingleShm(ServerTypeType shmType, const char* shmName);
	virtual ~SingleShm();
	virtual bool Init() override;
	
	virtual void Send(SessionIDType sessionID, Buffer<BuffSize>* buffer) override;
	virtual void DoRecv(Connect* connect) override;
	virtual void HandleIOEvent() override;

protected:
	virtual void CheckConnectStatus();
	virtual void CheckEvent();
	virtual void HandleEvent();

public:
	std::string m_ShmName;
protected:
	bool m_Connected;
	SessionIDType m_SessionID;
	void* m_ShmAddr;
	ShmBuffer<ShmBuffSize>* m_ShmBuffer;
#ifdef WINDOWS
	void* m_File;
	void* m_FileMap;
#endif // WINDOWS
};
}


