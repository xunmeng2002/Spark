#pragma once
#include <Spark/Types.h>
#include <Spark/Network/Protocol/Head.h>
#include <bit>
#include <limits>

namespace spark::network
{
//协议版本。报文头结构或字段语义变化时必须同步递增，否则两端会按不同的格式解析同一串字节
constexpr UShortType ProtocolVersionValue = 2;

//魔术字。线上小端字节序为 53 50 4B 32，即 ASCII 的 SPK2
constexpr IntType ProtocolMagicValue = 0x324B5053;

//Step 协议中魔术字的文本形态，与 ProtocolMagicValue 是同一串字节的两种写法
constexpr char ProtocolMagicText[] = "SPK2";

//一帧的总长上限（报文头 + 包体 + 报文尾），收发两侧的缓冲都必须容纳它
constexpr unsigned int MaxFrameSize = 64 * 1024;

//包体上限。BodyLen 是 16 位字段，一帧又要塞得进帧上限，取两者中小的那个
constexpr unsigned int MaxFrameBodyLen = MaxFrameSize - sizeof(HeadField) - sizeof(TailField);

//报文头与报文尾是直接把结构体 memcpy 上线的，布局一变线上格式就变，所以在这里把尺寸钉死
static_assert(sizeof(HeadField) == 16,
	"HeadField 布局变化会改变线上格式，必须同步升级 ProtocolVersionValue");
static_assert(sizeof(TailField) == 4, "TailField 布局变化会改变线上格式");
static_assert(sizeof(HeadField) + sizeof(TailField) == 20, "单帧固定开销必须是 20 字节");
static_assert(MaxFrameBodyLen <= std::numeric_limits<UShortType>::max(),
	"包体上限必须能写进 BodyLen（UShort），否则包体长度会被静默截断");
static_assert(std::endian::native == std::endian::little, "线协议依赖小端字节序");

static_assert(ProtocolMagicValue != 0, "魔术字不能为 0，否则无法与全零缓冲区分");
static_assert(ProtocolMagicText[0] == static_cast<char>(ProtocolMagicValue & 0xFF)
	&& ProtocolMagicText[1] == static_cast<char>((ProtocolMagicValue >> 8) & 0xFF)
	&& ProtocolMagicText[2] == static_cast<char>((ProtocolMagicValue >> 16) & 0xFF)
	&& ProtocolMagicText[3] == static_cast<char>((ProtocolMagicValue >> 24) & 0xFF),
	"ProtocolMagicText 必须与 ProtocolMagicValue 的字节镜像一致");
}
