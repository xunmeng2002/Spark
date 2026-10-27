// 本文件由 ../Templates/Cpp/Spark/EnumString.h.tpl 生成；请勿手改，改动请改模板后重跑 pumpall.py
#pragma once
#include <Spark/Types.h>


namespace Spark
{
inline const char* GetConnectStatusString(ConnectStatusType value)
{
	switch (value)
	{
		case ConnectStatusType::UnConnected:
			return "UnConnected";
		case ConnectStatusType::Connecting:
			return "Connecting";
		case ConnectStatusType::Accepted:
			return "Accepted";
		case ConnectStatusType::Rejected:
			return "Rejected";
		case ConnectStatusType::Connected:
			return "Connected";
		case ConnectStatusType::DisConnected:
			return "DisConnected";
		default:
			return "Unknown";
	}
	return "";
}

inline const char* GetIoModelString(IoModelType value)
{
	switch (value)
	{
		case IoModelType::Select:
			return "Select";
		case IoModelType::Epoll:
			return "Epoll";
		case IoModelType::Iocp:
			return "Iocp";
		default:
			return "Unknown";
	}
	return "";
}

inline const char* GetIoTypeString(IoTypeType value)
{
	switch (value)
	{
		case IoTypeType::Tcp:
			return "Tcp";
		case IoTypeType::Udp:
			return "Udp";
		case IoTypeType::Shm:
			return "Shm";
		default:
			return "Unknown";
	}
	return "";
}

inline const char* GetProtocolTypeString(ProtocolTypeType value)
{
	switch (value)
	{
		case ProtocolTypeType::Xtp:
			return "Xtp";
		case ProtocolTypeType::Step:
			return "Step";
		default:
			return "Unknown";
	}
	return "";
}

inline const char* GetServerTypeString(ServerTypeType value)
{
	switch (value)
	{
		case ServerTypeType::Client:
			return "Client";
		case ServerTypeType::Server:
			return "Server";
		default:
			return "Unknown";
	}
	return "";
}

}
