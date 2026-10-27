// 本文件由 ../Templates/Cpp/Spark/Network/Protocol/Items.h.tpl 生成；请勿手改，改动请改模板后重跑 pumpall.py
#pragma once
#include <Spark/Types.h>

namespace Spark::Network
{
class Items
{
public:
	static constexpr UInt16Type Magic{0x0000};
	static constexpr UInt16Type PackageId{0x0001};
	static constexpr UInt16Type BodyLen{0x0002};
	static constexpr UInt16Type MessageChain{0x0003};
	static constexpr UInt16Type MsgSeqNum{0x0004};
	static constexpr UInt16Type CheckSum{0x0005};
	static constexpr UInt16Type FieldStart{0x0006};
	static constexpr UInt16Type FieldEnd{0x0007};
	static constexpr UInt16Type Version{0x0008};
};
}
