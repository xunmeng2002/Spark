#pragma once
#include "Tcp/SocketInit.h"
#include "Tcp/TcpBase.h"
#include <list>
#include <map>


namespace spark::network
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
	fd_set m_ReadFds;
	fd_set m_WriteFds;
	fd_set m_ErrorFds;
	SOCKET m_MaxID;
	timeval m_SelectSocketTimeOut;
	timeval m_SelectSocketTimeOutTemp;
};
}
