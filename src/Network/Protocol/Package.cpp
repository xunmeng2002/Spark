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
	//MsgSeqNum 是无符号计数器，这里的 int 形参按模 2^32 转换（负值落到 0xFFFFFFFF 附近）。
	//形参类型本身仍是有符号 int（改它要动公开签名），转换写成显式的是为了让这层取模是有意的
	SessionID = sessionID;
	Head.MsgSeqNum = static_cast<UInt32Type>(msgSeqNum);
	Head.MessageChain = messageChain;
}
int Package::MakePackage(ProtocolTypeType protocolType, char* buff, int size)
{
	FillProtocolHead(Head);
	//这道闸门必须排在任何指针与容量运算之前：ToXtpStream是公开纯虚函数，仓外实现不保证先比容量再写，所以拿一个负的bodyCapacity进去等于把越界的可能交给下游
	if (buff == nullptr)
	{
		WriteLog(LogLevel::Error, "Package Buffer Is Null.");
		return 0;
	}
	if (size < FixedFrameOverhead)
	{
		WriteLog(LogLevel::Error, "Package Buffer Too Small. Size:%d, Needed:%d", size, FixedFrameOverhead);
		return 0;
	}
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
		Head.BodyLen = static_cast<UInt16Type>(bodyLen);
		memcpy(buff, &Head, sizeof(Head));
		Tail.CheckSum = CalculateCrc32c(reinterpret_cast<const unsigned char*>(buff), sizeof(Head) + bodyLen);
		memcpy(data + bodyLen, &Tail, sizeof(Tail));

		return sizeof(Head) + bodyLen + sizeof(Tail);
	}
	else if (protocolType == ProtocolTypeType::Step)
	{
		//包头是定长的，包体长度可以在写头之前就算出来，所以只写一次头
		if (size < static_cast<int>(StepHeadLen) + static_cast<int>(StepTailLen))
		{
			WriteLog(LogLevel::Error, "Step Buffer Too Small. Size:%d, Needed:%u", size, StepHeadLen + StepTailLen);
			return 0;
		}
		const int headLen = static_cast<int>(StepHeadLen);
		int bodyCapacity = size - headLen - static_cast<int>(StepTailLen);
		int bodyLen = ToStepStream(buff + headLen, bodyCapacity);
		if (bodyLen < 0 || bodyLen > bodyCapacity || bodyLen > static_cast<int>(MaxFrameBodyLen))
		{
			WriteLog(LogLevel::Error, "Step Body Length Invalid. BodyLen:%d, BodyCapacity:%d, MaxFrameBodyLen:%u",
				bodyLen, bodyCapacity, MaxFrameBodyLen);
			return 0;
		}
		Head.BodyLen = static_cast<UInt16Type>(bodyLen);
		if (StepUtility::HeadToStream(&Head, buff, headLen) != headLen)
		{
			WriteLog(LogLevel::Error, "Step Head To Stream Failed. Expected HeadLen:%d", headLen);
			return 0;
		}
		Tail.CheckSum = CalculateCrc32c(reinterpret_cast<const unsigned char*>(buff), headLen + bodyLen);
		StepUtility::TailToStream(&Tail, buff + headLen + bodyLen, static_cast<int>(StepTailLen));
		return headLen + bodyLen + static_cast<int>(StepTailLen);
	}
	return 0;
}
}
