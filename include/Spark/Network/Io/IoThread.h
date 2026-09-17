#pragma once
#include <Spark/Network/NetworkExport.h>
#include <Spark/Types.h>
#include <Spark/Core/Thread/ThreadBase.h>
#include <Spark/Network/Io/IoBase.h>
#include <Spark/TemplateLib/Buffer/Buffer.h>
#include <Spark/Network/Io/Connect.h>
#include <string>
#include <mutex>
#include <list>
#include <map>

namespace Spark::Network
{
class NETWORK_EXPORTS IoThread : public Spark::Core::ThreadBase
{
public:
    IoThread(const char* threadName);
    ~IoThread();
    void SetIo(IoBase* io);

protected:
    virtual void Run() override;
    virtual void ThreadExit() override;

private:
    IoBase* io_;
};
}
