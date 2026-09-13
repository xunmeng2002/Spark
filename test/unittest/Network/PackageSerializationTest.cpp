#include "PackageFactory.h"
#include "Packages.h"
#include <Spark/TemplateLib/ObjectPool/ObjectPool.h>
#include <Spark/Network/Protocol/PackageReader.h>
#include <Spark/Network/Protocol/StepUtility.h>
#include <Spark/Network/Protocol/Items.h>
#include <Spark/Network/Protocol/ProtocolVersion.h>
#include <gtest/gtest.h>

#include <cstring>
#include <string>
using namespace spark;
using namespace spark::network;
using namespace spark::packages;
// ============================================================
// Package 序列化测试
// 端到端测试 MakePackage → PackageReader::ParsePackage 往返
// 使用最简单的 NotifyComponentConnectStatusPackage（3 字段）
// ============================================================

namespace
{
    constexpr SessionIDType kSessionID = 42;
    constexpr const char* kIP = "192.168.1.100";

    // 构造并填入字段
    NotifyComponentConnectStatusPackage* CreateSamplePackage(int msgSeqNum = 1001)
    {
        auto* pkg = NotifyComponentConnectStatusPackage::Allocate();
        pkg->Prepare(kSessionID, 0, msgSeqNum);

        auto* field = ObjectPool<NotifyComponentConnectStatusField>::GetInstance().Allocate();
        field->SessionID = kSessionID;
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
        std::string key = anchor + std::to_string(Items::Version) + "=";
        size_t pos = frame.find(key);
        if (pos == std::string::npos)
        {
            return false;
        }
        std::string origin = std::to_string(ProtocolVersionValue);
        std::string fresh = std::to_string(version);
        if (origin.size() != fresh.size())
        {
            return false;
        }
        frame.replace(pos + key.size(), origin.size(), fresh);
        return true;
    }

    // 验证解析后的包与原始值一致
    void VerifyPackage(const NotifyComponentConnectStatusPackage* parsed,
                       SessionIDType expectedSessionID,
                       int expectedMsgSeqNum)
    {
        ASSERT_NE(parsed, nullptr);
        ASSERT_NE(parsed->NotifyComponentConnectStatus, nullptr);
        EXPECT_EQ(parsed->SessionID, expectedSessionID);
        EXPECT_EQ(parsed->Head.MsgSeqNum, expectedMsgSeqNum);
        EXPECT_EQ(parsed->Head.PackageID, NotifyComponentConnectStatusPackage::PackageID);
        EXPECT_EQ(parsed->NotifyComponentConnectStatus->SessionID, kSessionID);
        EXPECT_EQ(static_cast<int>(parsed->NotifyComponentConnectStatus->Component),
                  static_cast<int>(ComponentType::TradeFront));
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
    PackageReader reader(ProtocolTypeType::Step, &factory, kSessionID, kIP);
    unsigned int appended = reader.Append(buff, totalLen);
    EXPECT_EQ((int)appended, totalLen);

    Package* parsedRaw = nullptr;
    bool parseResult = reader.ParsePackage(parsedRaw);
    EXPECT_TRUE(parseResult);
    ASSERT_NE(parsedRaw, nullptr);

    // 5. 验证字段
    auto* parsed = static_cast<NotifyComponentConnectStatusPackage*>(parsedRaw);
    VerifyPackage(parsed, kSessionID, 1001);

    // 6. 清理
    parsed->Deallocate();
}

TEST(PackageSerializationTest, StepRoundTrip_MultipleMessages)
{
    // 连续两条消息验证 Reader 状态正确
    PackageFactory factory;
    PackageReader reader(ProtocolTypeType::Step, &factory, kSessionID, kIP);

    for (int seq = 1; seq <= 3; ++seq)
    {
        auto* pkg = CreateSamplePackage(seq);
        pkg->NotifyComponentConnectStatus->IsConnected = (seq % 2 == 1);  // toggle

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
    EXPECT_GT(totalLen, (int)(sizeof(HeadField) + sizeof(TailField)));

    pkg->Deallocate();

    PackageFactory factory;
    PackageReader reader(ProtocolTypeType::Xtp, &factory, kSessionID, kIP);
    reader.Append(buff, totalLen);

    Package* parsedRaw = nullptr;
    bool parseResult = reader.ParsePackage(parsedRaw);
    EXPECT_TRUE(parseResult);
    ASSERT_NE(parsedRaw, nullptr);

    auto* parsed = static_cast<NotifyComponentConnectStatusPackage*>(parsedRaw);
    VerifyPackage(parsed, kSessionID, 1001);

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
    tail.CheckSum = 0xFF;  // 错误值
    memcpy(buff + totalLen - sizeof(TailField), &tail, sizeof(tail));

    PackageFactory factory;
    PackageReader reader(ProtocolTypeType::Xtp, &factory, kSessionID, kIP);
    reader.Append(buff, totalLen);

    Package* parsedRaw = nullptr;
    EXPECT_TRUE(reader.ParsePackage(parsedRaw));   // 不是致命错误，不断链
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
    head.Version = static_cast<UShortType>(ProtocolVersionValue + 1);
    memcpy(buff, &head, sizeof(head));

    PackageFactory factory;
    PackageReader reader(ProtocolTypeType::Xtp, &factory, kSessionID, kIP);
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
    PackageReader reader(ProtocolTypeType::Xtp, &factory, kSessionID, kIP);
    reader.Append(buff, totalLen);

    Package* parsedRaw = nullptr;
    EXPECT_TRUE(reader.ParsePackage(parsedRaw));
    EXPECT_EQ(parsedRaw, nullptr);
    EXPECT_EQ(reader.Length(), sizeof(ProtocolMagicValue) - 1);
}

TEST(PackageSerializationTest, XtpRoundTrip_GarbagePrefixIsResynced)
{
    // 前面塞一段噪声，接收端应当丢弃它们并重新对齐到真正的魔术字
    const char garbage[] = { 'n', 'o', 'i', 's', 'e', '!', '!' };

    char buff[MaxPackageSize] = {};
    int totalLen = MakeFrame(ProtocolTypeType::Xtp, buff, 1001);
    ASSERT_GT(totalLen, 0);

    PackageFactory factory;
    PackageReader reader(ProtocolTypeType::Xtp, &factory, kSessionID, kIP);
    EXPECT_EQ(reader.Append(const_cast<char*>(garbage), sizeof(garbage)), sizeof(garbage));

    Package* parsedRaw = nullptr;
    EXPECT_TRUE(reader.ParsePackage(parsedRaw));
    EXPECT_EQ(parsedRaw, nullptr);

    EXPECT_EQ(reader.Append(buff, totalLen), (unsigned int)totalLen);
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
    PackageReader reader(ProtocolTypeType::Xtp, &factory, kSessionID, kIP);
    reader.Append(corrupt, corruptLen);
    reader.Append(good, goodLen);

    Package* parsedRaw = nullptr;
    EXPECT_TRUE(reader.ParsePackage(parsedRaw));
    ASSERT_NE(parsedRaw, nullptr);
    // 越过坏帧，好帧的字段完整解出
    EXPECT_EQ(parsedRaw->Head.MsgSeqNum, 2);
    VerifyPackage(static_cast<NotifyComponentConnectStatusPackage*>(parsedRaw), kSessionID, 2);
    parsedRaw->Deallocate();
}

TEST(PackageSerializationTest, XtpRoundTrip_MagicSplitAcrossAppend)
{
    // 魔术字跨收包边界时必须一个字节都不丢
    char buff[MaxPackageSize] = {};
    int totalLen = MakeFrame(ProtocolTypeType::Xtp, buff, 1001);
    ASSERT_GT(totalLen, 0);
    // 先只送入魔术字的前两个字节
    constexpr int kSplit = 2;
    ASSERT_LT(kSplit, (int)sizeof(ProtocolMagicValue));

    PackageFactory factory;
    PackageReader reader(ProtocolTypeType::Xtp, &factory, kSessionID, kIP);
    EXPECT_EQ(reader.Append(buff, kSplit), (unsigned int)kSplit);

    Package* parsedRaw = nullptr;
    EXPECT_TRUE(reader.ParsePackage(parsedRaw));
    EXPECT_EQ(parsedRaw, nullptr);
    EXPECT_EQ(reader.Length(), kSplit);

    EXPECT_EQ(reader.Append(buff + kSplit, totalLen - kSplit), (unsigned int)(totalLen - kSplit));
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
    PackageReader reader(ProtocolTypeType::Step, &factory, kSessionID, kIP);
    reader.Append(&frame[0], (unsigned int)frame.size());

    Package* parsedRaw = nullptr;
    EXPECT_FALSE(reader.ParsePackage(parsedRaw));
    EXPECT_EQ(parsedRaw, nullptr);
}

TEST(PackageSerializationTest, StepRoundTrip_GarbagePrefixIsResynced)
{
    const char garbage[] = { 'n', 'o', 'i', 's', 'e', '!', '!' };

    char buff[MaxPackageSize] = {};
    int totalLen = MakeFrame(ProtocolTypeType::Step, buff, 1001);
    ASSERT_GT(totalLen, 0);

    PackageFactory factory;
    PackageReader reader(ProtocolTypeType::Step, &factory, kSessionID, kIP);
    EXPECT_EQ(reader.Append(const_cast<char*>(garbage), sizeof(garbage)), sizeof(garbage));

    Package* parsedRaw = nullptr;
    EXPECT_TRUE(reader.ParsePackage(parsedRaw));
    EXPECT_EQ(parsedRaw, nullptr);

    EXPECT_EQ(reader.Append(buff, totalLen), (unsigned int)totalLen);
    EXPECT_TRUE(reader.ParsePackage(parsedRaw));
    ASSERT_NE(parsedRaw, nullptr);
    VerifyPackage(static_cast<NotifyComponentConnectStatusPackage*>(parsedRaw), kSessionID, 1001);
    parsedRaw->Deallocate();
}

TEST(PackageSerializationTest, StepRoundTrip_ResyncAfterCorruptFrame)
{
    char corrupt[MaxPackageSize] = {};
    int corruptLen = MakeFrame(ProtocolTypeType::Step, corrupt, 1);
    ASSERT_GT(corruptLen, 0);
    // 改坏包体中的一个字节，包头仍然合法，只有 CRC 失配
    corrupt[corruptLen - (int)StepTailLen - 1] ^= 0x5A;

    char good[MaxPackageSize] = {};
    int goodLen = MakeFrame(ProtocolTypeType::Step, good, 2);
    ASSERT_GT(goodLen, 0);

    PackageFactory factory;
    PackageReader reader(ProtocolTypeType::Step, &factory, kSessionID, kIP);
    reader.Append(corrupt, corruptLen);
    reader.Append(good, goodLen);

    Package* parsedRaw = nullptr;
    EXPECT_TRUE(reader.ParsePackage(parsedRaw));
    ASSERT_NE(parsedRaw, nullptr);
    EXPECT_EQ(parsedRaw->Head.MsgSeqNum, 2);
    VerifyPackage(static_cast<NotifyComponentConnectStatusPackage*>(parsedRaw), kSessionID, 2);
    parsedRaw->Deallocate();
}

TEST(PackageSerializationTest, StepRoundTrip_AnchorSplitAcrossAppend)
{
    char buff[MaxPackageSize] = {};
    int totalLen = MakeFrame(ProtocolTypeType::Step, buff, 1001);
    ASSERT_GT(totalLen, 0);

    int anchorLen = (int)StepUtility::GetPackageStartAnchor().size();
    constexpr int kSplit = 5;
    ASSERT_LT(kSplit, anchorLen);

    PackageFactory factory;
    PackageReader reader(ProtocolTypeType::Step, &factory, kSessionID, kIP);
    EXPECT_EQ(reader.Append(buff, kSplit), (unsigned int)kSplit);

    Package* parsedRaw = nullptr;
    EXPECT_TRUE(reader.ParsePackage(parsedRaw));
    EXPECT_EQ(parsedRaw, nullptr);
    EXPECT_EQ(reader.Length(), kSplit);

    EXPECT_EQ(reader.Append(buff + kSplit, totalLen - kSplit), (unsigned int)(totalLen - kSplit));
    EXPECT_TRUE(reader.ParsePackage(parsedRaw));
    ASSERT_NE(parsedRaw, nullptr);
    VerifyPackage(static_cast<NotifyComponentConnectStatusPackage*>(parsedRaw), kSessionID, 1001);
    parsedRaw->Deallocate();
}

// ============================================================
// Prepare 方法测试
// ============================================================

TEST(PackageSerializationTest, PrepareSetsFields)
{
    NotifyComponentConnectStatusPackage pkg;
    pkg.Prepare(kSessionID, 1, 999);

    EXPECT_EQ(pkg.SessionID, kSessionID);
    EXPECT_EQ(pkg.Head.MsgSeqNum, 999);
    EXPECT_EQ(pkg.Head.MessageChain, 1);
    EXPECT_EQ(pkg.Head.PackageID, NotifyComponentConnectStatusPackage::PackageID);
}

// ============================================================
// 空字段（nullptr）包序列化
// ============================================================

TEST(PackageSerializationTest, EmptyField_NullBody)
{
    // 字段为 nullptr 时 MakePackage 应仍能产生有效报文（body 为空）
    auto* pkg = NotifyComponentConnectStatusPackage::Allocate();
    pkg->Prepare(kSessionID, 0, 1);
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
    pkg.Prepare(kSessionID, 0, 1);

    char buff[MaxPackageSize] = {};
    buff[0] = 'X';
    // Xtp 的长度契约在写报文头之前判定，缓冲一个字节都不该动
    EXPECT_EQ(pkg.MakePackage(ProtocolTypeType::Xtp, buff, MaxPackageSize), 0);
    EXPECT_EQ(buff[0], 'X');

    // Step 的包头要先落盘才量得出包头长度，判定在其后，只要求拒绝发送
    EXPECT_EQ(pkg.MakePackage(ProtocolTypeType::Step, buff, MaxPackageSize), 0);
}
