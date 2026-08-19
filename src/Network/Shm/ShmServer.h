#pragma once
#include "Shm/ShmBase.h"
#include <chrono>


namespace spark::network
{
class ShmServer : public ShmBase
{
public:
	ShmServer(const char* shmName, int milliSeconds);
	~ShmServer();

private:
	virtual void Accept() override;
	virtual void CheckConnect() override;
	virtual void CheckData() override;
	virtual void HandleData() override;

	virtual void RemoveConnect(Connect* connect) override;
protected:
	unsigned m_ConnectCount;
	std::chrono::system_clock::time_point m_LastWriteTimePoint;
};
}

