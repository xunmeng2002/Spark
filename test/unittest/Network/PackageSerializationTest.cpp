#include "PackageFactory.h"
#include "Packages.h"
#include <Spark/TemplateLib/ObjectPool/ObjectPool.h>
#include <Spark/Network/Protocol/PackageReader.h>
#include <Spark/Network/Protocol/StepUtility.h>
#include <Spark/Network/Protocol/Items.h>
#include <Spark/Network/Protocol/ProtocolVersion.h>
#include <Spark/Network/Protocol/ProtocolUtility.h>
#include <gtest/gtest.h>

#include <cstdio>
#include <cstring>
#include <format>
#include <limits>
#include <string>
using namespace Spark;
using namespace Spark::Network;
using namespace Spark::Packages;
// ============================================================
// Package 序列化测试
// 端到端测试 MakePackage → PackageReader::ParsePackage 往返
// 使用最简单的 NotifyComponentConnectStatusPackage（3 字段）
// ============================================================

namespace
{
constexpr SessionIdType SessionId = 42;
constexpr const char* IP = "192.168.1.100";

// 构造并填入字段
NotifyComponentConnectStatusPackage* CreateSamplePackage(int msgSeqNum = 1001)
{
    auto* pkg = NotifyComponentConnectStatusPackage::Allocate();
    pkg->Prepare(SessionId, 0, msgSeqNum);

    auto* field = ObjectPool<NotifyComponentConnectStatusField>::GetInstance().Allocate();
    field->SessionId = SessionId;
    field->Component = ComponentType::TradeFront;
    field->IsConnected = true;

    pkg->NotifyComponentConnectStatus = field;
    return pkg;
}

// 造一条完整报文，返回总长度；buff 由调用方提供
int MakeFrame(ProtocolTypeType protocolType, char* buff, int msgSeqNum)
{
    auto* pkg = CreateSamplePackage(msgSeqNum);
    int totalLen = pkg->MakePackage(protocolType, buff, MaxPackageSize);
    pkg->Deallocate();
    return totalLen;
}

// 把 Step 包头里的版本值就地改成另一个等宽版本号
bool PatchStepVersion(std::string& frame, unsigned short version)
{
    const std::string& anchor = StepUtility::GetPackageStartAnchor();
    std::string key = anchor + std::format("{:04X}", Items::Version) + "=";
    size_t pos = frame.find(key);
    if (pos == std::string::npos)
    {
        return false;
    }
    std::string origin = std::format("{:04X}", ProtocolVersionValue);
    std::string fresh = std::format("{:04X}", version);
    if (origin.size() != fresh.size())
    {
        return false;
    }
    frame.replace(pos + key.size(), origin.size(), fresh);
    return true;
}

// 验证解析后的包与原始值一致
void VerifyPackage(const NotifyComponentConnectStatusPackage* parsed, SessionIdType expectedSessionId, int expectedMsgSeqNum)
{
    ASSERT_NE(parsed, nullptr);
    ASSERT_NE(parsed->NotifyComponentConnectStatus, nullptr);
    EXPECT_EQ(parsed->SessionId, expectedSessionId);
    EXPECT_EQ(parsed->Head.MsgSeqNum, expectedMsgSeqNum);
    EXPECT_EQ(parsed->Head.PackageId, NotifyComponentConnectStatusPackage::PackageId);
    EXPECT_EQ(parsed->NotifyComponentConnectStatus->SessionId, SessionId);
    EXPECT_EQ(static_cast<int>(parsed->NotifyComponentConnectStatus->Component), static_cast<int>(ComponentType::TradeFront));
    EXPECT_EQ(parsed->NotifyComponentConnectStatus->IsConnected, true);
}
}

// ============================================================
// STEP 协议往返
// ============================================================

TEST(PackageSerializationTest, StepRoundTrip)
{
    // 1. 构造包
    auto* pkg = CreateSamplePackage();

    // 2. 序列化到缓冲
    char buff[MaxPackageSize] = {};
    int totalLen = pkg->MakePackage(ProtocolTypeType::Step, buff, MaxPackageSize);
    EXPECT_GT(totalLen, 0);
    // 报文以魔术字锚点开头，末尾是固定长度的报尾
    EXPECT_EQ(buff[0], static_cast<char>(SOH));
    EXPECT_EQ(memcmp(buff, StepUtility::GetPackageStartAnchor().c_str(), StepUtility::GetPackageStartAnchor().size()), 0);
    EXPECT_GT(totalLen, static_cast<int>(StepTailLen));

    // 3. 释放原包（模拟网络传输）
    pkg->Deallocate();

    // 4. 用 PackageReader 解析
    PackageFactory factory;
    PackageReader reader(ProtocolTypeType::Step, &factory, SessionId, IP);
    size_t appended = reader.Append(buff, totalLen);
    EXPECT_EQ(appended, static_cast<size_t>(totalLen));

    Package* parsedRaw = nullptr;
    bool parseResult = reader.ParsePackage(parsedRaw);
    EXPECT_TRUE(parseResult);
    ASSERT_NE(parsedRaw, nullptr);

    // 5. 验证字段
    auto* parsed = static_cast<NotifyComponentConnectStatusPackage*>(parsedRaw);
    VerifyPackage(parsed, SessionId, 1001);

    // 6. 清理
    parsed->Deallocate();
}

TEST(PackageSerializationTest, StepRoundTrip_MultipleMessages)
{
    // 连续两条消息验证 Reader 状态正确
    PackageFactory factory;
    PackageReader reader(ProtocolTypeType::Step, &factory, SessionId, IP);

    for (int seq = 1; seq <= 3; ++seq)
    {
        auto* pkg = CreateSamplePackage(seq);
        pkg->NotifyComponentConnectStatus->IsConnected = (seq % 2 == 1); // toggle

        char buff[MaxPackageSize] = {};
        int totalLen = pkg->MakePackage(ProtocolTypeType::Step, buff, MaxPackageSize);
        pkg->Deallocate();

        reader.Append(buff, totalLen);

        Package* parsedRaw = nullptr;
        bool parseResult = reader.ParsePackage(parsedRaw);
        EXPECT_TRUE(parseResult) << "Failed at seq=" << seq;
        ASSERT_NE(parsedRaw, nullptr);

        auto* parsed = static_cast<NotifyComponentConnectStatusPackage*>(parsedRaw);
        EXPECT_EQ(parsed->Head.MsgSeqNum, seq);
        EXPECT_EQ(parsed->NotifyComponentConnectStatus->IsConnected, (seq % 2 == 1));
        parsed->Deallocate();
    }
}

// ============================================================
// XTP 协议往返
// ============================================================

TEST(PackageSerializationTest, XtpRoundTrip)
{
    auto* pkg = CreateSamplePackage();

    char buff[MaxPackageSize] = {};
    int totalLen = pkg->MakePackage(ProtocolTypeType::Xtp, buff, MaxPackageSize);
    EXPECT_GT(totalLen, 0);
    // XTP: Head(binary) + Body + Tail(binary)
    EXPECT_GT(totalLen, FixedFrameOverhead);

    pkg->Deallocate();

    PackageFactory factory;
    PackageReader reader(ProtocolTypeType::Xtp, &factory, SessionId, IP);
    reader.Append(buff, totalLen);

    Package* parsedRaw = nullptr;
    bool parseResult = reader.ParsePackage(parsedRaw);
    EXPECT_TRUE(parseResult);
    ASSERT_NE(parsedRaw, nullptr);

    auto* parsed = static_cast<NotifyComponentConnectStatusPackage*>(parsedRaw);
    VerifyPackage(parsed, SessionId, 1001);

    parsed->Deallocate();
}

TEST(PackageSerializationTest, XtpRoundTrip_CheckSumVerify)
{
    // 校验和不符的帧必须被丢弃。丢弃方式是重同步而不是清空整段缓冲：
    // 只保留末尾不足一个魔术字的字节，等下一次收包再对齐
    char buff[MaxPackageSize] = {};
    int totalLen = MakeFrame(ProtocolTypeType::Xtp, buff, 1001);
    ASSERT_GT(totalLen, 0);

    // 篡改校验和
    TailField tail = {};
    memcpy(&tail, buff + totalLen - sizeof(TailField), sizeof(tail));
    tail.CheckSum = 0xFF; // 错误值
    memcpy(buff + totalLen - sizeof(TailField), &tail, sizeof(tail));

    PackageFactory factory;
    PackageReader reader(ProtocolTypeType::Xtp, &factory, SessionId, IP);
    reader.Append(buff, totalLen);

    Package* parsedRaw = nullptr;
    EXPECT_TRUE(reader.ParsePackage(parsedRaw)); // 不是致命错误，不断链
    EXPECT_EQ(parsedRaw, nullptr);
    EXPECT_EQ(reader.Length(), sizeof(ProtocolMagicValue) - 1);
}

TEST(PackageSerializationTest, XtpRoundTrip_WrongVersionIsFatal)
{
    // 版本不符说明对端说的不是本协议的格式，必须返回 false 让上层断链，
    // 不能当成可重同步的乱码。版本检查排在长度与校验之前
    char buff[MaxPackageSize] = {};
    int totalLen = MakeFrame(ProtocolTypeType::Xtp, buff, 1001);
    ASSERT_GT(totalLen, 0);

    HeadField head = {};
    memcpy(&head, buff, sizeof(head));
    EXPECT_EQ(head.Version, ProtocolVersionValue);
    head.Version = static_cast<UInt16Type>(ProtocolVersionValue + 1);
    memcpy(buff, &head, sizeof(head));

    PackageFactory factory;
    PackageReader reader(ProtocolTypeType::Xtp, &factory, SessionId, IP);
    reader.Append(buff, totalLen);

    Package* parsedRaw = nullptr;
    EXPECT_FALSE(reader.ParsePackage(parsedRaw));
    EXPECT_EQ(parsedRaw, nullptr);
}

TEST(PackageSerializationTest, XtpRoundTrip_CorruptHeadIsDiscarded)
{
    // 魔术字被改坏后这一帧已经无法定位，应当被当作噪声丢弃
    char buff[MaxPackageSize] = {};
    int totalLen = MakeFrame(ProtocolTypeType::Xtp, buff, 1001);
    ASSERT_GT(totalLen, 0);

    buff[0] ^= 0x01;

    PackageFactory factory;
    PackageReader reader(ProtocolTypeType::Xtp, &factory, SessionId, IP);
    reader.Append(buff, totalLen);

    Package* parsedRaw = nullptr;
    EXPECT_TRUE(reader.ParsePackage(parsedRaw));
    EXPECT_EQ(parsedRaw, nullptr);
    EXPECT_EQ(reader.Length(), sizeof(ProtocolMagicValue) - 1);
}

TEST(PackageSerializationTest, XtpRoundTrip_GarbagePrefixIsResynced)
{
    // 前面塞一段噪声，接收端应当丢弃它们并重新对齐到真正的魔术字
    const char garbage[] = {'n', 'o', 'i', 's', 'e', '!', '!'};

    char buff[MaxPackageSize] = {};
    int totalLen = MakeFrame(ProtocolTypeType::Xtp, buff, 1001);
    ASSERT_GT(totalLen, 0);

    PackageFactory factory;
    PackageReader reader(ProtocolTypeType::Xtp, &factory, SessionId, IP);
    EXPECT_EQ(reader.Append(const_cast<char*>(garbage), sizeof(garbage)), sizeof(garbage));

    Package* parsedRaw = nullptr;
    EXPECT_TRUE(reader.ParsePackage(parsedRaw));
    EXPECT_EQ(parsedRaw, nullptr);

    EXPECT_EQ(reader.Append(buff, totalLen), static_cast<size_t>(totalLen));
    EXPECT_TRUE(reader.ParsePackage(parsedRaw));
    ASSERT_NE(parsedRaw, nullptr);
    EXPECT_EQ(parsedRaw->Head.MsgSeqNum, 1001);
    parsedRaw->Deallocate();
}

TEST(PackageSerializationTest, XtpRoundTrip_ResyncAfterCorruptFrame)
{
    // 坏帧后面的好帧不能跟着一起丢：重同步的落点必须是好帧的魔术字
    char corrupt[MaxPackageSize] = {};
    int corruptLen = MakeFrame(ProtocolTypeType::Xtp, corrupt, 1);
    ASSERT_GT(corruptLen, 0);
    // 只改包体里的一个字节，长度字段仍然可信，只有 CRC 失配
    corrupt[sizeof(HeadField) + 1] ^= 0x5A;

    char good[MaxPackageSize] = {};
    int goodLen = MakeFrame(ProtocolTypeType::Xtp, good, 2);
    ASSERT_GT(goodLen, 0);

    PackageFactory factory;
    PackageReader reader(ProtocolTypeType::Xtp, &factory, SessionId, IP);
    reader.Append(corrupt, corruptLen);
    reader.Append(good, goodLen);

    Package* parsedRaw = nullptr;
    EXPECT_TRUE(reader.ParsePackage(parsedRaw));
    ASSERT_NE(parsedRaw, nullptr);
    // 越过坏帧，好帧的字段完整解出
    EXPECT_EQ(parsedRaw->Head.MsgSeqNum, 2);
    VerifyPackage(static_cast<NotifyComponentConnectStatusPackage*>(parsedRaw), SessionId, 2);
    parsedRaw->Deallocate();
}

TEST(PackageSerializationTest, XtpRoundTrip_MagicSplitAcrossAppend)
{
    // 魔术字跨收包边界时必须一个字节都不丢
    char buff[MaxPackageSize] = {};
    int totalLen = MakeFrame(ProtocolTypeType::Xtp, buff, 1001);
    ASSERT_GT(totalLen, 0);
    // 先只送入魔术字的前两个字节
    constexpr int Split = 2;
    ASSERT_LT(Split, static_cast<int>(sizeof(ProtocolMagicValue)));

    PackageFactory factory;
    PackageReader reader(ProtocolTypeType::Xtp, &factory, SessionId, IP);
    EXPECT_EQ(reader.Append(buff, Split), static_cast<size_t>(Split));

    Package* parsedRaw = nullptr;
    EXPECT_TRUE(reader.ParsePackage(parsedRaw));
    EXPECT_EQ(parsedRaw, nullptr);
    EXPECT_EQ(reader.Length(), static_cast<size_t>(Split));

    EXPECT_EQ(reader.Append(buff + Split, totalLen - Split), static_cast<size_t>(totalLen - Split));
    EXPECT_TRUE(reader.ParsePackage(parsedRaw));
    ASSERT_NE(parsedRaw, nullptr);
    EXPECT_EQ(parsedRaw->Head.MsgSeqNum, 1001);
    parsedRaw->Deallocate();
}

// ============================================================
// STEP 协议的定位、版本与容错
// ============================================================

TEST(PackageSerializationTest, StepRoundTrip_WrongVersionIsFatal)
{
    char buff[MaxPackageSize] = {};
    int totalLen = MakeFrame(ProtocolTypeType::Step, buff, 1001);
    ASSERT_GT(totalLen, 0);

    std::string frame(buff, buff + totalLen);
    ASSERT_TRUE(PatchStepVersion(frame, static_cast<unsigned short>(ProtocolVersionValue + 1)));

    PackageFactory factory;
    PackageReader reader(ProtocolTypeType::Step, &factory, SessionId, IP);
    reader.Append(&frame[0], frame.size());

    Package* parsedRaw = nullptr;
    EXPECT_FALSE(reader.ParsePackage(parsedRaw));
    EXPECT_EQ(parsedRaw, nullptr);
}

TEST(PackageSerializationTest, StepRoundTrip_GarbagePrefixIsResynced)
{
    const char garbage[] = {'n', 'o', 'i', 's', 'e', '!', '!'};

    char buff[MaxPackageSize] = {};
    int totalLen = MakeFrame(ProtocolTypeType::Step, buff, 1001);
    ASSERT_GT(totalLen, 0);

    PackageFactory factory;
    PackageReader reader(ProtocolTypeType::Step, &factory, SessionId, IP);
    EXPECT_EQ(reader.Append(const_cast<char*>(garbage), sizeof(garbage)), sizeof(garbage));

    Package* parsedRaw = nullptr;
    EXPECT_TRUE(reader.ParsePackage(parsedRaw));
    EXPECT_EQ(parsedRaw, nullptr);

    EXPECT_EQ(reader.Append(buff, totalLen), static_cast<size_t>(totalLen));
    EXPECT_TRUE(reader.ParsePackage(parsedRaw));
    ASSERT_NE(parsedRaw, nullptr);
    VerifyPackage(static_cast<NotifyComponentConnectStatusPackage*>(parsedRaw), SessionId, 1001);
    parsedRaw->Deallocate();
}

TEST(PackageSerializationTest, StepRoundTrip_ResyncAfterCorruptFrame)
{
    char corrupt[MaxPackageSize] = {};
    int corruptLen = MakeFrame(ProtocolTypeType::Step, corrupt, 1);
    ASSERT_GT(corruptLen, 0);
    // 改坏包体中的一个字节，包头仍然合法，只有 CRC 失配
    corrupt[corruptLen - static_cast<int>(StepTailLen) - 1] ^= 0x5A;

    char good[MaxPackageSize] = {};
    int goodLen = MakeFrame(ProtocolTypeType::Step, good, 2);
    ASSERT_GT(goodLen, 0);

    PackageFactory factory;
    PackageReader reader(ProtocolTypeType::Step, &factory, SessionId, IP);
    reader.Append(corrupt, corruptLen);
    reader.Append(good, goodLen);

    Package* parsedRaw = nullptr;
    EXPECT_TRUE(reader.ParsePackage(parsedRaw));
    ASSERT_NE(parsedRaw, nullptr);
    EXPECT_EQ(parsedRaw->Head.MsgSeqNum, 2);
    VerifyPackage(static_cast<NotifyComponentConnectStatusPackage*>(parsedRaw), SessionId, 2);
    parsedRaw->Deallocate();
}

TEST(PackageSerializationTest, StepRoundTrip_AnchorSplitAcrossAppend)
{
    char buff[MaxPackageSize] = {};
    int totalLen = MakeFrame(ProtocolTypeType::Step, buff, 1001);
    ASSERT_GT(totalLen, 0);

    int anchorLen = static_cast<int>(StepUtility::GetPackageStartAnchor().size());
    constexpr int Split = 5;
    ASSERT_LT(Split, anchorLen);

    PackageFactory factory;
    PackageReader reader(ProtocolTypeType::Step, &factory, SessionId, IP);
    EXPECT_EQ(reader.Append(buff, Split), static_cast<size_t>(Split));

    Package* parsedRaw = nullptr;
    EXPECT_TRUE(reader.ParsePackage(parsedRaw));
    EXPECT_EQ(parsedRaw, nullptr);
    EXPECT_EQ(reader.Length(), static_cast<size_t>(Split));

    EXPECT_EQ(reader.Append(buff + Split, totalLen - Split), static_cast<size_t>(totalLen - Split));
    EXPECT_TRUE(reader.ParsePackage(parsedRaw));
    ASSERT_NE(parsedRaw, nullptr);
    VerifyPackage(static_cast<NotifyComponentConnectStatusPackage*>(parsedRaw), SessionId, 1001);
    parsedRaw->Deallocate();
}

// ============================================================
// Prepare 方法测试
// ============================================================

TEST(PackageSerializationTest, PrepareSetsFields)
{
    NotifyComponentConnectStatusPackage pkg;
    pkg.Prepare(SessionId, 1, 999);

    EXPECT_EQ(pkg.SessionId, SessionId);
    EXPECT_EQ(pkg.Head.MsgSeqNum, 999);
    EXPECT_EQ(pkg.Head.MessageChain, 1);
    EXPECT_EQ(pkg.Head.PackageId, NotifyComponentConnectStatusPackage::PackageId);
}

// ============================================================
// 空字段（nullptr）包序列化
// ============================================================

TEST(PackageSerializationTest, EmptyField_NullBody)
{
    // 字段为 nullptr 时 MakePackage 应仍能产生有效报文（body 为空）
    auto* pkg = NotifyComponentConnectStatusPackage::Allocate();
    pkg->Prepare(SessionId, 0, 1);
    // 不设置 NotifyComponentConnectStatus

    char buff[MaxPackageSize] = {};
    int totalLenStep = pkg->MakePackage(ProtocolTypeType::Step, buff, MaxPackageSize);
    EXPECT_GT(totalLenStep, 0);

    int totalLenXtp = pkg->MakePackage(ProtocolTypeType::Xtp, buff, MaxPackageSize);
    EXPECT_GT(totalLenXtp, 0);

    pkg->Deallocate();
}

// ============================================================
// 包体超出一帧上限
// ============================================================

namespace
{
// 生成器不看 size 参数，这里只回报一个超限的包体长度，用来验证 MakePackage 的长度契约
class OversizedBodyPackage : public Package
{
public:
    void Deallocate() override {}
    int ToStepStream(char*, int) const override { return static_cast<int>(MaxFrameBodyLen) + 1; }
    bool FromStepStream(char*, int, int) override { return true; }
    int ToXtpStream(char*, int) const override { return static_cast<int>(MaxFrameBodyLen) + 1; }
    bool FromXtpStream(char*, int, int) override { return true; }
    const char* GetDebugString() const override { return "OversizedBodyPackage"; }
};
}

TEST(PackageSerializationTest, OversizedBody_RejectedBeforeWrite)
{
    OversizedBodyPackage pkg;
    pkg.Prepare(SessionId, 0, 1);

    char buff[MaxPackageSize] = {};
    buff[0] = 'X';
    // Xtp 的长度契约在写报文头之前判定，缓冲一个字节都不该动
    EXPECT_EQ(pkg.MakePackage(ProtocolTypeType::Xtp, buff, MaxPackageSize), 0);
    EXPECT_EQ(buff[0], 'X');

    // Step 的包头定长，容量闸门同样排在调用生成器之前，只要求拒绝发送
    EXPECT_EQ(pkg.MakePackage(ProtocolTypeType::Step, buff, MaxPackageSize), 0);
}

// ============================================================
// 生成器的容量边界：正好放得下 / 差 1 字节
// ============================================================

TEST(PackageSerializationTest, StepBody_CapacityBoundary)
{
    auto* pkg = CreateSamplePackage();
    char reference[MaxPackageSize] = {};
    int bodyLen = pkg->ToStepStream(reference, MaxPackageSize);
    ASSERT_GT(bodyLen, 0);

    //容量正好等于包体长度（每个字段的 SOH 都在内）：成功的最小边界，字节与参考逐字节一致
    char exact[MaxPackageSize] = {};
    EXPECT_EQ(pkg->ToStepStream(exact, bodyLen), bodyLen);
    EXPECT_EQ(std::string(exact, bodyLen), std::string(reference, bodyLen));

    //再少 1 字节：为 SOH 预留的那 1 字节放不下，生成器必须回 -1 而不是把字段截断写出去
    char truncated[MaxPackageSize] = {};
    EXPECT_EQ(pkg->ToStepStream(truncated, bodyLen - 1), -1);

    pkg->Deallocate();
}

TEST(PackageSerializationTest, XtpBody_CapacityBoundary)
{
    auto* pkg = CreateSamplePackage();
    char reference[MaxPackageSize] = {};
    int bodyLen = pkg->ToXtpStream(reference, MaxPackageSize);
    ASSERT_GT(bodyLen, 0);

    char exact[MaxPackageSize] = {};
    EXPECT_EQ(pkg->ToXtpStream(exact, bodyLen), bodyLen);
    EXPECT_EQ(std::string(exact, bodyLen), std::string(reference, bodyLen));

    //XTP 的字段是定长记录，差 1 字节时末条记录放不下
    char truncated[MaxPackageSize] = {};
    EXPECT_EQ(pkg->ToXtpStream(truncated, bodyLen - 1), -1);

    pkg->Deallocate();
}

// ============================================================
// 包体长度为负（生成器报容量不足）
// ============================================================

namespace
{
//生成器回 -1 表示"给定容量放不下"，MakePackage 必须拒绝发送
class TruncatedBodyPackage : public Package
{
public:
    void Deallocate() override {}
    int ToStepStream(char*, int) const override { return -1; }
    bool FromStepStream(char*, int, int) override { return true; }
    int ToXtpStream(char*, int) const override { return -1; }
    bool FromXtpStream(char*, int, int) override { return true; }
    const char* GetDebugString() const override { return "TruncatedBodyPackage"; }
};
}

TEST(PackageSerializationTest, TruncatedBody_RejectedBeforeWrite)
{
    TruncatedBodyPackage pkg;
    pkg.Prepare(SessionId, 0, 1);

    char buff[MaxPackageSize] = {};
    buff[0] = 'X';
    // Xtp 的长度契约在写报文头之前判定，缓冲一个字节都不该动
    EXPECT_EQ(pkg.MakePackage(ProtocolTypeType::Xtp, buff, MaxPackageSize), 0);
    EXPECT_EQ(buff[0], 'X');

    // Step 的包头定长，容量闸门同样排在调用生成器之前，只要求拒绝发送
    EXPECT_EQ(pkg.MakePackage(ProtocolTypeType::Step, buff, MaxPackageSize), 0);
}

// ============================================================
// 缓冲小于单帧固定开销（报文头 + 报文尾 = 20 字节）
// ============================================================

namespace
{
//按仓外实现的常见写法故意"先写后判"：拿到包体缓冲先改一个字节，再去看容量够不够。
//ToXtpStream 是公开纯虚函数，仓外实现不保证先比容量再写，所以 MakePackage 必须在调用它之前
//就把尺寸挡住——只要它被调用过，是否越界写就已经交给下游决定了
class WriteBeforeMeasurePackage : public Package
{
public:
    void Deallocate() override {}
    int ToStepStream(char* buff, int capacity) const override { return Probe(buff, capacity); }
    bool FromStepStream(char*, int, int) override { return true; }
    int ToXtpStream(char* buff, int capacity) const override { return Probe(buff, capacity); }
    bool FromXtpStream(char*, int, int) override { return true; }
    const char* GetDebugString() const override { return "WriteBeforeMeasurePackage"; }

    bool WasMeasured() const { return is_measured_; }
    int ObservedCapacity() const { return observed_capacity_; }

private:
    int Probe(char* buff, int capacity) const
    {
        is_measured_ = true;
        observed_capacity_ = capacity;
        buff[0] = 'W';
        return 0;
    }

    mutable bool is_measured_ = false;
    mutable int observed_capacity_ = 0;
};
}

TEST(PackageSerializationTest, MakePackage_BufferSmallerThanFixedOverhead)
{
    //物理缓冲给足，传进去的 size 才是唯一的自变量：闸门一旦失效，越界写会落在分配内被最后那条
    //断言读到，而不是把用例变成一次随机崩溃
    const int sizes[] = {FixedFrameOverhead - 1, 0, -1, std::numeric_limits<int>::min()};

    for (int size : sizes)
    {
        WriteBeforeMeasurePackage pkg;
        pkg.Prepare(SessionId, 0, 1);

        char buff[MaxPackageSize] = {};
        std::memset(buff, 'S', sizeof(buff));

        EXPECT_EQ(pkg.MakePackage(ProtocolTypeType::Xtp, buff, size), 0) << "size=" << size;
        EXPECT_EQ(pkg.MakePackage(ProtocolTypeType::Step, buff, size), 0) << "size=" << size;
        //闸门必须排在指针与容量运算之前：生成器一次都不该被调用，负容量也不该被传出去
        EXPECT_FALSE(pkg.WasMeasured()) << "size=" << size;

        EXPECT_EQ(std::string(buff, sizeof(buff)), std::string(sizeof(buff), 'S')) << "size=" << size;
    }
}

TEST(PackageSerializationTest, MakePackage_BufferExactlyFixedOverheadReachesGenerator)
{
    //闸门边界必须正好落在 20 字节：恰为 20 时 XTP 的包体容量是 0，仍要放行到生成器，
    //由生成器回一个装得进 0 字节的包体长度。闸门写宽一字节就会把这种边界帧误拒
    WriteBeforeMeasurePackage pkg;
    pkg.Prepare(SessionId, 0, 1);

    char buff[MaxPackageSize] = {};
    EXPECT_EQ(pkg.MakePackage(ProtocolTypeType::Xtp, buff, FixedFrameOverhead), FixedFrameOverhead);
    EXPECT_TRUE(pkg.WasMeasured());
    EXPECT_EQ(pkg.ObservedCapacity(), 0);
}

TEST(PackageSerializationTest, StepPackage_BufferExactlyHeadPlusTailReachesGenerator)
{
    //Step 的闸门是定长包头 62 + 报尾 14 = 76 字节：恰为 76 时包体容量是 0，仍要放行到生成器；
    //差 1 字节时生成器一次都不该被调用。闸门写宽一字节就会把这种边界帧误拒
    const int needed = static_cast<int>(StepHeadLen + StepTailLen);

    WriteBeforeMeasurePackage pkg;
    pkg.Prepare(SessionId, 0, 1);

    char buff[MaxPackageSize] = {};
    EXPECT_EQ(pkg.MakePackage(ProtocolTypeType::Step, buff, needed - 1), 0);
    EXPECT_FALSE(pkg.WasMeasured());

    //紧邻的第二次调用才会放行：正方向边界必须落在 76 而不是更大
    EXPECT_EQ(pkg.MakePackage(ProtocolTypeType::Step, buff, needed), needed);
    EXPECT_TRUE(pkg.WasMeasured());
    EXPECT_EQ(pkg.ObservedCapacity(), 0);
}

TEST(PackageSerializationTest, MakePackage_NullBufferRejected)
{
    //缓冲为空指针时尺寸再大也无意义：闸门必须按空指针判失败，而不是拿它去做指针算术。
    //这条属新增的正性检查，旧实现会在这里崩
    WriteBeforeMeasurePackage pkg;
    pkg.Prepare(SessionId, 0, 1);

    EXPECT_EQ(pkg.MakePackage(ProtocolTypeType::Xtp, nullptr, MaxPackageSize), 0);
    EXPECT_EQ(pkg.MakePackage(ProtocolTypeType::Step, nullptr, MaxPackageSize), 0);
    EXPECT_FALSE(pkg.WasMeasured());
}

// ============================================================
// int32s 桶字段的读路径
// 读侧由 atoi 改为 StepUtility::ParseInteger（越界拒绝而非静默截断），
// 这里用含 int32 字段的真实包把该分支打进：取值正确、越界被拒。
// 原有用例只覆盖 NotifyComponentConnectStatusPackage，其三个字段分属 int64s/enums/bools，
// 一条都到不了这个分支。
// ============================================================

namespace
{
// 就地改写某个 item 的十进制值。新旧值必须等宽：线上字段以 SOH 分隔，
// 宽度一变就会挪动后面的分隔符，构造出的帧不再是"只改了值"的那一条。
// 定位靠"键 + 期望值"两段一起匹配，而不是只看键——同一个 item 可能在一条帧里出现多次
// （同名具名类型被多个包引用），撞上别处就会改错地方
bool PatchStepItemValue(std::string& frame, UInt16Type itemId, const std::string& origin, const std::string& replacement)
{
    if (origin.size() != replacement.size())
    {
        return false;
    }
    std::string key = std::format("{:04X}=", itemId);
    size_t pos = frame.find(key);
    while (pos != std::string::npos)
    {
        size_t valuePos = pos + key.size();
        if (frame.compare(valuePos, origin.size(), origin) == 0)
        {
            frame.replace(valuePos, origin.size(), replacement);
            return true;
        }
        pos = frame.find(key, pos + 1);
    }
    return false;
}

// 值改过之后 CRC 必然失配，而读侧是"先校 CRC 再进解析"，不重算报尾就永远走不到读字段那一步
void RefreshStepTail(char* buff, int totalLen)
{
    int tailIndex = totalLen - static_cast<int>(StepTailLen);
    TailField tail = {};
    tail.CheckSum = CalculateCrc32c(reinterpret_cast<const unsigned char*>(buff), tailIndex);
    EXPECT_EQ(StepUtility::TailToStream(&tail, buff + tailIndex, static_cast<int>(StepTailLen)), static_cast<int>(StepTailLen));
}

ReqInsertOrderPackage* CreateSampleInsertOrder(ClientOrderIdType clientOrderId)
{
    auto* pkg = ReqInsertOrderPackage::Allocate();
    pkg->Prepare(SessionId, 0, 1001);

    auto* field = ObjectPool<ReqInsertOrderField>::GetInstance().Allocate();
    std::memset(field, 0, sizeof(*field));
    std::snprintf(field->AccountId, sizeof(field->AccountId), "Xunmeng001");
    std::snprintf(field->ExchangeId, sizeof(field->ExchangeId), "SHSE");
    std::snprintf(field->InstrumentId, sizeof(field->InstrumentId), "600036");
    field->Direction = DirectionType::Buy;
    field->OffsetFlag = OffsetFlagType::Open;
    field->OrderPriceType = OrderPriceTypeType::LimitPrice;
    field->Price = 88.88;
    field->Volume = 1000;
    field->ClientOrderId = clientOrderId;

    pkg->ReqInsertOrder = field;
    return pkg;
}
}

TEST(PackageSerializationTest, StepRoundTrip_Int32FieldPreserved)
{
    // 取 int32 值域上沿附近的值：atoi 与 from_chars 在合法十进制下取值一致，
    // 但一旦写侧改成补位/带符号，这里的写法断言会先报出来
    constexpr ClientOrderIdType ExpectedClientOrderId = 2000000000;

    auto* pkg = CreateSampleInsertOrder(ExpectedClientOrderId);
    char buff[MaxPackageSize] = {};
    int totalLen = pkg->MakePackage(ProtocolTypeType::Step, buff, MaxPackageSize);
    EXPECT_GT(totalLen, 0);
    pkg->Deallocate();

    std::string frame(buff, totalLen);
    EXPECT_NE(frame.find(std::format("{:04X}={:d}", Items::ClientOrderId, ExpectedClientOrderId)), std::string::npos)
        << "int32 字段必须写成裸十进制（无补位、无前导空格、无正号），否则严格解析会拒绝自家输出";

    PackageFactory factory;
    PackageReader reader(ProtocolTypeType::Step, &factory, SessionId, IP);
    EXPECT_EQ(reader.Append(buff, totalLen), static_cast<size_t>(totalLen));

    Package* parsedRaw = nullptr;
    ASSERT_TRUE(reader.ParsePackage(parsedRaw));
    ASSERT_NE(parsedRaw, nullptr);

    auto* parsed = static_cast<ReqInsertOrderPackage*>(parsedRaw);
    ASSERT_NE(parsed->ReqInsertOrder, nullptr);
    EXPECT_EQ(parsed->ReqInsertOrder->ClientOrderId, ExpectedClientOrderId);
    // 相邻字段一并核对：Volume 属 int64s（atoll）、Price 属 doubles（atof），本次改动不应波及
    EXPECT_EQ(parsed->ReqInsertOrder->Volume, 1000);
    EXPECT_DOUBLE_EQ(parsed->ReqInsertOrder->Price, 88.88);
    EXPECT_EQ(static_cast<int>(parsed->ReqInsertOrder->Direction), static_cast<int>(DirectionType::Buy));

    parsed->Deallocate();
}

TEST(PackageSerializationTest, StepRoundTrip_Int32OutOfRangeIsRejected)
{
    // 改前 atoi 对越界值静默截断（实现定义），报文照常解析成功并落进一个错值；
    // 改后必须整包拒绝。本用例先跑一条未改写的对照帧，确认"失败"不是别的原因造成的假绿
    const std::string InRangeValue = "2000000000";
    const std::string OverflowValue = "9999999999";

    auto* pkg = CreateSampleInsertOrder(2000000000);
    char buff[MaxPackageSize] = {};
    int totalLen = pkg->MakePackage(ProtocolTypeType::Step, buff, MaxPackageSize);
    EXPECT_GT(totalLen, 0);
    pkg->Deallocate();

    std::string frame(buff, totalLen);
    ASSERT_TRUE(PatchStepItemValue(frame, Items::ClientOrderId, InRangeValue, OverflowValue));

    {
        PackageFactory factory;
        PackageReader reader(ProtocolTypeType::Step, &factory, SessionId, IP);
        reader.Append(buff, totalLen);
        Package* controlRaw = nullptr;
        ASSERT_TRUE(reader.ParsePackage(controlRaw)) << "对照帧都解析不了，下面的失败结论不作数";
        ASSERT_NE(controlRaw, nullptr);
        EXPECT_EQ(static_cast<ReqInsertOrderPackage*>(controlRaw)->ReqInsertOrder->ClientOrderId, 2000000000);
        controlRaw->Deallocate();
    }

    std::memcpy(buff, frame.data(), frame.size());
    RefreshStepTail(buff, totalLen);

    PackageFactory factory;
    PackageReader reader(ProtocolTypeType::Step, &factory, SessionId, IP);
    EXPECT_EQ(reader.Append(buff, totalLen), static_cast<size_t>(totalLen));

    Package* parsedRaw = nullptr;
    EXPECT_FALSE(reader.ParsePackage(parsedRaw));
    EXPECT_EQ(parsedRaw, nullptr);
}

namespace
{
bool ParseStepFrame(char* frameData, int totalLen, Package*& parsed)
{
    PackageFactory factory;
    PackageReader reader(ProtocolTypeType::Step, &factory, SessionId, IP);
    if (reader.Append(frameData, totalLen) != static_cast<size_t>(totalLen))
    {
        return false;
    }
    parsed = nullptr;
    return reader.ParsePackage(parsed);
}

RspQryCapitalPackage* CreateSampleCapital(AccountTypeType accountType, MoneyType asset)
{
    auto* pkg = RspQryCapitalPackage::Allocate();
    pkg->Prepare(SessionId, 0, 1002);

    auto* field = ObjectPool<CapitalField>::GetInstance().Allocate();
    std::memset(field, 0, sizeof(*field));
    std::snprintf(field->TradingDay, sizeof(field->TradingDay), "20260930");
    std::snprintf(field->AccountId, sizeof(field->AccountId), "Xunmeng001");
    field->AccountType = accountType;
    field->Asset = asset;

    pkg->Capital = field;
    return pkg;
}

struct NonFiniteAssetSample
{
    MoneyType AssetValue;
    const char* WrittenText;
};

constexpr NonFiniteAssetSample NonFiniteAssetSamples[] = {
    {std::numeric_limits<MoneyType>::quiet_NaN(), "nan"},
    {std::numeric_limits<MoneyType>::infinity(), "inf"},
    {-std::numeric_limits<MoneyType>::infinity(), "-inf"},
};
}

TEST(PackageSerializationTest, StepRoundTrip_EnumFieldRejectsNonIntegerText)
{
    auto* pkg = CreateSampleCapital(AccountTypeType::Primary, 100.5);
    char buff[MaxPackageSize] = {};
    const int totalLen = pkg->MakePackage(ProtocolTypeType::Step, buff, MaxPackageSize);
    EXPECT_GT(totalLen, 0);
    pkg->Deallocate();

    std::string frame(buff, totalLen);
    EXPECT_NE(frame.find(std::format("{:04X}=0", Items::AccountType)), std::string::npos)
        << "枚举写侧必须写成裸十进制整数，否则严格解析会拒绝自家输出";

    Package* parsedRaw = nullptr;
    ASSERT_TRUE(ParseStepFrame(buff, totalLen, parsedRaw)) << "对照帧都解析不了，下面的失败结论不作数";
    EXPECT_EQ(static_cast<int>(static_cast<RspQryCapitalPackage*>(parsedRaw)->Capital->AccountType), static_cast<int>(AccountTypeType::Primary));
    parsedRaw->Deallocate();

    std::string nonIntegerFrame = frame;
    ASSERT_TRUE(PatchStepItemValue(nonIntegerFrame, Items::AccountType, "0", "x"));
    std::memcpy(buff, nonIntegerFrame.data(), nonIntegerFrame.size());
    RefreshStepTail(buff, totalLen);
    parsedRaw = nullptr;
    EXPECT_FALSE(ParseStepFrame(buff, totalLen, parsedRaw));
    EXPECT_EQ(parsedRaw, nullptr);

    std::string unnamedValueFrame = frame;
    ASSERT_TRUE(PatchStepItemValue(unnamedValueFrame, Items::AccountType, "0", "9"));
    std::memcpy(buff, unnamedValueFrame.data(), unnamedValueFrame.size());
    RefreshStepTail(buff, totalLen);
    parsedRaw = nullptr;
    ASSERT_TRUE(ParseStepFrame(buff, totalLen, parsedRaw));
    EXPECT_EQ(static_cast<int>(static_cast<RspQryCapitalPackage*>(parsedRaw)->Capital->AccountType), 9);
    parsedRaw->Deallocate();
}

TEST(PackageSerializationTest, StepRoundTrip_NonFiniteDoubleFieldIsRejected)
{
    auto* controlPkg = CreateSampleCapital(AccountTypeType::Primary, 100.5);
    char controlBuff[MaxPackageSize] = {};
    const int controlLen = controlPkg->MakePackage(ProtocolTypeType::Step, controlBuff, MaxPackageSize);
    EXPECT_GT(controlLen, 0);
    controlPkg->Deallocate();

    Package* controlRaw = nullptr;
    ASSERT_TRUE(ParseStepFrame(controlBuff, controlLen, controlRaw)) << "对照帧都解析不了，下面的失败结论不作数";
    EXPECT_DOUBLE_EQ(static_cast<RspQryCapitalPackage*>(controlRaw)->Capital->Asset, 100.5);
    controlRaw->Deallocate();

    for (const NonFiniteAssetSample& sample : NonFiniteAssetSamples)
    {
        auto* pkg = CreateSampleCapital(AccountTypeType::Primary, sample.AssetValue);
        char buff[MaxPackageSize] = {};
        const int totalLen = pkg->MakePackage(ProtocolTypeType::Step, buff, MaxPackageSize);
        EXPECT_GT(totalLen, 0);
        pkg->Deallocate();

        const std::string frame(buff, totalLen);
        EXPECT_NE(frame.find(std::format("{:04X}={}", Items::Asset, sample.WrittenText)), std::string::npos)
            << "写侧本就把 " << sample.WrittenText << " 原样写出，读侧必须拒绝";

        Package* parsedRaw = nullptr;
        EXPECT_FALSE(ParseStepFrame(buff, totalLen, parsedRaw)) << sample.WrittenText;
        EXPECT_EQ(parsedRaw, nullptr);
    }
}

TEST(PackageSerializationTest, StepRoundTrip_BoolFieldRejectsNonZeroOne)
{
    char buff[MaxPackageSize] = {};
    const int totalLen = MakeFrame(ProtocolTypeType::Step, buff, 1001);
    ASSERT_GT(totalLen, 0);

    std::string frame(buff, totalLen);
    EXPECT_NE(frame.find(std::format("{:04X}=1", Items::IsConnected)), std::string::npos);

    Package* controlRaw = nullptr;
    ASSERT_TRUE(ParseStepFrame(buff, totalLen, controlRaw)) << "对照帧都解析不了，下面的失败结论不作数";
    EXPECT_TRUE(static_cast<NotifyComponentConnectStatusPackage*>(controlRaw)->NotifyComponentConnectStatus->IsConnected);
    controlRaw->Deallocate();

    ASSERT_TRUE(PatchStepItemValue(frame, Items::IsConnected, "1", "2"));
    std::memcpy(buff, frame.data(), frame.size());
    RefreshStepTail(buff, totalLen);

    Package* parsedRaw = nullptr;
    EXPECT_FALSE(ParseStepFrame(buff, totalLen, parsedRaw));
    EXPECT_EQ(parsedRaw, nullptr);
}
