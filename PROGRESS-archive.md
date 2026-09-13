# Spark 进度归档

主文件：`PROGRESS.md`。本文件保存已关闭条目的**原文**，只移动不删改，按 ID 倒序分段。

## ❓ 原待讨论 / 待决策

### Q.15

- **8 位整型是否补进类型模板（2026-09-13 记，未决）**：别名调色板只有一个来源（`../Model/Types.xml` 的容器 + `../Templates/Cpp/Spark/Types.h.tpl` 的分段），而字段按**别名名**引用，所以"多一个宽度"看似只要两处；但真正决定宽度语义的是 **14 个模板**里按族展开的 `types[@name]` / `formats[@name]` 映射（`Protocol/Packages/Packages.cpp.tpl` 的 `formats` 直接决定 Step 文本的格式化符，现为 `%d` / `%u` / `%lld` / `%f` / `%s`）。**主要陷阱**：Step 反序列化按 `$type` 分支，未列出的族一律落到 `else: atoi(value.c_str())`——加个 `int8_t` 会走进去，编译通过、小值正确、**超范围静默截断**（`"300"` → 44）；Xtp 是按宽度 memcpy，加别名反而"看着对"，于是错在文本那一侧。**实测在用宽度只有 5 种**：模型里 `sqltype` 仅出现 `bool` / `short` / `int` / `bigint` / `double`，没有 8 位字段。要补就得四条链一起（Types 容器 + 14 处映射含 `%hhd` + Step 范围夹取 + MDB 的 `tinyint` 与 `FieldType` 映射），**建议等真出现 0–255 的新字段再做**（新字段，不涉及改已有宽度）。

关闭（2026-09-13）：用户决定本批就补，并把 `Types.xml` 统一成 `Int32`/`UInt32` 风格；上条列出的四条链已全部落地（含 Step 的 `ParseInteger` 范围校验），详见 `PROGRESS.md` 的「类型调色板补齐 8/16/32/64 位」。原文把范围写成"14 个模板"，实际按段名逐个数是 **25 个**（`ushorts`/`ints` 两个段名的出现处）；"14"这个数留在原文里不改。原文末尾"建议等真出现 0–255 的新字段再做"即由此决定关闭。

### Q.14

- **Step 协议两端实现已分叉，需要单独决策（2026-09-13 排查连带发现）**：`D:\Gitee\SimExchange\Source\StepProtocol\` 有一份**独立的 Step 实现**（自己的 `StepUtility.h`：`StepHeaderLen 44`、`StepTailLen 7`、`StepVersion 1`，头字段是 `Version/BodyLen/MessageType/MessageChain/MsgSeqNum`，**没有魔术字**；`HeadFromStream` 只收 2 个参数）。它与本仓 `StepUtility` 不共享任何代码，本次改造**没有**动它。如果 SimExchange 的 Step 端点是本仓 Step 协议的真实对端，那么本仓 `ProtocolVersionValue = 2` + 锚点 `SOH + "0=SPK2" + SOH` 之后，两者已经对不上（SimExchange 的包不会命中锚点，会被当噪声丢弃；本仓的包在 SimExchange 侧也解析不了）。**需要用户确认这条链路上到底谁跟谁通信**，再决定是否要同步改 SimExchange。

关闭（2026-09-13）：用户明确 `D:\Gitee\SimExchange` 已废弃，不再是 Step/Xtp 的真实对端——该链路不存在，无需同步对端实现。原文末尾"需要用户确认这条链路上到底谁跟谁通信"即由此答复关闭。
