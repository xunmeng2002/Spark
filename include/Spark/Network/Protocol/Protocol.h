#pragma once
#include <Spark/Network/NetworkExport.h>
#include <Spark/Network/Protocol/PackageReader.h>
#include <Spark/Network/Protocol/PackageFactoryBase.h>
#include <Spark/Network/Protocol/ProtocolSubscriber.h>
#include <Spark/Network/Io/IoBase.h>
#include <Spark/Network/Io/IoThread.h>
#include <map>

namespace Spark::Network
{
class NETWORK_EXPORTS Protocol : public IoSubscriber
{
public:
    Protocol(ProtocolTypeType protocolType, ServerTypeType serverType, IoModelType ioModel, int milliSeconds, PackageFactoryBase* packageFactory);
    ~Protocol();
    void Subscribe(ProtocolSubscriber* subscriber);
    void UnSubscribe();
    void RegisterFront(const char* address);
    void SetIoThread(IoThread* ioThread);
    void SetTimeOut(int milliSeconds);
    bool Start();
    void Stop();
    void Join();
    virtual bool Init();
    IoBase* GetIo();
    IoThread* GetIoThread();

    void DisConnect(SessionIdType sessionId);
    virtual bool Send(Package* package);

    virtual void OnConnect(SessionIdType sessionId, const char* ip, int port) override;
    virtual void OnDisConnect(SessionIdType sessionId, const char* ip, int port) override;
    virtual void OnRecv(SessionIdType sessionId, Buffer<BufferSize>* buffer) override;

protected:
    ProtocolTypeType protocolType_;
    ServerTypeType serverType_;
    IoModelType ioModel_;
    int milliSeconds_;
    IoBase* ioBase_;
    IoThread* ioThread_;
    PackageFactoryBase* packageFactory_;
    ProtocolSubscriber* subscriber_;
    std::map<SessionIdType, PackageReader*> sessionPackageReaders_;
};
}
