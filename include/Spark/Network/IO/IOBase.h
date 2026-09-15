#pragma once
#include <Spark/Network/NetworkExport.h>
#include <Spark/Types.h>
#include <Spark/TemplateLib/Buffer/Buffer.h>
#include <Spark/Network/IO/Connect.h>
#include <Spark/Network/IO/IOUtility.h>
#include <string>
#include <chrono>
#include <mutex>
#include <list>
#include <map>
#include <condition_variable>


namespace Spark::Network
{
class IOSubscriber
{
public:
	virtual void OnConnect(SessionIDType sessionID, const char* ip, int port) = 0;
	virtual void OnDisConnect(SessionIDType sessionID, const char* ip, int port) = 0;
	virtual void OnRecv(SessionIDType sessionID, Buffer<BuffSize>* buffer) = 0;
};

class NETWORK_EXPORTS IOBase
{
public:
	IOBase(ServerTypeType serverType, const char* addressName, int milliSeconds);
	virtual ~IOBase();
	void Subscribe(IOSubscriber* subscriber);
	void UnSubscribe();
	virtual void SetTimeOut(int milliSeconds);

	virtual bool Init() { return true; }
	virtual bool ConnectToServer(const char* addressName) { return false; }
	virtual void DisConnect(SessionIDType sessionID);
	virtual void DisConnectAll();
	virtual void Send(SessionIDType sessionID, Buffer<BuffSize>* buffer) = 0;
	
	virtual void HandleIOEvent() = 0;

protected:
	virtual void DoDisConnect();
	virtual void DoSend(Connect* connect) = 0;
	virtual void DoRecv(Connect* connect) = 0;

	virtual void AddConnect(Connect* connect);
	virtual void RemoveConnect(Connect* connect);
	virtual Connect* GetConnect(SessionIDType sessionID);

	SessionIDType GetSessionID();
	

protected:
	ServerTypeType serverType_;
	std::string	addressName_;
	std::string address_;
	std::string port_;
	std::chrono::milliseconds timeOut_;
	IOSubscriber* ioSubscriber_;
	SessionIDType lastSessionIndex_;

	std::map<SessionIDType, Connect*> connects_;
	std::mutex connectsMutex_;
	std::list<SessionIDType> disConnectSessionIds_;
	std::mutex disConnectSessionIdsMutex_;

	std::mutex mutex_;
};
}

