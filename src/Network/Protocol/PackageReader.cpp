#include <Spark/Network/Protocol/PackageReader.h>
#include <Spark/Network/Protocol/ProtocolUtility.h>
#include <Spark/Network/Protocol/ProtocolVersion.h>
#include <Spark/Network/Protocol/StepUtility.h>
#include <Spark/Core/Logger/Logger.h>
#include <Spark/TemplateLib/ObjectPool/ObjectPool.h>
#include <stdio.h>
#include <cstring>
#include <algorithm>

using namespace Spark::Core;
namespace Spark::Network
{
PackageReader::PackageReader(ProtocolTypeType protocolType, PackageFactoryBase* packageFactory, SessionIdType sessionId, const char* ipAddress)
    : buff_{0}
{
    data_ = buff_;
    length_ = 0;
    discardLength_ = 0;

    protocolType_ = protocolType;
    packageFactory_ = packageFactory;
    sessionId_ = sessionId;
    snprintf(ipAddress_, sizeof(IpAddressType), "%s", ipAddress);
    memset(&(head_), 0, sizeof(HeadField));
    memset(&(tail_), 0, sizeof(TailField));
}
PackageReader::~PackageReader()
{
    data_ = nullptr;
    length_ = 0;
}
PackageReader* PackageReader::Allocate(ProtocolTypeType protocolType, PackageFactoryBase* packageFactory, SessionIdType sessionId,
                                       const char* ipAddress)
{
    return ObjectPool<PackageReader>::GetInstance().Allocate(protocolType, packageFactory, sessionId, ipAddress);
}
void PackageReader::Deallocate()
{
    ObjectPool<PackageReader>::GetInstance().Deallocate(this);
}
void PackageReader::Reset()
{
    data_ = buff_;
    length_ = 0;
    discardLength_ = 0;
}

unsigned int PackageReader::Append(const char* data, unsigned int len)
{
    len = std::min(len, TailSize());
    memcpy(Tail(), data, len);
    length_ += len;
    return len;
}
void PackageReader::PopFront(unsigned int len)
{
    len = std::min(len, length_);
    length_ = length_ - len;
    if (length_ > 0)
    {
        std::memmove(buff_, data_ + len, length_);
    }
    data_ = buff_;
}

int PackageReader::Length()
{
    return length_;
}
unsigned int PackageReader::TailSize()
{
    return unsigned((buff_ + MaxPackageSize) - (data_ + length_));
}
char* PackageReader::Data()
{
    return data_;
}
char* PackageReader::Tail()
{
    return data_ + length_;
}
bool PackageReader::ParsePackage(Package*& package)
{
    if (protocolType_ == ProtocolTypeType::Xtp)
    {
        return ParseXtpPackage(package);
    }
    else if (protocolType_ == ProtocolTypeType::Step)
    {
        return ParseStepPackage(package);
    }
    return false;
}

bool PackageReader::ParseXtpPackage(Package*& package)
{
    while (true)
    {
        AlignResult alignResult = AlignToAnchor(reinterpret_cast<const char*>(&ProtocolMagicValue), sizeof(ProtocolMagicValue));
        if (alignResult == AlignResult::GarbageStream)
        {
            return false;
        }
        if (alignResult != AlignResult::Aligned || length_ < sizeof(HeadField))
        {
            return true;
        }
        memcpy(&head_, data_, sizeof(HeadField));
        //版本先于长度校验：版本不符时 BodyLen 的语义本身就不可信
        if (head_.Version != ProtocolVersionValue)
        {
            WriteLog(LogLevel::Error, "Protocol Version Not Match. RemoteVersion:%u, LocalVersion:%u, SessionId:%lld, IP:%s",
                     static_cast<unsigned int>(head_.Version), static_cast<unsigned int>(ProtocolVersionValue), sessionId_, ipAddress_);
            return false;
        }
        if (!IsBodyLenWithinFrameLimit())
        {
            DiscardFront(1);
            continue;
        }
        if (length_ < (sizeof(HeadField) + head_.BodyLen + sizeof(TailField)))
        {
            return true;
        }
        memcpy(&tail_, data_ + sizeof(HeadField) + head_.BodyLen, sizeof(tail_));
        auto checkSum = CalculateCrc32c(reinterpret_cast<const unsigned char*>(data_), sizeof(HeadField) + head_.BodyLen);
        if (checkSum != tail_.CheckSum)
        {
            WriteLog(LogLevel::Error, "CheckSum not Match. Tail.CheckSum:0x%08X, CalculateCrc32c:0x%08X", tail_.CheckSum, checkSum);
            //只丢一个字节，让下一次扫描重新定位到真正的魔术字，而不是清空整段缓冲
            DiscardFront(1);
            continue;
        }
        if (!packageFactory_->IsInboundPackageAccepted(head_.PackageId))
        {
            WriteLog(LogLevel::Error, "Inbound Package Not Accepted. PackageId:%d, SessionId:%lld, IP:%s", head_.PackageId, sessionId_, ipAddress_);
            return false;
        }
        package = packageFactory_->CreatePackage(head_.PackageId);
        if (package == nullptr)
        {
            WriteLog(LogLevel::Warning, "CreatePackage Failed. ProtocolType:%d, PackageId:%d", protocolType_, head_.PackageId);
            return false;
        }

        package->SessionId = sessionId_;
        snprintf(package->IpAddress, sizeof(IpAddressType), "%s", ipAddress_);
        package->Head = head_;
        package->Tail = tail_;
        auto ret = package->FromXtpStream(data_, sizeof(HeadField), sizeof(HeadField) + head_.BodyLen);
        PopFront(sizeof(HeadField) + head_.BodyLen + sizeof(TailField));
        if (!ret)
        {
            WriteLog(LogLevel::Warning, "FromXtpStream Failed. ProtocolType:%d, PackageId:%d, BodyLen:%d", protocolType_, head_.PackageId,
                     head_.BodyLen);
            package->Deallocate();
            package = nullptr;
            return false;
        }
        discardLength_ = 0;
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
        ::memset(&head_, 0, sizeof(head_));
        if (!StepUtility::HeadFromStream(data_, 0, length_, &head_, headEndIndex))
        {
            if (length_ <= StepMaxHeaderLen)
            {
                return true;
            }
            WriteLog(LogLevel::Warning, "Parse Head Failed. SessionId:%lld, IP:%s, BufferLen:%u", sessionId_, ipAddress_, length_);
            DiscardFront(1);
            continue;
        }
        if (head_.Version != ProtocolVersionValue)
        {
            WriteLog(LogLevel::Error, "Protocol Version Not Match. RemoteVersion:%u, LocalVersion:%u, SessionId:%lld, IP:%s",
                     static_cast<unsigned int>(head_.Version), static_cast<unsigned int>(ProtocolVersionValue), sessionId_, ipAddress_);
            return false;
        }
        if (!IsBodyLenWithinFrameLimit())
        {
            DiscardFront(1);
            continue;
        }
        int tailIndex = headEndIndex + head_.BodyLen;
        if (length_ < unsigned(tailIndex + StepTailLen))
        {
            return true;
        }
        if (!StepUtility::TailFromStream(data_, tailIndex, tailIndex + StepTailLen, &tail_))
        {
            WriteLog(LogLevel::Warning, "Parse Tail Failed. SessionId:%lld, IP:%s", sessionId_, ipAddress_);
            DiscardFront(1);
            continue;
        }
        auto checkSum = CalculateCrc32c(reinterpret_cast<const unsigned char*>(data_), tailIndex);
        if (checkSum != tail_.CheckSum)
        {
            WriteLog(LogLevel::Warning, "CheckSum not Match. Tail.CheckSum:0x%08X, CalculateCrc32c:0x%08X", tail_.CheckSum, checkSum);
            DiscardFront(1);
            continue;
        }
        if (!packageFactory_->IsInboundPackageAccepted(head_.PackageId))
        {
            WriteLog(LogLevel::Error, "Inbound Package Not Accepted. PackageId:%d, SessionId:%lld, IP:%s", head_.PackageId, sessionId_, ipAddress_);
            return false;
        }
        package = packageFactory_->CreatePackage(head_.PackageId);
        if (package == nullptr)
        {
            WriteLog(LogLevel::Warning, "CreatePackage Failed. ProtocolType:%d, PackageId:%d", protocolType_, head_.PackageId);
            return false;
        }

        package->SessionId = sessionId_;
        snprintf(package->IpAddress, sizeof(IpAddressType), "%s", ipAddress_);
        memcpy(&package->Head, &head_, sizeof(HeadField));
        memcpy(&package->Tail, &tail_, sizeof(TailField));
        auto ret = package->FromStepStream(data_, headEndIndex, tailIndex);
        PopFront(tailIndex + StepTailLen);
        if (!ret)
        {
            WriteLog(LogLevel::Warning, "FromStepStream Failed. ProtocolType:%d, PackageId:%d, BodyLen:%d", protocolType_, head_.PackageId,
                     head_.BodyLen);
            package->Deallocate();
            package = nullptr;
            return false;
        }
        discardLength_ = 0;
        return true;
    }
}

PackageReader::AlignResult PackageReader::AlignToAnchor(const char* anchor, unsigned int anchorLength)
{
    unsigned int offset = 0;
    if (FindBytes(data_, length_, anchor, anchorLength, offset))
    {
        if (offset > 0)
        {
            DiscardFront(offset);
        }
        return AlignResult::Aligned;
    }
    //锚点可能跨收包边界，所以保留末尾不足一个锚点的字节，其余都是无意义的前缀
    unsigned int discardLength = length_ - std::min(length_, anchorLength - 1);
    if (discardLength > 0)
    {
        if (discardLength_ == 0)
        {
            //首次丢弃说明对端说的可能不是本协议，把这段字节的开头记下来便于定位
            unsigned int probe = 0;
            ::memcpy(&probe, data_, std::min(length_, unsigned(sizeof(probe))));
            WriteLog(LogLevel::Warning, "Package Start Not Found, Resync. SessionId:%lld, IP:%s, HeadValue:0x%08X, BufferLen:%u", sessionId_,
                     ipAddress_, probe, length_);
        }
        DiscardFront(discardLength);
    }
    if (discardLength_ > MaxPackageSize)
    {
        WriteLog(LogLevel::Error, "Garbage Stream Detected, DisConnect. SessionId:%lld, IP:%s, DiscardedBytes:%u", sessionId_, ipAddress_,
                 discardLength_);
        return AlignResult::GarbageStream;
    }
    return AlignResult::NeedMoreData;
}
void PackageReader::DiscardFront(unsigned int len)
{
    len = std::min(len, length_);
    discardLength_ += len;
    PopFront(len);
}
bool PackageReader::IsBodyLenWithinFrameLimit() const
{
    if (head_.BodyLen <= MaxFrameBodyLen)
    {
        return true;
    }
    WriteLog(LogLevel::Warning, "Body Length Exceeds Frame Limit. BodyLen:%u, MaxFrameBodyLen:%u, SessionId:%lld, IP:%s",
             static_cast<unsigned int>(head_.BodyLen), MaxFrameBodyLen, sessionId_, ipAddress_);
    return false;
}
}
