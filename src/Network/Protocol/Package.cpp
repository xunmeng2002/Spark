#include <Spark/Network/Protocol/Package.h>
#include <Spark/Network/Protocol/StepUtility.h>
#include <Spark/Network/Protocol/ProtocolUtility.h>
#include <Spark/Network/Protocol/ProtocolVersion.h>
#include <Spark/Core/Logger/Logger.h>
#include <string.h>


using namespace spark::core;

namespace spark::network
{
namespace
{
//报文头里由协议自身约定的字段，与业务无关，所以统一在这里填，避免两条分支各写一遍
void FillProtocolHead(HeadField& head)
{
	head.Magic = ProtocolMagicValue;
	head.Version = ProtocolVersionValue;
	head.Reserved = false;
}
}

Package::~Package() {
	SessionID = 0;
	memset(IPAddress, 0, sizeof(IPAddressType));
	memset(&Head, 0, sizeof(Head));
	memset(&Tail, 0, sizeof(Tail));
}
void Package::Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum)
{
	SessionID = sessionID;
	Head.MsgSeqNum = msgSeqNum;
	Head.MessageChain = messageChain;
}
int Package::MakePackage(ProtocolTypeType protocolType, char* buff, int size)
{
	FillProtocolHead(Head);
	if (protocolType == ProtocolTypeType::Xtp)
	{
		char* data = buff + sizeof(Head);
		int bodyCapacity = size - static_cast<int>(sizeof(Head)) - static_cast<int>(sizeof(Tail));
		int bodyLen = ToXtpStream(data, bodyCapacity);
		if (bodyLen < 0 || bodyLen > bodyCapacity || bodyLen > static_cast<int>(MaxFrameBodyLen))
		{
			WriteLog(LogLevel::Error, "Xtp Body Length Invalid. BodyLen:%d, BodyCapacity:%d, MaxFrameBodyLen:%u",
				bodyLen, bodyCapacity, MaxFrameBodyLen);
			return 0;
		}
		Head.BodyLen = static_cast<UShortType>(bodyLen);
		memcpy(buff, &Head, sizeof(Head));
		Tail.CheckSum = static_cast<IntType>(CalculateCrc32c((const unsigned char*)buff, sizeof(Head) + bodyLen));
		memcpy(data + bodyLen, &Tail, sizeof(Tail));

		return sizeof(Head) + bodyLen + sizeof(Tail);
	}
	else if (protocolType == ProtocolTypeType::Step)
	{
		//包体长度写在报文头里，而包头长度取决于包体长度字段的宽度，
		//所以先用占位长度量出包头长度，写完包体后再回填并校验头长未变
		Head.BodyLen = 0;
		int headLen = StepUtility::HeadToStream(&Head, buff, size);
		if (headLen <= 0 || headLen + (int)StepTailLen >= size)
		{
			WriteLog(LogLevel::Error, "Step Head To Stream Failed. HeadLen:%d, Size:%d", headLen, size);
			return 0;
		}
		int bodyCapacity = size - headLen - (int)StepTailLen;
		int bodyLen = ToStepStream(buff + headLen, bodyCapacity);
		if (bodyLen < 0 || bodyLen > bodyCapacity || bodyLen > static_cast<int>(MaxFrameBodyLen))
		{
			WriteLog(LogLevel::Error, "Step Body Length Invalid. BodyLen:%d, BodyCapacity:%d, MaxFrameBodyLen:%u",
				bodyLen, bodyCapacity, MaxFrameBodyLen);
			return 0;
		}
		Head.BodyLen = static_cast<UShortType>(bodyLen);
		if (StepUtility::HeadToStream(&Head, buff, size) != headLen)
		{
			WriteLog(LogLevel::Error, "Step Head Length Changed After BodyLen Filled. HeadLen:%d, BodyLen:%d", headLen, Head.BodyLen);
			return 0;
		}
		Tail.CheckSum = static_cast<IntType>(CalculateCrc32c((const unsigned char*)buff, headLen + bodyLen));
		StepUtility::TailToStream(&Tail, buff + headLen + bodyLen, StepTailLen);
		return headLen + bodyLen + StepTailLen;
	}
	return 0;
}
}
