#pragma once
#include <Spark/Types.h>

namespace spark::network
{
//报文头
class HeadField
{
public:
	static constexpr UShortType FieldID = 0x0001;
	IntType Magic;		//魔术字，固定为ProtocolMagicValue
	IntType MsgSeqNum;		//请求编号
	UShortType PackageID;		//报文代码
	UShortType BodyLen;		//报文长度
	UShortType Version;		//协议版本，固定为ProtocolVersionValue
	BoolType MessageChain;		//报文链标记
	BoolType Reserved;		//保留字段，必须为false
};
//报文尾
class TailField
{
public:
	static constexpr UShortType FieldID = 0x0002;
	IntType CheckSum;		//CRC32C校验和，覆盖报文头与包体
};
}