#pragma once
#include <Spark/Network/NetworkExport.h>
#include <Spark/Network/Protocol/PackageReader.h>
#include <Spark/Network/Protocol/PackageFactoryBase.h>
#include <Spark/Network/Protocol/ProtocolSubscriber.h>
#include <Spark/Network/IO/IOBase.h>
#include <Spark/Network/IO/IOThread.h>
#include <map>

namespace spark::network
{
class NETWORK_EXPORTS Protocol : public IOSubscriber
{
public:
	Protocol(ProtocolTypeType protocolType, ServerTypeType serverType, IOModelType ioModel, int milliSeconds, PackageFactoryBase* packageFactory);
	~Protocol();
	void Subscribe(ProtocolSubscriber* subscriber);
	void UnSubscribe();
	void RegisterFront(const char* address);
	void SetIOThread(IOThread* ioThread);
	void SetTimeOut(int milliSeconds);
	bool Start();
	void Stop();
	void Join();
	virtual bool Init();
	IOBase* GetIO();
	IOThread* GetIOThread();

	void DisConnect(SessionIDType sessionID);
	virtual bool Send(Package* package);

	virtual void OnConnect(SessionIDType sessionID, const char* ip, int port) override;
	virtual void OnDisConnect(SessionIDType sessionID, const char* ip, int port) override;
	virtual void OnRecv(SessionIDType sessionID, Buffer<BuffSize>* buffer) override;

protected:
	ProtocolTypeType m_ProtocolType;
	ServerTypeType m_ServerType;
	IOModelType m_IOModel;
	int m_MilliSeconds;
	IOBase* m_IOBase;
	IOThread* m_IOThread;
    PackageFactoryBase* m_PackageFactory;
	ProtocolSubscriber* m_Subscriber;
	std::map<SessionIDType, PackageReader*> m_SessionPackageReaders;
};
}
