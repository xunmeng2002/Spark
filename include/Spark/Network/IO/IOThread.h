#pragma once
#include <Spark/Network/NetworkExport.h>
#include <Spark/Types.h>
#include <Spark/Core/Thread/ThreadBase.h>
#include <Spark/Network/IO/IOBase.h>
#include <Spark/TemplateLib/Buffer/Buffer.h>
#include <Spark/Network/IO/Connect.h>
#include <string>
#include <mutex>
#include <list>
#include <map>


namespace spark::network
{
class NETWORK_EXPORTS IOThread : public spark::core::ThreadBase
{
public:
	IOThread(const char* threadName);
	~IOThread();
	void SetIO(IOBase* io);

protected:
	virtual void Run() override;
	virtual void ThreadExit() override;

private:
	IOBase* m_IO;
};
}
