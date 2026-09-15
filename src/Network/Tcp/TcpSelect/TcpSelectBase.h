#pragma once
#include "Tcp/SocketInit.h"
#include "Tcp/TcpBase.h"
#include <list>
#include <map>


namespace Spark::Network
{
class TcpSelectBase : public TcpBase
{
public:
	TcpSelectBase(ServerTypeType serverType, const char* addressName, int milliSeconds);
	virtual void SetTimeOut(int milliSeconds) override;
protected:
	virtual void PrepareFds();
	virtual void HandleTcpEvent() override;

protected:
	fd_set readFds_;
	fd_set writeFds_;
	fd_set errorFds_;
	SOCKET maxId_;
	timeval selectSocketTimeOut_;
	timeval selectSocketTimeOutTemp_;
};
}
