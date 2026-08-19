#pragma once
#include <Spark/Types.h>

namespace spark::network
{
//报文头
class HeadField
{
public:
	static constexpr UShortType FieldID = 0x0001;
	UShortType PackageID;		//报文代码
	UShortType BodyLen;		//报文长度
	BoolType MessageChain;		//报文链标记
	IntType MsgSeqNum;		//请求编号
};
//报文尾
class TailField
{
public:
	static constexpr UShortType FieldID = 0x0002;
	UShortType CheckSum;		//校验和tag = 6固定长度3位
};
}