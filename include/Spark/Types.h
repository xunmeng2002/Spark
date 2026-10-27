// 本文件由 ../Templates/Cpp/Spark/Types.h.tpl 生成；请勿手改，改动请改模板后重跑 pumpall.py
#pragma once
#include <stdint.h>

namespace Spark
{
//布尔类型
using BoolType = bool;


//无符号8位整型类型
using UInt8Type = uint8_t;


//8位整型类型
using Int8Type = int8_t;


//无符号16位整型类型
using UInt16Type = uint16_t;


//16位整型类型
using Int16Type = int16_t;


//无符号32位整型类型
using UInt32Type = uint32_t;


//32位整型类型
using Int32Type = int32_t;


//无符号64位整型类型
using UInt64Type = unsigned long long;


	//64位整型类型
using Int64Type = long long;

	//会话代码类型
using SessionIdType = long long;


//双精度浮点类型
using DoubleType = double;


//日期类型
using DateType = char[16];

//Ip地址类型
using IpAddressType = char[32];

//时间类型
using TimeType = char[16];


//连接状态
enum class ConnectStatusType : int32_t
{
	//未连接
	UnConnected = 0,
	//正在链接
	Connecting = 1,
	//已接受
	Accepted = 2,
	//已拒绝
	Rejected = 3,
	//已连接
	Connected = 4,
	//已断开连接
	DisConnected = 5,
};

//IO模型
enum class IoModelType : int32_t
{
	//Select
	Select = 0,
	//Epoll
	Epoll = 1,
	//Iocp
	Iocp = 2,
};

//IO类型
enum class IoTypeType : int32_t
{
	//Tcp
	Tcp = 0,
	//Udp
	Udp = 1,
	//共享内存
	Shm = 2,
};

//协议类型
enum class ProtocolTypeType : int32_t
{
	//Xtp协议
	Xtp = 0,
	//Step协议
	Step = 1,
};

//服务类别
enum class ServerTypeType : int32_t
{
	//客户端
	Client = 0,
	//服务端
	Server = 1,
};



//线协议是按字段宽度直接 memcpy 上线的：宽度一变线上格式就静默错位，所以在这里钉死
//64 位保留 long long 而非 int64_t：Linux 上 int64_t 是 long，会让全仓 %lld 变成格式不匹配
static_assert(sizeof(long long) == 8, "Int64 别名用 long long，线上必须占 8 字节");
//无符号侧同理：保留 unsigned long long 而非 uint64_t，否则 Linux 上 %llu 同样不匹配
static_assert(sizeof(unsigned long long) == 8, "UInt64 别名用 unsigned long long，线上必须占 8 字节");
static_assert(sizeof(double) == 8, "Double 别名用 double，线上必须占 8 字节");
static_assert(sizeof(bool) == 1, "Bool 别名用 bool，线上必须占 1 字节");
}
