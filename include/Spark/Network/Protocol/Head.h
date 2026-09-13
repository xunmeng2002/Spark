#pragma once
#include <Spark/Types.h>

namespace spark::network
{
//报文头
class HeadField
{
public:
	static constexpr UInt16Type FieldID = 0x0001;
	Int32Type Magic;		//魔术字，固定为ProtocolMagicValue
	Int32Type MsgSeqNum;		//请求编号
	UInt16Type PackageID;		//报文代码
	UInt16Type BodyLen;		//报文长度
	UInt16Type Version;		//协议版本，固定为ProtocolVersionValue
	BoolType MessageChain;		//报文链标记
	BoolType Reserved;		//保留字段，必须为false
};
//报文尾
class TailField
{
public:
	static constexpr UInt16Type FieldID = 0x0002;
	Int32Type CheckSum;		//CRC32C校验和，覆盖报文头与包体
};
}