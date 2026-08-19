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


namespace spark::network
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
	ServerTypeType m_ServerType;
	std::string	m_AddressName;
	std::string m_Address;
	std::string m_Port;
	std::chrono::milliseconds m_TimeOut;
	IOSubscriber* m_IOSubscriber;
	SessionIDType m_LastSessionIndex;

	std::map<SessionIDType, Connect*> m_Connects;
	std::mutex m_ConnectsMutex;
	std::list<SessionIDType> m_DisConnectSessionIDs;
	std::mutex m_DisConnectSessionIDsMutex;

	std::mutex m_Mutex;
};
}

