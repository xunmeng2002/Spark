#pragma once
#include <Spark/Network/NetworkExport.h>
#include <Spark/Types.h>
#include <Spark/TemplateLib/Buffer/Buffer.h>
#include <list>
#include <mutex>

namespace spark::network
{
class NETWORK_EXPORTS Connect
{
public:
	Connect(SessionIDType sessionID, const char* remoteAddress, int remotePort,  ConnectStatusType connectStatus)
		:SessionID(sessionID), ConnectStatus(connectStatus), RemotePort(remotePort)
	{
		strncpy(RemoteAddress, remoteAddress, sizeof(RemoteAddress));
	}
	virtual ~Connect() = default;

	virtual void Deallocate() = 0;
	void PushBack(Buffer<BuffSize>* buffer);
	void PushFront(Buffer<BuffSize>* buffer);
	Buffer<BuffSize>* GetNextBuffer();

	SessionIDType SessionID = 0LL;
	ConnectStatusType ConnectStatus = ConnectStatusType::UnConnected;
	char RemoteAddress[40]{ 0 };
	int RemotePort = 0;
	std::list<Buffer<BuffSize>*> Buffers;
	std::mutex BuffersMutex;
};
}

