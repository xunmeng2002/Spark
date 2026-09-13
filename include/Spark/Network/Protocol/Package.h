#pragma once
#include <Spark/Network/NetworkExport.h>
#include <Spark/Types.h>
#include <Spark/Network/Protocol/Head.h>
#include <Spark/Network/Protocol/ProtocolVersion.h>
#include <stdint.h>


#define MaxPackageSize 2 * 64 * 1024

namespace spark::network
{
static_assert(MaxPackageSize >= MaxFrameSize, "读侧缓冲必须容纳一帧上限，否则对端发来的最大帧无法解析");
class NETWORK_EXPORTS Package
{
public:
	Package()
		:SessionID(0), IPAddress{ 0 }
	{
	}
	virtual ~Package();
	virtual void Deallocate() = 0;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum);
	virtual int MakePackage(ProtocolTypeType protocolType, char* buff, int size);
	virtual int ToStepStream(char* buff, int size) const = 0;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) = 0;
	virtual int ToXtpStream(char* buff, int size) const = 0;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) = 0;
	virtual const char* GetDebugString() const = 0;

public:
	SessionIDType SessionID;
	IPAddressType IPAddress;

	HeadField Head;
	TailField Tail;
};
}
