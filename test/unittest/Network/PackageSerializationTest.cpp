#include <gtest/gtest.h>
#include <Spark/Network/Protocol/PackageReader.h>
#include <Spark/Network/Protocol/PackageFactory.h>
#include <Spark/Network/Protocol/StepUtility.h>
#include "Packages.h"
#include <Spark/TemplateLib/ObjectPool/ObjectPool.h>

#include <cstring>

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
    NotifyComponentConnectStatusPackage* CreateSamplePackage()
    {
        auto* pkg = NotifyComponentConnectStatusPackage::Allocate();
        pkg->Prepare(kSessionID, 0, 1001);

        auto* field = ObjectPool<NotifyComponentConnectStatusField>::GetInstance().Allocate();
        field->SessionID = kSessionID;
        field->Component = ComponentType::TradeFront;
        field->IsConnected = true;

        pkg->NotifyComponentConnectStatus = field;
        return pkg;
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
    EXPECT_GT(totalLen, static_cast<int>(StepHeaderLen + StepTailLen));

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
        auto* pkg = NotifyComponentConnectStatusPackage::Allocate();
        pkg->Prepare(kSessionID, 0, seq);

        auto* field = ObjectPool<NotifyComponentConnectStatusField>::GetInstance().Allocate();
        field->SessionID = kSessionID;
        field->Component = ComponentType::TradeFront;
        field->IsConnected = (seq % 2 == 1);  // toggle
        pkg->NotifyComponentConnectStatus = field;

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
    // 验证校验和：修改尾部的 CheckSum 后解析应失败
    auto* pkg = CreateSamplePackage();

    char buff[MaxPackageSize] = {};
    int totalLen = pkg->MakePackage(ProtocolTypeType::Xtp, buff, MaxPackageSize);
    pkg->Deallocate();

    // 篡改校验和
    int tailOffset = totalLen - sizeof(TailField);
    TailField* tail = reinterpret_cast<TailField*>(buff + tailOffset);
    tail->CheckSum = 0xFF;  // 错误值

    PackageFactory factory;
    PackageReader reader(ProtocolTypeType::Xtp, &factory, kSessionID, kIP);
    reader.Append(buff, totalLen);

    Package* parsedRaw = nullptr;
    EXPECT_FALSE(reader.ParsePackage(parsedRaw));  // 应因校验和失败而 Reset
    EXPECT_EQ(parsedRaw, nullptr);
    EXPECT_EQ(reader.Length(), 0);  // Reset 后缓冲清空
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
