#pragma once
#include "Tcp/TcpSelect/TcpSelectBase.h"


namespace Spark::Network
{
class TcpSelectClient : public TcpSelectBase
{
public:
	TcpSelectClient(const char* addressName, int milliSeconds);

	virtual bool ConnectToServer(const char* ip, unsigned short port) override;

	void CheckConnect();

protected:
	fd_set writeFds_;

	std::map<SessionIDType, TcpConnect*> connectings_;
	std::list<SessionIDType> connectSuccessedSessions_;
	std::list<SessionIDType> connectFailedSessions_;
};
}
