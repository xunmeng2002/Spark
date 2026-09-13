#include <Spark/Network/Protocol/PackageReader.h>
#include <Spark/Network/Protocol/ProtocolUtility.h>
#include <Spark/Network/Protocol/ProtocolVersion.h>
#include <Spark/Network/Protocol/StepUtility.h>
#include <Spark/Core/Logger/Logger.h>
#include <Spark/TemplateLib/ObjectPool/ObjectPool.h>
#include <stdio.h>
#include <cstring>
#include <algorithm>

using namespace spark::core;
namespace spark::network
{
PackageReader::PackageReader(ProtocolTypeType protocolType, PackageFactoryBase* packageFactory, SessionIDType sessionID, const char* ipAddress)
	:m_Buff{ 0 }
{
	m_Data = m_Buff;
	m_Length = 0;
	m_DiscardLength = 0;

	m_ProtocolType = protocolType;
	m_PackageFactory = packageFactory;
	m_SessionID = sessionID;
	snprintf(m_IPAddress, sizeof(IPAddressType), "%s", ipAddress);
	memset(&(m_Head), 0, sizeof(HeadField));
	memset(&(m_Tail), 0, sizeof(TailField));
}
PackageReader::~PackageReader()
{
	m_Data = nullptr;
	m_Length = 0;
}
PackageReader* PackageReader::Allocate(ProtocolTypeType protocolType, PackageFactoryBase* packageFactory, SessionIDType sessionID, const char* ipAddress)
{
	return ObjectPool<PackageReader>::GetInstance().Allocate(protocolType, packageFactory, sessionID, ipAddress);
}
void PackageReader::Deallocate()
{
	ObjectPool<PackageReader>::GetInstance().Deallocate(this);
}
void PackageReader::Reset()
{
	m_Data = m_Buff;
	m_Length = 0;
	m_DiscardLength = 0;
}
void PackageReader::PopFront(unsigned int len)
{
	len = std::min(len, m_Length);
	m_Length = m_Length - len;
	if (m_Length > 0)
	{
		std::memmove(m_Buff, m_Data + len, m_Length);
	}
	m_Data = m_Buff;
}
char* PackageReader::Data()
{
	return m_Data;
}
int PackageReader::Length()
{
	return m_Length;
}
char* PackageReader::Tail()
{
	return m_Data + m_Length;
}
unsigned int PackageReader::TailSize()
{
	return unsigned((m_Buff + MaxPackageSize) - (m_Data + m_Length));
}
unsigned int PackageReader::Append(char* data, unsigned int len)
{
	len = std::min(len, TailSize());
	memcpy(Tail(), data, len);
	m_Length += len;
	return len;
}
void PackageReader::DiscardFront(unsigned int len)
{
	len = std::min(len, m_Length);
	m_DiscardLength += len;
	PopFront(len);
}
bool PackageReader::IsBodyLenWithinFrameLimit() const
{
	if (m_Head.BodyLen <= MaxFrameBodyLen)
	{
		return true;
	}
	WriteLog(LogLevel::Warning, "Body Length Exceeds Frame Limit. BodyLen:%u, MaxFrameBodyLen:%u, SessionID:%lld, IP:%s",
		static_cast<unsigned int>(m_Head.BodyLen), MaxFrameBodyLen, m_SessionID, m_IPAddress);
	return false;
}
PackageReader::AlignResult PackageReader::AlignToAnchor(const char* anchor, unsigned int anchorLength)
{
	unsigned int offset = 0;
	if (FindBytes(m_Data, m_Length, anchor, anchorLength, offset))
	{
		if (offset > 0)
		{
			DiscardFront(offset);
		}
		return AlignResult::Aligned;
	}
	//锚点可能跨收包边界，所以保留末尾不足一个锚点的字节，其余都是无意义的前缀
	unsigned int discardLength = m_Length - std::min(m_Length, anchorLength - 1);
	if (discardLength > 0)
	{
		if (m_DiscardLength == 0)
		{
			//首次丢弃说明对端说的可能不是本协议，把这段字节的开头记下来便于定位
			unsigned int probe = 0;
			::memcpy(&probe, m_Data, std::min(m_Length, unsigned(sizeof(probe))));
			WriteLog(LogLevel::Warning, "Package Start Not Found, Resync. SessionID:%lld, IP:%s, HeadValue:0x%08X, BufferLen:%u",
				m_SessionID, m_IPAddress, probe, m_Length);
		}
		DiscardFront(discardLength);
	}
	if (m_DiscardLength > MaxPackageSize)
	{
		WriteLog(LogLevel::Error, "Garbage Stream Detected, DisConnect. SessionID:%lld, IP:%s, DiscardedBytes:%u",
			m_SessionID, m_IPAddress, m_DiscardLength);
		return AlignResult::GarbageStream;
	}
	return AlignResult::NeedMoreData;
}
bool PackageReader::ParsePackage(Package*& package)
{
	if (m_ProtocolType == ProtocolTypeType::Xtp)
	{
		return ParseXtpPackage(package);
	}
	else if (m_ProtocolType == ProtocolTypeType::Step)
	{
		return ParseStepPackage(package);
	}
	return false;
}
bool PackageReader::ParseXtpPackage(Package*& package)
{
	while (true)
	{
		AlignResult alignResult = AlignToAnchor((const char*)&ProtocolMagicValue, sizeof(ProtocolMagicValue));
		if (alignResult == AlignResult::GarbageStream)
		{
			return false;
		}
		if (alignResult != AlignResult::Aligned || m_Length < sizeof(HeadField))
		{
			return true;
		}
		memcpy(&m_Head, m_Data, sizeof(HeadField));
		//版本先于长度校验：版本不符时 BodyLen 的语义本身就不可信
		if (m_Head.Version != ProtocolVersionValue)
		{
			WriteLog(LogLevel::Error, "Protocol Version Not Match. RemoteVersion:%u, LocalVersion:%u, SessionID:%lld, IP:%s",
				static_cast<unsigned int>(m_Head.Version), static_cast<unsigned int>(ProtocolVersionValue),
				m_SessionID, m_IPAddress);
			return false;
		}
		if (!IsBodyLenWithinFrameLimit())
		{
			DiscardFront(1);
			continue;
		}
		if (m_Length < (sizeof(HeadField) + m_Head.BodyLen + sizeof(TailField)))
		{
			return true;
		}
		memcpy(&m_Tail, m_Data + sizeof(HeadField) + m_Head.BodyLen, sizeof(m_Tail));
		auto checkSum = CalculateCrc32c((const unsigned char*)m_Data, sizeof(HeadField) + m_Head.BodyLen);
		if (checkSum != static_cast<unsigned int>(m_Tail.CheckSum))
		{
			WriteLog(LogLevel::Error, "CheckSum not Match. Tail.CheckSum:0x%08X, CalculateCrc32c:0x%08X", m_Tail.CheckSum, checkSum);
			//只丢一个字节，让下一次扫描重新定位到真正的魔术字，而不是清空整段缓冲
			DiscardFront(1);
			continue;
		}
		if (!m_PackageFactory->IsInboundPackageAccepted(m_Head.PackageID))
		{
			WriteLog(LogLevel::Error, "Inbound Package Not Accepted. PackageID:%d, SessionID:%lld, IP:%s", m_Head.PackageID, m_SessionID, m_IPAddress);
			return false;
		}
		package = m_PackageFactory->CreatePackage(m_Head.PackageID);
		if (package == nullptr)
		{
			WriteLog(LogLevel::Warning, "CreatePackage Failed. ProtocolType:%d, PackageID:%d", m_ProtocolType, m_Head.PackageID);
			return false;
		}

		package->SessionID = m_SessionID;
		snprintf(package->IPAddress, sizeof(IPAddressType), "%s", m_IPAddress);
		package->Head = m_Head;
		package->Tail = m_Tail;
		auto ret = package->FromXtpStream(m_Data, sizeof(HeadField), sizeof(HeadField) + m_Head.BodyLen);
		PopFront(sizeof(HeadField) + m_Head.BodyLen + sizeof(TailField));
		if (!ret)
		{
			WriteLog(LogLevel::Warning, "FromXtpStream Failed. ProtocolType:%d, PackageID:%d, BodyLen:%d", m_ProtocolType, m_Head.PackageID, m_Head.BodyLen);
			package->Deallocate();
			package = nullptr;
			return false;
		}
		m_DiscardLength = 0;
		return true;
	}
}
bool PackageReader::ParseStepPackage(Package*& package)
{
	const std::string& anchor = StepUtility::GetPackageStartAnchor();
	unsigned int anchorLength = unsigned(anchor.size());
	while (true)
	{
		AlignResult alignResult = AlignToAnchor(anchor.c_str(), anchorLength);
		if (alignResult == AlignResult::GarbageStream)
		{
			return false;
		}
		if (alignResult != AlignResult::Aligned)
		{
			return true;
		}
		int headEndIndex = 0;
		::memset(&m_Head, 0, sizeof(m_Head));
		if (!StepUtility::HeadFromStream(m_Data, 0, m_Length, &m_Head, headEndIndex))
		{
			if (m_Length <= StepMaxHeaderLen)
			{
				return true;
			}
			WriteLog(LogLevel::Warning, "Parse Head Failed. SessionID:%lld, IP:%s, BufferLen:%u", m_SessionID, m_IPAddress, m_Length);
			DiscardFront(1);
			continue;
		}
		if (m_Head.Version != ProtocolVersionValue)
		{
			WriteLog(LogLevel::Error, "Protocol Version Not Match. RemoteVersion:%u, LocalVersion:%u, SessionID:%lld, IP:%s",
				static_cast<unsigned int>(m_Head.Version), static_cast<unsigned int>(ProtocolVersionValue),
				m_SessionID, m_IPAddress);
			return false;
		}
		if (!IsBodyLenWithinFrameLimit())
		{
			DiscardFront(1);
			continue;
		}
		int tailIndex = headEndIndex + m_Head.BodyLen;
		if (m_Length < unsigned(tailIndex + StepTailLen))
		{
			return true;
		}
		if (!StepUtility::TailFromStream(m_Data, tailIndex, tailIndex + StepTailLen, &m_Tail))
		{
			WriteLog(LogLevel::Warning, "Parse Tail Failed. SessionID:%lld, IP:%s", m_SessionID, m_IPAddress);
			DiscardFront(1);
			continue;
		}
		auto checkSum = CalculateCrc32c((const unsigned char*)m_Data, tailIndex);
		if (checkSum != static_cast<unsigned int>(m_Tail.CheckSum))
		{
			WriteLog(LogLevel::Warning, "CheckSum not Match. Tail.CheckSum:0x%08X, CalculateCrc32c:0x%08X", m_Tail.CheckSum, checkSum);
			DiscardFront(1);
			continue;
		}
		if (!m_PackageFactory->IsInboundPackageAccepted(m_Head.PackageID))
		{
			WriteLog(LogLevel::Error, "Inbound Package Not Accepted. PackageID:%d, SessionID:%lld, IP:%s", m_Head.PackageID, m_SessionID, m_IPAddress);
			return false;
		}
		package = m_PackageFactory->CreatePackage(m_Head.PackageID);
		if (package == nullptr)
		{
			WriteLog(LogLevel::Warning, "CreatePackage Failed. ProtocolType:%d, PackageID:%d", m_ProtocolType, m_Head.PackageID);
			return false;
		}

		package->SessionID = m_SessionID;
		snprintf(package->IPAddress, sizeof(IPAddressType), "%s", m_IPAddress);
		memcpy(&package->Head, &m_Head, sizeof(HeadField));
		memcpy(&package->Tail, &m_Tail, sizeof(TailField));
		auto ret = package->FromStepStream(m_Data, headEndIndex, tailIndex);
		PopFront(tailIndex + StepTailLen);
		if (!ret)
		{
			WriteLog(LogLevel::Warning, "FromStepStream Failed. ProtocolType:%d, PackageID:%d, BodyLen:%d", m_ProtocolType, m_Head.PackageID, m_Head.BodyLen);
			package->Deallocate();
			package = nullptr;
			return false;
		}
		m_DiscardLength = 0;
		return true;
	}
}
}
