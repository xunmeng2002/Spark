#pragma once
#include "Shm/ShmBase.h"


namespace spark::network
{
class ShmClient : public ShmBase
{
public:
	ShmClient(const char* shmName, int milliSeconds);
	virtual ~ShmClient();

	virtual bool ConnectToServer(const char* addressName) override;
protected:
	virtual void ConnectToServer() override;
	virtual void CheckConnect() override;
	virtual void CheckData() override;
	virtual void HandleData() override;


	void SendConnect();
	void CheckConnectResult();
	virtual void RemoveConnect(Connect* connect) override;
protected:
	bool m_Connected;
	bool m_HasSendConnect;
	ShmConnect<ShmBuffSize>* m_ShmConnect;
};
}
