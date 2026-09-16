#pragma once
#include <Spark/Network/NetworkExport.h>
#include <Spark/Network/Protocol/Package.h>
#include <Spark/Network/Protocol/PackageFactoryBase.h>


namespace Spark::Network
{
class NETWORK_EXPORTS PackageReader
{
public:
	PackageReader(ProtocolTypeType protocolType, PackageFactoryBase* packageFactory, SessionIdType sessionId, const char* ipAddress);
	~PackageReader();
	static PackageReader* Allocate(ProtocolTypeType protocolType, PackageFactoryBase* packageFactory, SessionIdType sessionId, const char* ipAddress);
	void Deallocate();

	void Reset();
	void PopFront(unsigned int len);
	char* Data();
	int Length();
	char* Tail();
	unsigned int TailSize();
	unsigned int Append(char* data, unsigned  int len);

	bool ParsePackage(Package*& package);

protected:
	bool ParseXtpPackage(Package*& package);
	bool ParseStepPackage(Package*& package);

private:
	enum class AlignResult
	{
		//已对齐，data_ 起头就是锚点
		Aligned,
		//缓冲里还凑不出完整锚点，已丢弃无意义的字节，等下一次收包
		NeedMoreData,
		//丢弃的字节数超出上限，判定为垃圾流，应断开连接
		GarbageStream,
	};
	AlignResult AlignToAnchor(const char* anchor, unsigned int anchorLength);
	void DiscardFront(unsigned int len);
	bool IsBodyLenWithinFrameLimit() const;

protected:
	ProtocolTypeType protocolType_;
    PackageFactoryBase* packageFactory_;
	SessionIdType sessionId_;
	IpAddressType ipAddress_;
	HeadField head_;
	TailField tail_;

	char buff_[MaxPackageSize * 2];
	char* data_;
	unsigned int length_;
	unsigned int discardLength_;
};
}
