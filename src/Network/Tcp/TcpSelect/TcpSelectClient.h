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

	std::map<SessionIdType, TcpConnect*> connectings_;
	std::list<SessionIdType> connectSuccessedSessions_;
	std::list<SessionIdType> connectFailedSessions_;
};
}
