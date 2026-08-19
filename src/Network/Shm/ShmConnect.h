#pragma once
#include <Spark/Network/NetworkExport.h>
#include <Spark/Types.h>
#include <Spark/Network/IO/Connect.h>
#include <Spark/TemplateLib/Buffer/ShmBuffer.h>
#include <Spark/TemplateLib/ObjectPool/ObjectPool.h>


namespace spark::network
{
template<unsigned SIZE>
class ShmConnect : public Connect
{
public:
	ShmConnect(SessionIDType sessionID, const char* remoteAddress, int remotePort, ServerTypeType serverType, void* shmAddr, ConnectStatusType connectStatus)
		:Connect(sessionID, remoteAddress, remotePort, connectStatus)
	{
		m_ShmBuffer = ShmBuffer<SIZE>::Allocate(serverType, remotePort, shmAddr, connectStatus);
	}
	virtual ~ShmConnect()
	{
		m_ShmBuffer->Deallocate();
		m_ShmBuffer = nullptr;
	}

	static ShmConnect* Allocate(SessionIDType sessionID, const char* remoteAddress, int remotePort, ServerTypeType serverType, void* shmAddr, ConnectStatusType connectStatus)
	{
		return ObjectPool<ShmConnect<SIZE>>::GetInstance().Allocate(sessionID, remoteAddress, remotePort, serverType, shmAddr, connectStatus);
	}
	virtual void Deallocate() override
	{
		ObjectPool<ShmConnect<SIZE>>::GetInstance().Deallocate(this);
	}

public:
	ShmBuffer<SIZE>* m_ShmBuffer;
};
}

