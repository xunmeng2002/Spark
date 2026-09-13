#include <Spark/Network/Protocol/Protocol.h>
#include <Spark/Core/Logger/Logger.h>
#include <Spark/Network/IO/IOFactory.h>
#include <stdexcept>

using namespace spark::core;
namespace spark::network
{
Protocol::Protocol(ProtocolTypeType protocolType, ServerTypeType serverType, IOModelType ioModel, int milliSeconds, PackageFactoryBase* packageFactory)
	:m_ProtocolType(protocolType), m_ServerType(serverType), m_IOModel(ioModel), m_MilliSeconds(milliSeconds), m_Subscriber(nullptr), m_PackageFactory(packageFactory), m_IOBase(nullptr), m_IOThread(nullptr)
{
}
Protocol::~Protocol()
{
	if (m_IOBase != nullptr)
	{
		delete m_IOBase;
		m_IOBase = nullptr;
	}
}
void Protocol::Subscribe(ProtocolSubscriber* subscriber)
{
	m_Subscriber = subscriber;
}
void Protocol::UnSubscribe()
{
	m_Subscriber = nullptr;
}
void Protocol::RegisterFront(const char* address)
{
	if (m_IOBase != nullptr)
	{
		delete m_IOBase;
	}
	m_IOBase = IOFactory::CreateIO(m_ServerType, address, m_IOModel, m_MilliSeconds);
	m_IOBase->Subscribe(this);
	if (m_IOThread != nullptr)
	{
		m_IOThread->SetIO(m_IOBase);
	}
}
void Protocol::SetIOThread(IOThread* ioThread)
{
	m_IOThread = ioThread;
	if (m_IOBase != nullptr)
	{
		m_IOThread->SetIO(m_IOBase);
	}
}
void Protocol::SetTimeOut(int milliSeconds)
{
	m_MilliSeconds = milliSeconds;
	if (m_IOBase != nullptr)
	{
		m_IOBase->SetTimeOut(milliSeconds);
	}
}
bool Protocol::Start()
{
	if (m_IOThread != nullptr)
	{
		return m_IOThread->Start();
	}
	return false;
}
void Protocol::Stop()
{
	if (m_IOThread != nullptr)
	{
		m_IOThread->Stop();
	}
}
void Protocol::Join()
{
	if (m_IOThread != nullptr)
	{
		m_IOThread->Join();
	}
}
bool Protocol::Init()
{
	if (m_IOBase == nullptr)
		return false;
	return m_IOBase->Init();
}
IOBase* Protocol::GetIO()
{
	return m_IOBase;
}
IOThread* Protocol::GetIOThread()
{
	return m_IOThread;
}

void Protocol::DisConnect(SessionIDType sessionID)
{
	if (m_IOBase == nullptr)
		return;
	m_IOBase->DisConnect(sessionID);
}
bool Protocol::Send(Package* package)
{
	if (m_IOBase == nullptr)
		return false;
	Buffer<BuffSize>* buffer = Buffer<BuffSize>::Allocate();
	auto len = package->MakePackage(m_ProtocolType, buffer->GetData(), BuffSize);
	if (len <= 0)
	{
		WriteLog(LogLevel::Info, "MakePackage len is 0");
	}
	buffer->SetLength(len);
	m_IOBase->Send(package->SessionID, buffer);
	return true;
}

void Protocol::OnConnect(SessionIDType sessionID, const char* ip, int port)
{
	WriteLog(LogLevel::Info, "Protocol::OnConnect SessionID:%lld, IP:%s, Port:%d", sessionID, ip, port);
	m_SessionPackageReaders.insert(std::make_pair(sessionID, PackageReader::Allocate(m_ProtocolType, m_PackageFactory, sessionID, ip)));
	if (m_Subscriber)
	{
		m_Subscriber->OnProtocolConnect(sessionID, ip, port);
	}
}
void Protocol::OnDisConnect(SessionIDType sessionID, const char* ip, int port)
{
	WriteLog(LogLevel::Info, "Protocol::OnDisConnect SessionID:%lld, IP:%s, Port:%d", sessionID, ip, port);
	auto it = m_SessionPackageReaders.find(sessionID);
	if (it != m_SessionPackageReaders.end())
	{
		it->second->Deallocate();
		m_SessionPackageReaders.erase(it);
	}
	if (m_Subscriber)
	{
		m_Subscriber->OnProtocolDisConnect(sessionID, ip, port);
	}
}
void Protocol::OnRecv(SessionIDType sessionID, Buffer<BuffSize>* buffer)
{
	if (m_IOBase == nullptr)
	{
		buffer->Deallocate();
		return;
	}
	auto it = m_SessionPackageReaders.find(sessionID);
	if (it == m_SessionPackageReaders.end() || it->second == nullptr)
	{
		WriteLog(LogLevel::Error, "Cannot Find PackageReader for SessionID:%lld", sessionID);
		buffer->Deallocate();
		m_IOBase->DisConnect(sessionID);
		return;
	}
	auto packageReader = it->second;
	packageReader->Append(buffer->GetData(), buffer->GetLength());
	//Append 已经拷走字节，之后再无引用，所以在这里归还，后面的解析路径不必再考虑释放
	buffer->Deallocate();
	while (true)
	{
		Package* package = nullptr;
		if (!packageReader->ParsePackage(package))
		{
			m_IOBase->DisConnect(sessionID);
			break;
		}
		else if (package == nullptr)
		{
			break;
		}
		else if (m_Subscriber != nullptr)
		{
			m_Subscriber->OnMessage(package);
		}
	}
}
}

