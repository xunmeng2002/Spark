#include <Spark/Network/Protocol/PackageReader.h>
#include <Spark/Network/Protocol/ProtocolUtility.h>
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
void PackageReader::Shift(unsigned int len)
{
	m_Data += len;
	m_Length -= len;
}
unsigned int PackageReader::Append(char* data, unsigned int len)
{
	len = std::min(len, TailSize());
	memcpy(Tail(), data, len);
	m_Length += len;
	return len;
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
	if (m_Length < sizeof(HeadField))
		return true;
	memcpy(&m_Head, m_Data, sizeof(HeadField));
	if (m_Length < (m_Head.BodyLen + sizeof(HeadField) + sizeof(TailField)))
	{
		return true;
	}
	memcpy(&m_Tail, m_Data + sizeof(HeadField) + m_Head.BodyLen, sizeof(m_Tail));
	auto checkSum = CalculateSum((unsigned char*)m_Data, m_Head.BodyLen + sizeof(HeadField));
	if (checkSum != m_Tail.CheckSum)
	{
		WriteLog(LogLevel::Error, "CheckSum not Match. Tail.CheckSum:%d, CalculateSum:%d", m_Tail.CheckSum, checkSum);
		Reset();
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
	return ret;
}
bool PackageReader::ParseStepPackage(Package*& package)
{
	if (m_Length < StepHeaderLen)
	{
		return true;
	}
	int packageStartIndex = 0;
	if (!StepUtility::GetPackageStart(m_Data, 0, m_Length, packageStartIndex))
	{
		WriteLog(LogLevel::Warning, "Cannot Find PackageStart Mark.");
		return false;
	}
	if (m_Length - packageStartIndex < StepHeaderLen)
	{
		return true;
	}
	::memset(&m_Head, 0, sizeof(m_Head));
	if (!StepUtility::HeadFromStream(m_Data, packageStartIndex, packageStartIndex + StepHeaderLen, &m_Head))
	{
		WriteLog(LogLevel::Warning, "Parse Head Failed.");
		return false;
	}
	if ((m_Length - packageStartIndex) < (StepHeaderLen + m_Head.BodyLen + StepTailLen))
	{
		return true;
	}
	if (!StepUtility::TailFromStream(m_Data, packageStartIndex + StepHeaderLen + m_Head.BodyLen, packageStartIndex + StepHeaderLen + m_Head.BodyLen + StepTailLen, &m_Tail))
	{
		WriteLog(LogLevel::Warning, "Parse Tail Failed.");
		return false;
	}
	auto checkSum = CalculateSum((unsigned char*)m_Data + packageStartIndex, StepHeaderLen + m_Head.BodyLen);
	if (checkSum != m_Tail.CheckSum)
	{
		WriteLog(LogLevel::Warning, "CheckSum not Match. Tail.CheckSum:%d, CalculateSum:%d", m_Tail.CheckSum, checkSum);
		Reset();
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
	auto ret = package->FromStepStream(m_Data, packageStartIndex + StepHeaderLen, packageStartIndex + StepHeaderLen + m_Head.BodyLen);
	PopFront(packageStartIndex + StepHeaderLen + m_Head.BodyLen + StepTailLen);
	return ret;
}
}

