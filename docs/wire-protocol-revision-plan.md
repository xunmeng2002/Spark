# Spark 线协议报文头改造方案（v2）

本文针对 `Spark/src/Network/Protocol/` 的报文头设计给出改造方案。结论先行：当前报文头只有 4 个字段、无同步字、无版本、校验和只有 8 位有效位，**问题的重心不是"网络传坏了检不出来"，而是"两端不是同一个构建时无法被发现"**。改造的目标是把隐式的 ABI 约定变成显式的、可校验的线上契约。

本文是方案与计划，不含代码改动。所有"现状"结论均标注文件与行号，便于逐条复核。

## 1. 背景与目标

Xtp 路径当前的报文头是把一个 native C++ 结构体原样 `memcpy` 上线的：

```cpp
// Spark/src/Network/Protocol/Head.h（由 Model/Head.xml 生成）
class HeadField
{
public:
    static constexpr UShortType FieldID = 0x0001;
    UShortType PackageID;    // 报文代码
    UShortType BodyLen;      // 报文长度
    BoolType   MessageChain; // 报文链标记
    IntType    MsgSeqNum;    // 请求编号
};
```

`Package::MakePackage` 直接 `memcpy(buff, &Head, sizeof(Head))`（`Spark/src/Network/Protocol/Package.cpp:27`），接收侧 `memcpy(&m_Head, m_Data, sizeof(HeadField))`（`Spark/src/Network/Protocol/PackageReader.cpp:98`）。两端用同一份生成的 `Head.h`，所以能对上。

这个做法成立的前提有四个，且**全部是隐式的、没有任何检查**：

1. 两端 `sizeof(HeadField)` 相同。
2. 两端结构体填充字节位置相同。
3. 两端字节序相同（小端）。
4. 两端每个字段的语义相同。

任何一个不成立，故障现象是**校验和不过**或**字段读到错位的值**，而不是"协议版本不匹配"。代码里到处是 `Please Check ApiVersion` 的日志（`Spark/src/Network/Protocol/PackageReader.cpp:113`、`QuantTrading/src/Packages/Packages.cpp:103` 等），但**线上没有任何版本字段可查**——这句话目前是一句无根据的猜测。

本次改造目标：

- 用一个固定 magic 让"帧边界"可判定、可重同步。
- 用一个版本字段让版本不匹配变成**明确、可日志、可运维**的错误。
- 把结构体布局从"编译器说了算"变成"编译期断言保证"。
- 把校验和从 8 位有效位提升到能检出字节重排的水平。
- **不动包体格式，不动业务代码。**

非目标（本次不做）：包体 schema 演化机制、分片重组、协议协商握手、加密与认证。见第 11 节。

## 2. 现状事实

### 2.1 报文头与报文尾

| 项 | 值 | 证据 |
| :--- | :--- | :--- |
| `sizeof(HeadField)` | 12 字节（含 3 字节填充） | 2+2+1+ 填充 3 +4 |
| `sizeof(TailField)` | 2 字节 | 仅一个 `UShortType CheckSum` |
| 单帧固定开销 | 14 字节 | `Package.cpp:31` |
| `BodyLen` 上限 | 65535（`UShortType` 的天然上限） | `Head.h:12` |
| `PackageID` 空间 | 65536，当前用 52 个 | `QuantTrading/src/Packages/PackageFactory.cpp` |

字段布局（小端）：

```text
偏移  长  字段          说明
0     2   PackageID     报文代码
2     2   BodyLen       体长，不含头尾
4     1   MessageChain  报文链标记
5     3   <填充>        编译器插入，无意义
8     4   MsgSeqNum     请求编号
```

### 2.2 两条路径的实际帧布局

**Xtp**（`Package.cpp:23-32`、`PackageReader.cpp:94-131`）：

```text
[HeadField 12][Body: BodyLen][TailField 2]
```

**Step**（`Package.cpp:33-40`、`PackageReader.cpp:132-190`）：

```text
[SOH][1=XXXX][SOH][2=NNNNN][SOH][3=NNNNNNNNNNNNN][SOH][4=N][SOH]
[Body: 6=X ... 7=X 字段区]
[5=NNNNN][SOH]
```

Step 头长被硬编码为 `StepHeaderLen = 36`（`Spark/include/Spark/Network/Protocol/StepUtility.h:8`），它是 `HeadToStream` 里四个 `sprintf` 格式串的**手工镜像**（`StepUtility.cpp:167-176`）。任何一处格式串或值宽度变化都会静默破坏这个常量。

### 2.3 校验和

```cpp
// Spark/src/Network/Protocol/ProtocolUtility.cpp:6-14
short CalculateSum(unsigned char* buff, int len)
{
    int sum = 0;
    for (auto i = 0; i < len; ++i) { sum += buff[i]; }
    return sum % 256;
}
```

逐字节累加后取模 256，即**8 位有效位**，却占用了 2 字节的 `UShortType CheckSum`。

### 2.4 生成链路

报文头本身是模型生成的，这决定了改动的形态：

```text
Gitee/Model/Types.xml  ──┐
Gitee/Spark/Model/Head.xml ─┤
Gitee/Templates/Cpp/Spark/Network/Protocol/Head.h.tpl
    │
    └── pumpall.py ──> Spark/include/Spark/Network/Protocol/Head.h
```

`Gitee/Spark/pumplist.xml:5` 是这条链的声明。`Gitee/Model/` 是一个**独立仓库**，`Types.xml` / `Items.xml` / `ShortItems.xml` 都在里面。

### 2.5 遗留文件

`Gitee/Spark/Model/XtpHead.xml` 定义了只有 `PackageID` + `BodyLen` 两项的头，但**没有任何 pumplist 或脚本引用它**（`grep` 全仓仅命中自身）。这是早期 Xtp 独立头设计的残留，先于 `Head.xml` 的统一头。建议在本次改造中一并删除或明确标注为废弃——但按 Harness §1，删除动作需你单独确认。

## 3. 问题清单

按"改完之后能消掉"的顺序排列。

| # | 问题 | 现状证据 | 后果 |
| :--- | :--- | :--- | :--- |
| P1 | 无帧同步字 | 两条路径都没有 magic | 一个坏字节后无法重新对齐；`Reset()` 把整段缓冲清零（`PackageReader.cpp:108/114/167/173`），夹在好包中间的一个坏包会连带丢掉已缓冲的全部好包 |
| P2 | 无协议版本字段 | `Head.xml` 无版本项 | 两端构建不一致时，表现为校验和不过或字段错位，日志只能喊 `Please Check ApiVersion` |
| P3 | 布局无编译期保证 | 无 `static_assert` | 改一个字段、换一个编译器或平台，线上格式静默改变，无一处编译错误 |
| P4 | 校验和仅 8 位有效 | `ProtocolUtility.cpp:13` | 检不出字节重排；随机错误的漏检率约 1/256 |
| P5 | 结构体填充被当作无意义字节 | 3 字节填充在偏移 5..7 | 填充内容不受协议约束，也不参与校验之外的任何语义；语义随编译器变化 |
| P6 | `StepHeaderLen` 手工镜像 | `StepUtility.h:8` | 格式串一改就错位 |
| P7 | 无方向/角色标识 | 方向由本地 `IsInboundPackageAccepted` 判定（`QuantTrading/src/Packages/PackageFactory.cpp:230`） | 配错方向的节点被静默断连，而非明确报错 |

P1 与 P2 是本方案的主要目标；P3 是**必须同时做**的，否则改了等于没改；P4 是顺带；P5 由 P3 一并解决（改成零填充）。

## 4. 外部参考

### 4.1 对比表

| 协议 | 帧头 | 同步字 | 版本 | 校验 | 备注 |
| :--- | :--- | :--- | :--- | :--- | :--- |
| Bitcoin P2P | 24 字节 | magic 4 字节 | 无 | SHA256d 前 4 字节 | 头部 4/12/4/4；靠扫 magic 重同步 |
| FIX 4.4 | 文本 | `8=FIX.4.4` | 在 BeginString 里 | `10=` 3 位十进制，mod 256 | 规定校验顺序；扫 `8=` 重同步 |
| SBE | 8 字节 | 无 | schemaId + version | 无 | 4 个 uint16；面向 schema 演化 |
| Kafka | 4 字节长度 + 头 | 无 | api_version | 无 | 每请求声明版本，不支持则报 UNSUPPORTED_VERSION |
| MoldUDP64 / ITCH | 20 字节 + 块 | 无 | 无 | 无（依赖 UDP 校验） | 消息长 uint16；0=心跳，0xFFFF=收市 |
| memcached 二进制 | 24 字节 | magic 1 字节 | 无 | 无 | magic 同时编码**方向**（0x80 请求 / 0x81 响应） |
| ZMTP 3.0 | greeting 10 字节 | `0xFF` + 8 填充 + `0x7F` | 有 | 无 | 签名足够独特，可直接扫 |

### 4.2 四条可复用的结论

**结论一：magic 的价值是"重同步锚点"，而在这个项目里它的附加价值是"构建一致性检测"。**

Bitcoin 的 magic 用于区分主网/测试网，接收端扫 magic 来重新对齐。本项目的对端只有自己，所以 magic 区分网络没有意义；真正有意义的是：**magic 是"这 4 个字节符合预期"的第一次检查，它能在解析长度字段之前就把"两端不是一个构建"这种情况挑出来。**

**结论二：TCP 之上的逐帧校验和，主要在检出实现缺陷，而不是网络损坏。**

Bitcoin Core 自己认为 payload 校验和已经过时——TCP 已有校验，验证坏校验和是浪费 CPU，并用 `XVERSION` 扩展声明"我不会验证"。这条对本项目同样成立：Xtp 走的是内部 TCP，逐帧校验和的实际职责是**检出"结构体大小不一致 / 字段偏移不一致 / 分帧错位"**这一类实现缺陷。所以选算法的标准是"便宜且能检出系统性错误"，不是密码学强度。

**结论三：版本字段的正确位置是帧头，而不是握手。（至少第一步）**

Kafka 每个请求都带 `api_version`，broker 不支持就返回 `UNSUPPORTED_VERSION`；FIX 把版本写进 BeginString；SBE 把 `schemaId` + `version` 放进 8 字节头。三者都不依赖单独的协商握手。对本项目，第一步只需要**每帧带版本 + 不符时明确报错断连**，握手是第二步。

**结论四：`BodyLen` 保持 16 位是对的，不该加宽。**

MoldUDP64 的消息长度就是 uint16，上限 65535。本项目的包体是快照级的（`RtnDepthMarketDataField` 约 200 字节），16 位绰绰有余。更重要的是：**16 位的 `BodyLen` 天然把单帧上限锁在 65551 字节，而 `PackageReader` 的窗口是 131072 字节**，所以"等到够长才解析"这条路径必然收敛，不存在被伪造长度卡死窗口的可能。加宽到 32 位反而会引入这个问题，并且要额外写溢出检查。

反过来，FIX 的**校验顺序**值得照抄（见 6.3）。

## 5. 设计目标与约束

约束来自 Harness 与项目现状：

- 不引入新依赖（§2）：CRC 用查表实现，不用第三方库。
- 不改包体格式：`Templates/Cpp/Protocol/Xtp/` 与 `Templates/Cpp/Protocol/Packages/` 不动。
- 不改业务代码：`HeadField` 的**成员名与类型保持不变**，`QuantTrading` 侧 48 处 `Head.MessageChain`、73 处 `Head.MsgSeqNum`、63 处 `Head.PackageID` 全部零改动编译通过。
- 不新增模型基本类型：`Gitee/Model/Types.xml` 只有 `bool / ushort / int / int64 / double / string / enum` 七组，新增 `uint`/`ubyte` 组会波及所有消费 `Types.xml` 的代码生成器。本方案只用现有类型。
- 单帧固定开销增幅控制在 6 字节以内。

## 6. 推荐方案（v2）

### 6.1 帧布局

**报文头改为 16 字节，零填充，字段重排以消除填充：**

| 偏移 | 长度 | 字段 | 类型 | 说明 |
| :--- | :--- | :--- | :--- | :--- |
| 0 | 4 | `Magic` | `Int` | 固定 `0x324B5053`（线上小端字节 `53 50 4B 32`，即 `SPK2`） |
| 4 | 4 | `MsgSeqNum` | `Int` | 请求编号，**不变** |
| 8 | 2 | `PackageID` | `UShort` | 报文代码，**不变** |
| 10 | 2 | `BodyLen` | `UShort` | 体长，**不变** |
| 12 | 2 | `Version` | `UShort` | 协议版本，本次为 `2` |
| 14 | 1 | `MessageChain` | `Bool` | **不变** |
| 15 | 1 | `Reserved` | `Bool` | 保留，必须为 `false` |

`4+4+2+2+2+1+1 = 16`，所有成员自然对齐，**无任何填充字节**。原来偏移 5..7 的 3 字节填充被回收，新增字段的实际净成本只有 3 字节。

**报文尾改为 4 字节：**

| 偏移 | 长度 | 字段 | 类型 | 说明 |
| :--- | :--- | :--- | :--- | :--- |
| 0 | 4 | `CheckSum` | `Int` | CRC32C（Castagnoli），范围 = 头 16 字节 + 体 `BodyLen` 字节 |

**单帧开销从 14 字节变为 20 字节**（+6）。对一个 300 字节的行情帧是 +2%。

**关键性质：`PackageID`、`BodyLen`、`MessageChain`、`MsgSeqNum` 四个成员的名称与类型一字未改**，只是顺序调整、插入两个新成员。`QuantTrading` 与 `Spark/test/` 的所有访问点无需修改。

### 6.2 校验算法

从"逐字节和 mod 256"换成 **CRC-32C**：

- 多项式（反射形式）`0x82F63B78`，初值 `0xFFFFFFFF`，末尾异或 `0xFFFFFFFF`。
- 基线实现用 **256 项查表**（`Spark/src/Network/Protocol/ProtocolUtility.cpp` 新增 `CalculateCrc32c`），无新增依赖、无编译期架构要求。
- 可选后续优化：`#if defined(__SSE4_2__)` 走 `_mm_crc32_u32`，ARM64 走 `__crc32cd`（`<arm_acle.h>`，ARMv8-A 基线）。**不要**在没有运行时检测的情况下无条件用内建指令，否则在不支持 SSE4.2 的机器上会非法指令崩溃。

为什么不是更便宜的 `Fletcher-16`：Fletcher-16 同样能检出字节重排，且能塞进现有 2 字节的 `UShort CheckSum`（改动更小）。但它需要 4 处比较逻辑全改、检出能力仍弱于 CRC32C，而 CRC32C 只多 2 字节。**如果你希望把报文尾的改动也省掉，这是一个可以接受的降级选项**，见第 7 节。

校验和的取值**不参与**任何密钥或安全判定，只用于一致性检查。

### 6.3 校验顺序

照抄 FIX 的规定顺序，逐级上升——**在分配任何对象或解析任何包体之前，先把帧本身验完**：

```text
1. 缓冲长度 >= 16（头长）            → 否则等待更多数据
2. Magic == 0x324B5053               → 不符：进入重同步（见 6.4），不直接断连
3. Version == kProtocolVersion       → 不符：明确报错并断连（见 6.5）
4. 缓冲长度 >= 16 + BodyLen + 4      → 否则等待更多数据
5. CheckSum == CRC32C(头 + 体)       → 不符：报错 + 重同步
6. IsInboundPackageAccepted(ID)      → 不符：报错 + 断连
7. CreatePackage + FromXtpStream     → 失败：回收 package + 断连
```

第 3 步放在第 4 步之前是关键：版本不符时，`BodyLen` 的语义本身就不可信，不该拿它算长度。

### 6.4 重同步

替换掉现在的 `Reset()`（清空整段缓冲）：

```text
FindNextMagic(buff, length, outOffset):
    在 [0, length - 4) 内找第一个 Magic
    找到   -> PopFront(outOffset)，丢弃前面所有字节，重新开始解析
    没找到 -> 保留最后 3 字节（magic 可能跨 recv 边界），丢弃其余
              若保留前的长度已接近窗口上限，报错并断连
```

三点设计说明：

- 保留 3 字节而不是 4，是因为下一次 `recv` 可能从 magic 的中间接上。
- "没找到"时**不**报错断连——正常情况下一段缓冲里没有完整 magic 是可能的（例如刚收到 2 个字节）。
- 只有"缓冲已经很大且仍找不到 magic"才判定为垃圾流并断连，避免用无意义的字节把 131072 的窗口撑满。

这一条直接消掉 P1 的后半段：一个坏包不再连带丢掉同批的好包。

### 6.5 版本不符的处理

- 每帧都校验 `Version`。
- 不符时**立刻断连**，并且**每个会话只记一次日志**，日志里带上对端版本与本端版本。

现在的行为是"版本不符 → 校验和/解析失败 → 断连 + 一堆误导性错误"，改完后日志能直接说明问题。

**第二步（不在本次范围）**：把 `NotifyConnect` / `NotifyDisConnect` 升级成真正的线上握手，首帧交换 `{Version, Magic, BuildStamp}`，服务端据此给出 `RspProtocolHello` 级别的明确拒绝。这一步需要放宽 `IsInboundPackageAccepted`——当前所有 `Notify*` 包入站一律返回 `false`（`QuantTrading/src/Packages/PackageFactory.cpp:235-250`）。对"两端都是同一套代码"的系统，完整协商是过度设计，一次性的版本声明足够。

### 6.6 编译期不变量

这是本方案里**最便宜、收益最大**的一条。在生成的 `Head.h` 里加断言：

```cpp
static_assert(sizeof(HeadField) == 16,
    "HeadField 布局变化会改变线上格式，必须同步升级 kProtocolVersion");
static_assert(sizeof(TailField) == 4,
    "TailField 布局变化会改变线上格式");
static_assert(sizeof(HeadField) + sizeof(TailField) == 20,
    "单帧固定开销必须是 20 字节");
```

再加一条字节序断言（本项目只支持小端）：

```cpp
static_assert(static_cast<unsigned>(0x01020304u) != 0x01020304u,
    "线协议依赖小端字节序");
```

这样 P3 从"没有保证"变成"改错就编译不过"。**注意：断言要加在模板里，让生成物自带**，否则下次重新 pump 就丢了。

### 6.7 Step 路径的对应改法

Step 是文本协议，FIX 的答案是 `8=FIX.4.4` 作为包首的文本 magic。对应做法：

- 在 `Gitee/Model/ShortItems.xml` 新增 `Items::Magic = 0x0000` 与 `Items::Version = 0x0008`，重新跑 `Model/parselist.xml` 生成 `Items.h`。
- `HeadToStream` 先写 magic（`0=SPK2`）再写版本。
- `GetPackageStart` 的锚点从 `SOH` + `1` + `=` 改为 `SOH` + `0` + `=` + `SPK2` + `SOH`。现在的锚点是一字节 SOH 加两个非常常见的字符，几乎不具选择性；改后是一个 4 字节字面量。
- **`StepHeaderLen` 必须改成由 `HeadToStream` 返回实际长度**，不能再硬编码 36（消掉 P6）。
- 校验和同样换 CRC32C，以十六进制写入报尾。

Step 当前**无任何调用方**——`QuantTrading` 四处 `Protocol` 构造点全部传 `ProtocolTypeType::Xtp`。所以 Step 改造可以与 Xtp 同批做，也可以延后；我建议同批做，因为改的都是同一批文件（`StepUtility.cpp`、`Head.xml`、`Items.xml`），分两批反而要动两次模型与生成物。

## 7. 备选与取舍

| 档位 | 内容 | 单帧开销 | 改动面 | 适用 |
| :--- | :--- | :--- | :--- | :--- |
| **A（推荐）** | 16 字节头 + CRC32C 尾 | 20 | 中 | 一次到位，消掉 P1~P6 |
| **B（最小）** | 16 字节头 + 保留 2 字节 `Fletcher-16` | 18 | 小 | 不想动 `TailField` 的类型时 |
| **C（仅加 magic）** | 在现有 12 字节头后追加 magic + 版本 | 22 | 小 | 只想要版本检测，接受填充浪费 |
| **D（无校验和）** | 16 字节头，去掉报尾，依赖 TCP | 16 | 中 | 与 Bitcoin 的结论一致，但会失去"两端结构体不一致"的检出能力 |

**方案 C 不推荐**：它保留了 3 字节填充，`sizeof` 变成 22 或 24 都取决于编译器，等于把 P3 和 P5 留下来，而这两条恰恰是最该修的。

**方案 D 值得认真考虑但本方案不推荐**：Bitcoin 的结论（TCP 之上逐帧校验和收益低）在这个项目里不成立，因为这里要防的是**实现缺陷**而非网络损坏——两个用不同 `Head.h` 构建的进程互相通信时，唯一能拦住它的是 magic + 校验和。magic 只覆盖头 4 字节，包体的结构性错位要靠 CRC 才能发现。

## 8. 改动清单

按仓库分组。**模型仓库是独立 git 仓库**，改动有独立的提交边界。

### 8.1 `Gitee/Model`（共享模型仓库）

| 文件 | 改动 |
| :--- | :--- |
| `Types.xml` | 不改（本方案只用现有类型） |
| `ShortItems.xml` | 新增 `Magic`、`Version` 两个 item（仅 Step 需要） |
| `Items.xml` | 由 `parselist.xml` 重新生成，不手改 |

### 8.2 `Gitee/Spark/Model`

| 文件 | 改动 |
| :--- | :--- |
| `Head.xml` | 按 6.1 重排 `Head` 的 item 顺序，新增 `Magic`/`Version`/`Reserved`；`Tail.CheckSum` 由 `UShort` 改 `Int` |
| `XtpHead.xml` | 建议删除或标注废弃（见 2.5，需你确认） |

### 8.3 `Gitee/Templates`

| 文件 | 改动 |
| :--- | :--- |
| `Cpp/Spark/Network/Protocol/Head.h.tpl` | 追加 6.6 的 `static_assert` 块 |
| `Cpp/Protocol/Xtp/*` | 不改（包体格式不变） |
| `Cpp/Protocol/Step/*` | 不改（包体格式不变） |
| `Cpp/Protocol/Packages/*` | 不改 |
| `Cpp/Protocol/Kernel/*` | 不改 |

### 8.4 `Gitee/Spark`（代码）

| 文件 | 改动 |
| :--- | :--- |
| `include/Spark/Network/Protocol/Head.h` | 重新 pump 生成 |
| `include/Spark/Network/Protocol/Items.h` | 重新 pump 生成（Step 用） |
| `src/Network/Protocol/ProtocolUtility.cpp/.h` | 新增 `CalculateCrc32c`；`CalculateSum` 保留还是删除待定 |
| `src/Network/Protocol/Package.cpp` | `MakePackage` 写入 `Magic`/`Version`；改用 `CalculateCrc32c` |
| `src/Network/Protocol/PackageReader.cpp` | 按 6.3 重写校验顺序；新增 `FindNextMagic` 重同步 |
| `src/Network/Protocol/StepUtility.cpp/.h` | Step 头的 magic/版本；`StepHeaderLen` 改为返回值 |
| `test/TestClient/*`、`test/TestServer/*` | 追加畸形帧测试（见第 10 节） |

### 8.5 `Gitee/QuantTrading`

**零改动。** 依据：`HeadField` 四个原成员的名称与类型未变；`Tail.CheckSum` 在 `QuantTrading` 全仓引用次数为 0（已 grep 确认）。

唯一需要重建的是构建产物——Xtp 是纯内部协议，两端必须同批升级。

## 9. 分期实施计划

| 期 | 内容 | 依赖 | 风险 |
| :--- | :--- | :--- | :--- |
| **P0** | 上一轮评估发现的三个缺陷：接收 Buffer 泄漏、Step 的 `stoi` 无保护、解析失败路径 package 未回收 | 无 | 低。与协议格式无关，可独立验证 |
| **P1** | 模型与模板：`Head.xml` 重排 + 新字段 + `static_assert` + 重新 pump | 无 | 低。生成物 diff 可逐行审 |
| **P2** | `MakePackage` / `ParseXtpPackage`：magic + 版本 + 校验顺序 + 重同步 | P1 | 中。**这是协议不兼容点**，两端必须同批 |
| **P3** | 校验和换 CRC32C | P2 | 低 |
| **P4** | Step 路径对齐（`Items.xml`、`StepUtility`、`StepHeaderLen`） | P3 | 低。无调用方 |
| **P5**（可选） | 线上握手 + `RspProtocolHello`；`FindNextMagic` 的内建指令优化 | P4 | 中。需要放宽 `IsInboundPackageAccepted` |

**P0 与 P1 可以并行**，且建议先做 P0——它是当前唯一会打崩进程的活路径缺陷，与协议改造无关。

**P2 是必须一刀切的点。** 上线顺序必须是"停全部进程 → 部署新版本 → 启动"，不能滚动升级。如果未来需要滚动升级，得走第 7 节的方案 C 或引入双栈，成本会显著上升。

## 10. 验收标准

现有测试架子可直接用：`Spark/test/TestClient/TestXtpClient.cpp`、`TestStepClient.cpp` 与 `Spark/test/TestServer/`（`test/CMakeLists.txt` 里已注册 `TestClient` / `TestServer` 两个 target）。

| # | 用例 | 期望 |
| :--- | :--- | :--- |
| 1 | 正常收发包 | 与改造前行为一致，`OnMessage` 字段值不变 |
| 2 | CRC32C 标准向量 | `CalculateCrc32c("123456789", 9) == 0xE3069283` |
| 3 | 伪造 magic | 字节 `53 50 4B 33` | 报"Magic 不符"，进入重同步，**不断连** |
| 4 | 版本不符 | `Version = 1` | 明确报版本不符并断连，日志含两端版本 |
| 5 | 篡改任意 1 字节 | 报 CRC 不符，重同步后下一帧正常解析 |
| 6 | 单坏包夹在好包中间 | **好包不丢**（这是 P1 的核心验收点） |
| 7 | 连续垃圾流（无 magic） | 达到窗口阈值后断连，不无限占用内存 |
| 8 | magic 跨 recv 边界 | 分两次发送，仍能正确对齐 |
| 9 | `BodyLen` 篡改 | 不卡死窗口，超时后重同步 |
| 10 | 包体为空 | 与改造前一致（生成的 `Req*` 空指针卫兵仍然生效） |
| 11 | `static_assert` | 人为改 `Head.xml` 加一个字段 → **编译失败** |

第 11 条是 P3 的验收方式：不加断言就能编译通过，加了断言就必须失败。

## 11. 风险

| 风险 | 说明 | 缓解 |
| :--- | :--- | :--- |
| 协议不兼容是硬切换 | 新老版本无法互操作，无协商 | P2 上线时全停全起；`Version` 字段保证老版本会被明确拒绝而不是静默错乱 |
| CRC 实现写错 | CRC 的参数（多项式、反射、初值、末异或）易错 | 用用例 2 的标准向量锁住；不要手改多项式 |
| 模型改动波及面 | `Head.xml` 被 `Head.h.tpl` 消费，改动会改变所有包的固定开销 | 重新 pump 后 `git diff` 逐个生成物复核；包模板未动，包体字节不变 |
| 断言被重新 pump 冲掉 | 断言若写在生成物里而非模板里会丢失 | 断言必须落在 `Head.h.tpl`（见 8.3） |
| 内存序假设 | 协议依赖小端，依赖 `BoolType` 为 1 字节 | 6.6 的两条 `static_assert` 把它变成编译期约束 |
| P0 的 Buffer 泄漏诊断未闭环 | 上一轮评估中该结论与"系统平时能跑"冲突，我未能从代码里找到释放点 | 建议 P0 开工前先实测一次 RSS 增长曲线确认 |

## 12. 待决策项

1. **校验和档位**：CRC32C（方案 A，`TailField` 改 4 字节）还是 Fletcher-16（方案 B，`TailField` 不动）？
2. **是否同批改 Step**：Step 当前无调用方，可延后；但同批做只需动一次模型。
3. **`XtpHead.xml` 是否删除**？它是无引用的残留文件，删除动作按 Harness §1 需你确认。
4. **`CalculateSum` 是否保留**？换 CRC32C 后它没有调用方；保留会变成死代码（违反 §5 的 DRY 精神），删除属公开 API 变更（Harness §3 第 1 条）。
5. **是否删除 3 字节填充以外的兼容包袱**：本方案让 `QuantTrading` 零改动，代价是 `Reserved` 用了 `Bool` 这个语义不太贴合的类型。若你愿意接受"在 `Gitee/Model/Types.xml` 新增 `uint`/`ubyte` 两组类型"的更大改动，可以换成语义正确的 `UByteType Reserved`。我倾向不改——新增基本类型会波及所有消费 `Types.xml` 的生成器。
6. **是否要 P5 的线上握手**？我建议先不做，用每帧版本 + 明确报错覆盖 90% 的运维需求。
7. **`kProtocolVersion` 放在哪里**？建议放 `Spark/include/Spark/Network/Protocol/ProtocolVersion.h`（手写，非生成），这样版本号的变更点与模型解耦。

## 附录 A 参考协议字段速查

**Bitcoin P2P**（24 字节头）

```text
0   4  magic       网络标识
4  12  command     ASCII，NUL 填充
16  4  length      payload 长度，小端
20  4  checksum    SHA256d(payload) 前 4 字节
```

**FIX 4.4**（文本，SOH 分隔）

```text
8=BeginString   必须是第一个字段
9=BodyLength    第二个字段
35=MsgType      第三个字段
...             其它头字段
10=CheckSum     最后一个字段，3 位十进制，字节和 mod 256
```

**SBE**（8 字节头，小端）

```text
0  2  blockLength  根块长度
2  2  templateId   消息模板 ID
4  2  schemaId     schema ID
6  2  version      schema 版本
```

**Kafka**（4 字节长度前缀 + 头）

```text
0  4  Size          后续内容长度，不含自身
4  2  api_key
6  2  api_version
8  4  correlation_id
12 .. client_id     int16 长度前缀
```

**MoldUDP64**（20 字节头，大端，后接块）

```text
0  10  Session       会话标识
10  8  SequenceNo    首个消息的序号
18  2  MessageCount  0=心跳，0xFFFF=收市
-- 块 --
0   2  MessageLength 后续消息字节数
```

## 附录 B 改造前后对照

```text
改造前（Xtp，单帧 14 字节开销）
[PackageID 2][BodyLen 2][Chain 1][填充 3][MsgSeqNum 4][Body][CheckSum 2]

改造后（Xtp，单帧 20 字节开销）
[Magic 4][MsgSeqNum 4][PackageID 2][BodyLen 2][Version 2][Chain 1][Rsv 1]
[Body]
[CRC32C 4]
```

```text
改造前（Step，头长硬编码 36）
SOH 1=XXXX SOH 2=NNNNN SOH 3=NNNNNNNNNNNNN SOH 4=N SOH

改造后（Step，头长由 HeadToStream 返回）
SOH 0=SPK2 SOH 8=N SOH 1=XXXX SOH 2=NNNNN SOH 3=NNNNNNNNNNNNN SOH 4=N SOH
```
