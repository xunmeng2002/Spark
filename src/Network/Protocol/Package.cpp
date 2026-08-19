#include <Spark/Network/Protocol/Package.h>
#include <Spark/Network/Protocol/StepUtility.h>
#include <Spark/Network/Protocol/ProtocolUtility.h>
#include <string.h>


namespace spark::network
{
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
	if (protocolType == ProtocolTypeType::Xtp)
	{
		char* data = buff + sizeof(Head);
		Head.BodyLen = ToXtpStream(data, size - sizeof(Head) - sizeof(Tail));
		memcpy(buff, &Head, sizeof(Head));
		Tail.CheckSum = CalculateSum((unsigned char*)buff, sizeof(Head) + Head.BodyLen);
		memcpy(data + Head.BodyLen, &Tail, sizeof(Tail));

		return sizeof(Head) + Head.BodyLen + sizeof(Tail);
	}
	else if (protocolType == ProtocolTypeType::Step)
	{
		Head.BodyLen = ToStepStream(buff + StepHeaderLen, size - StepHeaderLen - StepTailLen);
		StepUtility::HeadToStream(&Head, buff, StepHeaderLen);
		Tail.CheckSum = CalculateSum((unsigned char*)buff, StepHeaderLen + Head.BodyLen);
		StepUtility::TailToStream(&Tail, buff + StepHeaderLen + Head.BodyLen, StepTailLen);
		return StepHeaderLen + Head.BodyLen + StepTailLen;
	}
	return 0;
}
}

