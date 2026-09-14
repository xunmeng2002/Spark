# 项目进度

> 本文件用于跨会话状态记录，新会话开始请先阅读。

## ✅ 已完成

- **STEP 协议数字化收口：全定宽大写十六进制、`MsgSeqNum`/`CheckSum` 无符号化、两次 `HeadToStream` 塌缩成一次（2026-09-14，用户四项定策全选）**：
  起因是用户的观察——`MakePackage` 里调了**两次** `StepUtility::HeadToStream`（`Package.cpp:73` 与 `:88`）。这不是冗余而是**被迫**的：包体长度写在包头内部，而 `{:05d}` 的十进制宽度随取值变化，包头长度在编译期不可知，只能"先量后写再验"。用户由此要求协议里所有数字改十六进制定宽。**定宽之后包头长度变成编译期常量，两次调用自然塌缩成一次**——这才是本批的核心收益，不只是"换个进制好看"。四项决策：①全部 `Items` Key（含包体字段 Key，`0x100A` 原写作十进制 `4106`）统一 **4 位大写十六进制**；②**`MsgSeqNum` + `CheckSum` 改 `UInt32Type`**，`Magic` 保持 `Int32Type`；③魔术字**保持键值对** `0000=SPK2`；④仓外的 C# 对端与 DAG 副本**本批不动、登记待批**。
  - **改造后的线上格式**：包头 **62 字节**（`StepHeadLen = 11+10+10+10+14+7`）。写序**不是 ID 序**，是 `HeadToStream` 里 `AppendField` 的出现序：`0000` 锚点 11、`0008` Version 10、`0001` PackageID 10、`0002` BodyLen 10、`0004` MsgSeqNum（`{:08X}`）14、`0003` MessageChain（`{:d}`，**用户明确要求保持 1 位**）7。报尾由 11 改为 **14 字节**（`StepTailLen = 5+8+1`）。包体标记由 `SOH+'6'+'='` 变为 `SOH+"0006="`。完整样例（Version 已升 3、`MsgSeqNum` 取 `0xFFFFFFFF`）：`SOH 0000=SPK2 SOH 0008=0003 SOH 0001=00A1 SOH 0002=0008 SOH 0004=FFFFFFFF SOH 0003=1 SOH`，报尾 `SOH 0005=E3069283 SOH`。
  - **两次调用塌缩成一次**（`Package.cpp:68-94`）：删掉 `Head.BodyLen = 0` 占位、第二次写头、以及"头长是否变化"的断言及其 2 行注释。**新增一条前置闸门是必要的而非可选**：原实现没有 `size < headLen + StepTailLen` 检查，靠 `HeadToStream` 的旧 128 闸门兜；闸门收到 62 后若仍传整帧 `size`，`size = 100` 时 `bodyCapacity = 24` 为正数，会**先写出包体再靠写头失败返回**，多写一次缓冲。调用方传 `headLen` 而非整帧 `size`：调用方声明"这是包头区"、被调方声明"我至少要 `StepHeadLen`"，两边在 `size == headLen` 时正好吻合。
  - **`HeadToStream` 自校验长度**：写完若 `GetWrittenLength() != StepHeadLen` 即返回 0。这让 `StepHeadLen` 成为包头长度的**唯一真源**，格式改漏变成响亮失败而不是静默发出错帧。入口闸门从 `StepMaxHeaderLen`(128) 收紧为 `StepHeadLen`(62)，**属对外可见的契约变化**（仓外若依赖"`size < 128` 必返回 0"会受影响）；仓内无此依赖（`Package.cpp` 传 `StepHeadLen`）。`StepMaxHeaderLen` 现在**只剩读者一侧在用**（`PackageReader.cpp:229` 判"数据还没收全"），头文件里那行注释已改写明白。
  - **`MsgSeqNum`/`CheckSum` 无符号化的证据（先核实再动手）**：全仓**无生产代码写负数**。两仓所有 `Prepare` 调用点（Spark 2 处、QT 约 50 处）实参只有三种——字面量 `0`、CTp 风格 API 的 `int requestID`、回显 `package->Head.MsgSeqNum`；唯一的负值是上一批我为让 `%13d` 空格填充可见而写的 `StepUtilityTest.cpp` 用例，改定宽后该用例的目的本身消失。`CheckSum` 的证据更硬：生产方 `ProtocolUtility.h:7` 声明 `unsigned int CalculateCrc32c(...)`，而**所有消费方都先转回无符号**——8 处 `static_cast<unsigned int>(...)`/往返 cast 的唯一存在理由是类型写错（`PackageReader.cpp` 2 处、`Package.cpp` 1 处、`StepUtility.cpp` 2 处等，已全部消除）。`ProtocolVersion.h` 新增一条 `static_assert` 钉死这两个字段的无符号性，并在注释里写明理由：改回有符号会让 `FFFFFFFF` 产生补码歧义。
  - **读侧统一到一份解析策略（§5）**：`ParseInteger` 增加**带默认值**的 `int base = 10` 形参（生成物 `Packages.cpp` 的调用方全用十进制，默认值保持源码兼容）。`std::from_chars` 的两个性质正好是我们要的：对无符号目标**拒绝 `-` 号**；越界返回 `result_out_of_range`，**省掉手写上界**。据此**删除文件级 `strtoll` 版 `TryParseInteger`**（改完后已无任何调用方），`<errno.h>` 与 `<stdlib.h>` 随之移除（`<stdio.h>` 保留，`TailToStream` 仍用 `::snprintf`）。`HeadFromStream` 的 Version/PackageID/BodyLen/MsgSeqNum 全部改 `base 16`；`MessageChain` **保持 base 10**（`{:d}` 只产出 `0`/`1`，两种进制下逐字节相同），并单独用花括号包住 `case` 体以避开跨标签初始化。
  - **`GetNext` 的 `atoi` 换成有界十六进制解析，顺带修掉两条已登记的 `❓`（引用前已 grep 归档核实，见归档第 34 行）**：`atoi` 对超 `UInt16Type` 的串静默截断（`69645` → `0x100D`、`65536` → `0` 即 Magic，让非规范编码静默别名到合法 key）、对非法串静默返回 0（畸形帧会被当成 `0=...` 的合法字段）。**失败模式收紧**：调用方日志文案由 `Unexpected ItemID:0x0` 变为 `GetNext Failed`，仓内三类调用方都不区分这两种失败，无行为影响。
    - **同一处顺手关掉归档里记的第二个洞**：旧的 `TryParseInteger` 用 `*end != '\0'` 校验"停在第 1 个 NUL"而非"停到 `text.size()`"，于是 `6=100\0垃圾` 会被当成合法的 `0x100` 接受、SOH 前的垃圾被静默吞掉。新的 `ParseInteger` 判的是 `result.ptr != last`（`last = first + text.size()`），该畸形输入直接返回 false。
    - **归档里那条"切勿改用 `from_chars`/`ParseInteger<T>`"的告诫在本批已失效，且失效原因正是本批改掉的东西**：那条告诫的前提是"`HeadToStream` 用 `%13d` 空格右填充写 `MsgSeqNum`，`HeadFromStream` 正是靠 `strtoll` 跳前导空格才能读回自己的输出"——定宽零填充 `{:08X}` 之后**不再有任何前导空白**，`strtoll` 相对 `from_chars` 的三处差异面（跳前导空白、接受 `0x` 前缀、接受 `-`）全部变成无关或反向有利，故整条 `strtoll` 版解析器连同 `<errno.h>`/`<stdlib.h>` 一起删除。**这不是推翻旧结论，而是旧结论的前提已随本批消失**。
  - **`GetFieldStart`/`GetFieldEnd` 的单字符标记失效，抽出共用 helper**：原先硬编码 `buff[i] == '6'`/`'7'` 且假定 `buff[i+1] == '='`，键一变成 4 位就全失效。新增 `IsKeyAt`（判断某处是不是 `expectedKey` 后跟 `'='` 的完整键）与 `LocateFieldMarker`（扫描 `0006`/`0007`，回填其后的 fieldID、标记起始位置、以及标记之后的下一个位置）；两个公开入口退化成各取一个回填值的包装。**行为等价性已核**：原 `GetFieldStart` 不要求标记后有 SOH，但它的 `TryReadHexField` 内部**已经要求了**，所以统一版补上的 `GetNextSoh` 必然找到同一个 SOH。
  - **锚点与包头首字段共用一次格式化（§5）**：原先 `HeadToStream` 用 `{:c}{:d}={:s}`、`GetPackageStartAnchor` 用 `::snprintf` **各拼一遍同一串字节**，改格式时极易只改一处——锚点错了读者就永远找不到帧。抽出 `AppendPackageMagicField` 两处共用。
  - **14 处 Key 格式串未抽公共 helper，这是有意的取舍**：`std::format_string` 要求编译期字面量，无法由 `StepKeyTextLen` 拼出来；而给 `StepWriteCursor` 加 `AppendKeyedField` 会改动上一批刚加固、已被 60 个用例证明的有界写入契约，收益不抵风险。代价用**测试**兜住：新增 `WriteKeyWidthIsFourDigitHexForAllOverloads` 遍历全部 14 个写入入口。
  - **`ProtocolVersionValue` 2 → 3**：线格式破坏性变更，不升版本两端会按不同格式解析同一串字节。失败模式是 `HeadFromStream` 返回 false（锚点匹配不上）而非静默读错值，**这是有意的**。
  - **测试改动**：`MakeStepField` 改为 `{:04X}` 键，**这一个 helper 同时修正了 14 个写入用例的期望串与大量直接调用**；`MakeStepHeadStream` 形参改别名类型（`UInt32Type msgSeqNum` / `BoolType messageChain`）、缓冲改 `StepHeadLen`；`MakeRawStepHeadStream` 默认值全部改成合法十六进制文本（`version` 由 `std::format` 从 `ProtocolVersionValue` 推出，避免版本升级时漂移）；`MakeStepTailStream` 去掉 `static_cast<Int32Type>`。**两处畸形值用例在十六进制下判定会翻转，已修正**：`HeadFromStream_UnparseableValue` 的 `"abc"` 在十六进制下**是合法值**（0xABC），改为 `"000G"`；`HeadFromStream_TrailingGarbageValue` 的 `"2abc"` 同样合法且**被完整消费**，改为 `"2abcZ"` 以保留"尾随垃圾"的原意。`"99999"` 作为 BodyLen 仍越界（截断值由十进制 34464 改为 `0x9999 = 39321`，注释已更正）。
  - **新增 7 条用例**（`StepUtilityTest` 6 + `PackageSerializationTest` 1）：`HeadToStream_ExactCapacitySucceeds`（容量恰为 62 必须成功）、`HeadToStream_ByteExactGolden`（62 字节黄金串，字面量写 `0003` 让版本升级成为绊线，`MsgSeqNum` 取 `0xFFFFFFFF`）、`HeadStreamRoundTrip_HighBitMessageChainSeqNum`（`0x80000000` 往返，证明补码歧义消除）、`HeadFromStream_NegativeMsgSeqNum`（`from_chars` 对无符号拒绝 `-`）、`GetNext_NonHexKey` + `GetNext_KeyOutOfRange`、`WriteKeyWidthIsFourDigitHexForAllOverloads`、`StepPackage_BufferExactlyHeadPlusTailReachesGenerator`（Step 闸门 75/76 两个边界都覆盖）。原 `HeadToStream_ByteExactGolden` 里 `MsgSeqNum = -1` 与"读侧靠 `strtoll` 跳前导空白"那条契约断言随之删除——该目的本身已消失，注释里"日后换不容空白的解析器会先红"的提醒不再是载荷需求。三条往返用例的 `EXPECT_LT(headLen, StepMaxHeaderLen)` 一并改为 `EXPECT_EQ(headLen, StepHeadLen)`。
  - **三配置全绿**：Windows x64-Debug **392/392**、x64-Release **392/392**、WSL GCC Debug **391/391**（差 1 条仍是 `DirTest.cpp` 的 `#ifdef _WIN32`），零错误、零新增告警（MSVC 与 libstdc++ 13 都零告警）。基线 385/385/384，本批 **+7 条**。
  - **验证 5·自校验有效性的实证**：临时把 `HeadToStream` 里 Version 的 `{:04X}` 改成 `{:05X}`（包头变 63 字节），**17 条用例当场变红**，含 `HeadToStream_ExactCapacitySucceeds`（证明返回 0 而非发出错帧）与 `PackageSerializationTest.StepRoundTrip`（端到端）；改回后复绿。这是"格式改漏会被发现而不是发错帧"的实证。
  - **验证 4·`Head.xml` 类型改动不外溢（本批最关键的一条）**：`pumpall.py` 前后 `git status --short`，**只有 `Head.h` 变更**（`git diff` 实测 **+2 −2**，只有那两行类型），`test/Packages/Packages.cpp` **零 churn**；重跑 pump 幂等。两份生成物里 `Head.MsgSeqNum` / `Head.CheckSum` / `Tail.CheckSum` 出现 **0 次**（模板全文只有 `Head.PackageID = PackageID;` 一处引用 `Head`，是 `UInt16Type` 赋 `UInt16Type`）。故 **Templates 零改动、无需重新生成、QT 零源码改动**。
  - **跨仓**：`cmake --install out/build/x64-Release` 重刷 `Libs/Spark/x64-windows`（`Head.h`/`StepUtility.h`/`ProtocolVersion.h`/`Types.h` 与 `Network.lib`/`Network.dll` 已更新），QuantTrading `--target PackagesStatic` **构建通过**、`git status --short` **为空**（零源码改动）。**未做的**：QT 全量构建仍卡 `MdbStatic`（见既有登记），故"QT 全量编译"未验证；`Libs/Spark` 只有 `x64-windows` 一个三元组，Linux 侧仍属"未编译验证"。
  - **验证 9·文档一致性**：全仓 grep `%13d`/`%05d`/`0=SPK2`/`4106`/`StepMaxHeaderLen`，源文件与测试**无残留描述旧格式的段落**；唯一命中在 `docs/wire-protocol-revision-plan.md`（`:290` 与 `:480` 仍是 v2 的 `0=SPK2` 与十进制样例）。按 §8「禁止静默覆盖历史记录」，本批**不改写其正文**，登记待批。
  - **净收益核对**：`StepUtility.cpp` / `Package.cpp` / `PackageReader.cpp` 三个改动文件的 C 风格 cast 计数均为 **0**；`StepUtility.cpp` 里 `strtoll`/`atoi`/`atof`/`atoll`/`errno`/`ERANGE`/`TryParseInteger` **零残留**，`snprintf` 仅剩 `TailToStream` 一处。
  - **登记待批（均超出本批范围）**：①C# 对端 `SharpLibrary/Network/StepProtocol/`（8 文件）同步；②`DAG/DAGDemo/PersonalLib/include/Protocol/StepUtility.h` 声明副本同步；③**`Package::Prepare(SessionIDType, int messageChain, int msgSeqNum)` 的 `msgSeqNum` 形参仍是有符号 `int`**——改它要动公开签名，且 QT 约 50 个调用点传的是 CTp API 的 `int requestID`；负值会静默回绕成 2^32 附近的大值（今天则存成负数），两种都原样保留调用方的位模式；④`docs/wire-protocol-revision-plan.md` 补 v3 变更记录；⑤`TailToStream` 现在与 `HeadToStream` 形状相同（同样有定长宽度校验），可以同样迁移到 `StepWriteCursor`——本批按批准的计划保留了 `snprintf`。
  - **既有问题（非本批引入，未修，仅登记）**：①`Model/` 与 `model/` 大小写不一致——**此处我最初的记述有错，已按实测更正**：`../Model/` 三处（`pumplist.xml:2,3,4`、`model/parselist.xml:2`）指向的是**同级独立仓库** `D:/Gitee/Model`，该目录磁盘实测**确为大写 `Model`**，引用**完全正确**；真正不一致的只有指向 **Spark 自身**目录的 `./Model/` 两处（`pumplist.xml:5`、`pumptemp.py:8`），而 Spark 自身的目录磁盘实测与 git 跟踪**均为小写 `model/`**（`git ls-files` → `model/Head.xml`、`model/parselist.xml`）。`README.md:86` / `README.en.md:86` 写小写（与实际相符）、`docs/wire-protocol-revision-plan.md:106,112` 写大写（与实际不符）。因本机 Windows 与 WSL drvfs 都不区分大小写，眼下不影响运行，但在区分大小写的检出上 `./Model/Head.xml` 解析不到。**方向待用户定**（把目录改成大写、或把两处引用改成小写）。②`StepUtility.cpp` 的 `#include <string.h>` 与 `#include <Spark/Network/Protocol/ProtocolUtility.h>` 是**既有死包含**（前者全文无任何 string.h 函数、后者 `CalculateCrc32c`/`FindBytes` 零引用），已按用户指示于本批后续提交中移除；`Package.cpp`（`CalculateCrc32c` 2 次 + `memcpy`/`memset`）与 `PackageReader.cpp`（5 次 + `memcpy`/`memmove`/`memset`）的 include **均为实需，未动**。
  - **本轮代码审查（2026-09-14）**：阻断 **0** / 高 **0** / 中 **5** / 低 **5**，净结论"**可以合入**"。中 5 条里 **4 条采纳、1 条上报用户**；低 5 条里 **3 条采纳、1 条采纳（本人新写的行）、1 条驳回**。
    - **采纳·M-1（真实问题，本批引入）**：`Package.cpp:35` 的 `Head.MsgSeqNum = msgSeqNum;` 在字段改无符号后成了**无显式标注的 int→unsigned 转换**，`-Wsign-conversion` 会告警。改为 `static_cast<UInt32Type>(msgSeqNum)` 并注明取模是有意的——形参仍是 `int`（改它要动公开签名，见登记 ③）。**这条尤其要紧**：本工程 `CMAKE_CXX_FLAGS` 被覆写、**无 `/W` 等级**，有符号/无符号的转换问题在编译期是静默的，只能靠人看。
    - **采纳·M-2（真实问题，我原先漏了）**：`StepHeadLen`/`StepTailLen` 若硬编码字面量 `62`/`14`，`StepKeyTextLen` 一变下面的 `static_assert` 反应不明显。改为**从 `StepKeyTextLen` 推导**：`StepHeadLen = (1+key+1+4+1) + (key+1+4+1)*3 + (key+1+8+1) + (key+1+1+1)`、`StepTailLen = key+1+8+1`，断言保留 `== 62u` / `== 14u`。这样键宽改动会**先**在长度常量上显形，而不是留下"写侧发 4 位键、读侧找 5 位键"的静默错位。
    - **采纳·M-5**：`Package.cpp` 里 `CalculateCrc32c((const unsigned char*)buff, …)` 的 C 风格转换正好落在我本批改动的两行上（`Package.cpp:65` 与 `:93`），已改 `reinterpret_cast<const unsigned char*>`；同时删掉 XTP 分支里**现已多余**的 `static_cast<Int32Type>(...)`（字段改无符号后该 cast 失去意义，审查方将其登记为既有问题，我顺手清掉）。
    - **采纳·L-1**：`TailToStream` 把 `CheckSum` 直接交给 `snprintf` 的 `%08X`，**变参不参与编译期类型检查**，宽度与符号全靠"`UInt32Type` 就是 `unsigned int`"这一点。在 `ProtocolVersion.h` 增加 `static_assert(std::is_same<UInt32Type, unsigned int>::value, "TailToStream 的 %08X 依赖 UInt32Type 就是 unsigned int")` 钉住。
    - **采纳·L-4**：我在本批**新写/重写**的行里又留了 4 处 C 风格 `(int)`（2 处 `GetNext_NonHexKey`/`GetNext_KeyOutOfRange` 的 `(int)field.size()`、1 处 `HeadStreamRoundTrip_HighBitMessageChainSeqNum`、1 处 `HeadFromStream_BodyLenOutOfRange` 的 `(int)stream.size()`），与上一批"回收自己的新 cast"同性质，已全部改 `static_cast<int>`；**既有**的几十处（`StepUtilityTest` 已登记在 105 处/24 文件那批里）本批不动，避免扩大范围。实测 `git diff -U0 | grep '^+.*(int)'` 现在**为空**。
    - **采纳·L-5**：`MakeRawStepHeadStream` 的 6 个调用点里 4 处传硬编码的合法版本占位串（当时写 `"0002"`，本批升 3 后即成过期值），改用匿名命名空间新增的 `kStepVersionText = std::format("{:04X}", ProtocolVersionValue)`，并把它同时用作该形参的默认值——一个来源，版本再升不会漂移。另 2 处（`"000G"`、`"2abcZ"`）是**故意**传畸形值，保持字面量。
    - **驳回·L-2**：`GetPackageStartAnchor` 忽略 `AppendPackageMagicField` 的返回值。**不修**：该路径**不可达**——锚点只有 11 字节，`char buff[StepHeadLen]`(62) 装得下；唯一可表达出的失败是"写出空锚点"，而那正是忽略返回值时已经出现的结果；若加运行时分支，该分支出于同样原因是死代码，还会在静态初始化期间冒写日志的风险。代价已由 `PackageStartAnchor_Format` 与 `PackageStartAnchor_MatchesHeadToStream` 两条用例兜住。
    - **上报用户·M-4**：`ProtocolVersionValue` 是 **XTP 与 Step 共用**的一个常量，两者都在报文头里写它、也都在读侧校验它。XTP 的**线格式本批没有变化**（`MsgSeqNum`/`CheckSum` 与相邻字段同宽同位，`memcpy` 出来的字节逐位相同），但它会随这条常量**一起升到 3**，于是 XTP 对端也必须同步升级，否则在"协议版本不匹配"上被直接判死。两条路：**(i)** 保持共用版本号，靠文档说明"XTP 对端需随之升级"（本批现状，注释已写明）；**(ii)** 按协议类型拆成两个版本号，代价是要改两处校验分支。**本批按 (i) 落地，是否改 (ii) 待用户定**。
    - **审查后的复验**：Windows x64-Debug **392/392**、x64-Release **392/392**、WSL GCC Debug **391/391**，零错误零新增告警；`cmake --install out/build/x64-Release` 重刷 `Libs/Spark/x64-windows`（`bin/Network.dll` 更新），QuantTrading `--target PackagesStatic` **构建通过**且 `git status --short` **为空**（零源码改动）。
    - **第二轮聚焦审查（只审上述增量的差量）**：阻断 **0** / 应修 **0** / 建议 **1** / 登记 **1**，结论"可以合入"。审查独立核实了两条关键事实：读侧的 `Items::Version` 分支**只做 `base 16` 解析、不校验等于 `ProtocolVersionValue`**（只有 `Magic` 分支做等值校验），故把版本占位串由 `"0002"` 换成当前版本 `"0003"` **不改变任一用例的失败点**，四条用例仍各测其所声称者、无用例恒真；且 `Head.h` 的 `Version` 字段本就是 `UInt16Type`。另确认构建配置内**未启用 `-Wold-style-cast`/`-Werror`**，故遗留 `(int)` 不构成构建门禁，与"已登记待办"的说法一致。
      - **采纳·建议 1（并在同一处顺手收尾）**：审查发现 `HeadFromStream_NegativeMsgSeqNum` 内有 1 行仍是 `(int)stream.size()`——该行文本与旧 `HeadFromStream_BodyLenOutOfRange` 的尾部**逐字节相同**，故 git 判为"未变更上下文"，既不在我列的 4 行里，也没进"本批新增行"的取材范围。已改 `static_cast<int>`（全文件新增行现无 `(int)`）。**同一处暴露出的同类问题一并修**：`HeadFromStream_MissingKey` 手工拼的流里 `Items::Version` 传 `"2"`、`Items::MsgSeqNum` 传 `"7"`，两者都是定宽十六进制改造前的写法——**它们已经不是 `HeadToStream` 能产出的任何形态**，改 `kStepVersionText` 与 `"00000007"`。这属本批「改了线格式」的自然后果，不是扩大范围。
      - **登记待办（本批不动）**：审查提议若日后希望生产与测试**共享**版本文本，可在 `ProtocolVersion.h` 比照 `ProtocolMagicText` 增设 `constexpr std::string_view ProtocolVersionText`；当前测试本地用 `const std::string` 派生是合理替代（`std::format` 非 `constexpr`），不做也不算问题。
      - **增量后的三配置复验**：**392 / 392 / 391** 全绿。

- **协议写路径收尾：C 风格 cast 清零、`MakePackage` 前置闸门、`HeadToStream` 改走 `StepWriteCursor`、测试脚手架去重（2026-09-14，用户四项全选并逐项定策）**：
  上一批（Spark `3dd917a` / Templates `73d9b3f` / QuantTrading `274004d`）留下的四项尾巴一次收口。三项决策用户已定：`HeadToStream` **改走 `StepWriteCursor`**（不是只加 `static_assert`）；C 风格 cast 范围**只清模板与两份生成物**（仓内手写代码另有 105 处 / 24 文件，另开一批）；同一模板里另外两类 cast（385 处）**一并清**，`:284` **改 `memcpy` 读**（不用 `reinterpret_cast`）。
  - **模板与两份生成物的 C 风格 cast 归零**：`Templates/Cpp/Protocol/Packages/Packages.cpp.tpl` 四处——`:137`（`ToStepStream` 的 enum 分支）与 `:197`（`FromStepStream` 的 enum 分支）的 `(int)` / `(!!@type!!Type)` 各改 `static_cast`，`:284`（`FromXtpStream`）的 `*(UInt16Type*)(buff + offset)` 改为 `memcpy(&fieldID, buff + offset, sizeof(UInt16Type)); offset += sizeof(UInt16Type);`。**为什么 memcpy 而不是 `reinterpret_cast`**：后者同时不解决**未对齐访问**与**严格别名违规**；改完后与同一份生成物**写侧**已有的 `memcpy(buff + offset, &XField::FieldID, sizeof(UInt16Type))` 形状一致、字节结果完全相同，代价是每处 +1 行。**前两处的 `static_cast` 是必须的、不是冗余**：那两个分支的字段类型是 enum（`Types.h` 里 60 个枚举全是 `enum class … : int32_t`），而 `WriteString` 的重载集合只覆盖调色板别名与字符串指针、**没有 enum 重载**，要靠显式转成 `Int32Type`(`int`) 才能落到 `"{}={:d}"` 那个重载上。**以模板为唯一改动入口、只重生成不手改**——生成物里 `sizeof(UInt16Type)` 出现 980 / 265 处、函数签名 `(char* buff, int size)` 出现 376 / 104 处，任何对生成物的全局替换都会误伤。生成物实测：`(int)` 246→**0** / 76→**0**、`(XxxType)(atoi` 123→**0** / 38→**0**、`(UInt16Type*)` 188→**0** / 52→**0**；`static_cast<int>` 264→510 / 71→147、`static_cast<XxxType>(atoi` 0→123 / 0→38、`memcpy(&fieldID` 0→188 / 0→52。**等价性用「逆向变换还原」证明**：对 after 做回旧形态的逆变换后与 before **逐字节相同**（两仓均 True），既有 `static_cast<int>(sizeof(...))` 264 / 71 处零误伤；生成器**幂等**（连跑两次产物逐字节相同，1397567 字节）。
  - **`MakePackage` 加前置闸门**（`Package.cpp:36`，紧跟 `FillProtocolHead(Head);` 之后）：`if (buff == nullptr || size < static_cast<int>(sizeof(HeadField) + sizeof(TailField)))` → 记日志并返回 0。原实现的 `char* data = buff + sizeof(Head);` 与 `int bodyCapacity = size - 16 - 4;` 都排在**任何 `size` 校验之前**——`size < 20` 时前者是越过 one-past-the-end 的指针算术（`[expr.add]`，UB）、后者为负；今天不出事**不是 `MakePackage` 自己保证的**，而是「生成的 `ToXtpStream` 恰好把 `size` 检查放在 `memcpy` 之前」，而 `ToXtpStream` 是 `Package` 的**公开纯虚函数**（`Package.h:27`），契约上允许任何实现先写后判，所以负的 `bodyCapacity` 不能被当成下游会自己挡住的输入。闸门过了就保证 `size >= 20`，顺带消掉 `size = INT_MIN` 时 `size - 16 - 4` 的**有符号溢出 UB**。`buff == nullptr` 属**新增的正性检查**（§6 写前防护；原实现会崩）。**行为保值**：`size < 20` 时两条分支原本也返回 0（XTP 靠 `ToXtpStream` 返回 -1、Step 靠 `HeadToStream` 入口检查返回 0），唯一变化是日志文案；仓内唯一调用点传常量 `BuffSize`（`Protocol.cpp:108`，且有 `static_assert(BuffSize >= MaxFrameSize, …)`），不可达。
  - **`HeadToStream` 改用 `StepWriteCursor`**（`StepUtility.cpp:245`）：6 处 `::snprintf(buff + len, size - len, …)`、手写的 `buff[len] = SOH` 与其注释、以及 11 处格式实参 cast（`static_cast<int>(SOH)` 8、`static_cast<unsigned int>(Items::*)` 3）全部消失，改为 6 次 `AppendField`（**`AppendField` 自己补 SOH**，正好对上原实现的「每字段尾随 SOH」形态；容量由游标管）。**逐字段翻译**：`%c`→`{:c}`（`SOH` 已是 `char`）、`%u`→`{:d}`、`%04X`→`{:04X}`、`%05u`→`{:05d}`（两者都零填充）、`%13d`→`{:13d}`（**两者都是空格填充**）、`%s`→`{:s}`、末字段 `%d`→`{:d}`（`MessageChain` 是 `BoolType`）。**截断统一返回 0**，属**对外可见的契约收紧**（原实现返回一个越过 `size` 的假长度，靠 `Package.cpp` 的调用方顺序挡住；仓内无「返回值 > size 表示截断」的依赖）。**入口 `size < StepMaxHeaderLen` 闸门保留**：`Package.cpp:63` 与既有 `HeadToStream_InsufficientBuffer` 都依赖「`size < 128` 必返回 0」这条公开契约，去掉会让 `size` 落在 51..127 时由「失败」变「成功」。`TailToStream` **未动**（它有自己的 `len != StepTailLen - 1` 宽度校验，形状不同），故 `#include <stdio.h>` 保留；将来给包头加字段（如待议的 `Reserved`）只需多一行 `AppendField`。
  - **测试脚手架去重（§5）**：`StepUtilityTest.cpp` 在 `MakeStepField` 之后加两个骨架 helper `ExpectStepField` / `ExpectHexStepField`（搭 64 字节缓冲 → 建游标 → 写一个字段 → 按 `GetWrittenLength()` 逐字节比对），14 个用例各从 5 行缩到 1 行。**两个 helper 不合并**成一个带写回调的版本——合并后调用点会变成 `ExpectSingleField([&](StepWriteCursor& c){ … }, …)`，比被替换掉的样板更难读，属 §5「拒绝过度封装」。**不纳入** `WriteString_ExactCapacityFits` / `_ReservesByteForSoh` / `_NonPositiveCapacityTruncates` / `WriteCursor_StopsAfterTruncation` 四条（它们要测的**正是**骨架本身：`std::vector` 精确控容量、读截断标志、比对缓冲快照）。`WriteString_UInt64` 原用 `std::string(buff)`（依赖手写的那行 `\0`）改为按长度比对，属**加强**而非等价替换；`WriteString_BoolStaysNumeric` 原本**没写**那行 `\0`，走 helper 后自动一致——那个 `\0` 只是为 `std::string(buff)` 服务的脚手架，随 helper 消失是净收益。
  - **新增用例 4 条**（`HeadToStream_ByteExactGolden` 1 条 + `PackageSerializationTest` 3 条）：①`HeadToStream_ByteExactGolden` 用字面量拼出期望包头串与输出**逐字节**比对，专门钉住 `{:04X}` 大写、`{:05d}` 零填充、`{:13d}` **空格**填充三个翻译点（`MsgSeqNum` 取 `-1`，使空格填充可见，这是「两边都填充到 13」的实证而非推断），并断言 `HeadFromStream` 能把被空格填充的 `-1` 读回（读侧靠 `strtoll` 跳前导空白，**这是载荷需求而非松懈**，换成不容空白的解析器会直接打破 `HeadStreamRoundTrip`）。**实测黄金串 43 字节**（`PackageID = 0x00A1`、`BodyLen = 8`、`MsgSeqNum = -1`）；`ProtocolVersionValue` 实为 **2**，**计划里猜的 `8=8` 是错的**。②③④`PackageSerializationTest` 末尾加假 `Package` 子类 `WriteBeforeMeasurePackage`（**故意先写后判**：置标志、记容量、改缓冲首字节再返回 0）与三条用例——`MakePackage_BufferSmallerThanFixedOverhead`：`size` 取 `FixedFrameOverhead-1`(=`20-1`) / `0` / `-1` / `INT_MIN` 时两条分支均返回 0、生成器**未被调用**、整块 `char buff[MaxPackageSize]` 逐字节仍是 `'S'`；`MakePackage_BufferExactlyFixedOverheadReachesGenerator`：`size` 恰为 20 时 XTP 分支放行、返回 20、生成器被调用且观测到 `bodyCapacity == 0`；`MakePackage_NullBufferRejected`：`buff == nullptr` 传 XTP 与 Step 均返回 0 且生成器未被调用（**旧实现会在这里崩**，是本批安全收益的另一半）。**我此前把这一组记成「四条用例」是错的，实测三条**（`git diff --stat` 该文件 +86 行 / 0 行删除）。
  - **反向探针（用例不是空过）**：把 `Package.cpp` 的闸门临时改成 `if (false && (…))`，新用例确实变红（`MakePackage` 返回 20、`wasMeasured()` 为真、缓冲被改写），恢复后复绿。另把 `HeadStreamRoundTrip:533` 的 `EXPECT_LT(headLen, StepMaxHeaderLen)` 扩到 `_MinValues` / `_MaxValues`，`HeadFromStream` 的 `endIndex` 由 `(int)stream.size()` 改为实测 `headLen`、`EXPECT_EQ(headEndIndex, headLen)`。
  - **跨平台实证**：黄金串在 MSVC 与 libstdc++ 13 上都过（`BoolType` 的裸 `{}` 在 MSVC 出 `true`/`false`、libstdc++ 13 出 `1`/`0`，故必须 `{:d}`）。三配置全绿：Windows x64-Debug **385/385**、x64-Release **385/385**、WSL GCC Debug **384/384**（差 1 条仍是 `DirTest.cpp` 的 `#ifdef _WIN32`）。基线 381/380，本批 **+4 条**（`StepUtilityTest` 60 个 TEST）。
  - **我自己的 3 处新 C 风格 cast 已回收**：为「与文件既有写法一致」，我在新用例里写过 `(int)expected.size()` 与两处 `(int)StepMaxHeaderLen`，等于往 §6 违规堆里加料，已全部改 `static_cast<int>`；核对净数 `StepUtilityTest` 30→**27**、`PackageSerializationTest` 9→**9**，无新增（后随代码审查又消掉该文件一处既有 `(int)(sizeof(HeadField) + sizeof(TailField))`，见下条）。
  - **本轮代码审查（2026-09-14）**：阻断 **0** / 应修 **2** / 建议 **9** / 登记待办 **7**，净结论"无阻断项"。审查用 `pump.py` 在临时目录独立重跑三个模型的生成（Spark 1397569B / QT 386147B）与工作树**逐字节一致**，并把两份生成物的 690/502、197/145 行 diff **逐行分类**（新增行必须含 `static_cast<` / `memcpy(&fieldID`，删除行必须含 `(int)` / `Type)(atoi` / `*(UInt16Type*)`），**0 行例外**——这是"生成物无手改、模板可复现"的独立证据。
    - **采纳·应修 2 条**：①`PROGRESS.md` 按 §8 收口并按 §8.1 滚动（见上一条）；②变更清单/提交说明的数字改用实测值——**本条自身即是更正**：`PackageSerializationTest` 是 3 条用例不是 4 条、全量是 385 不是 384、既有 `static_cast<int>(sizeof(...))` 是 **264 / 71** 而不是我转述的"136+128"（审查独立复现了 264/71，并指出该数字来源不明，属我的笔误，以实测为准）。
    - **采纳·建议 5 条**：①`sizeof(HeadField) + sizeof(TailField)` 在本批新增代码里出现 4 次，触发 §5 的三段阈值，故在 `ProtocolVersion.h` 新增 `constexpr int FixedFrameOverhead = sizeof(HeadField) + sizeof(TailField);`，闸门、日志与测试三处共用，并把该头原有的 `static_assert(sizeof(HeadField) + sizeof(TailField) == 20, …)` 改写为 `FixedFrameOverhead == 20`。**该头是手写文件、不是生成物**（`Templates/Cpp/Spark/Network/Protocol/` 下只有 `Head.h.tpl` 与 `Items.h.tpl`），故加常量不牵动 Templates；且属**新增**公开常量而非删改，不触发 §3.1 的暂停。②空指针与"缓冲太小"拆成两条日志——合成一条时传 `nullptr` + 大 `size` 会打出 `Package Buffer Too Small. Size:131072, Needed:20`，现场无法反推是空指针还是尺寸问题。③`wasMeasured()` / `observedCapacity()` → `WasMeasured()` / `ObservedCapacity()`（小驼峰两种规约都不满足，本仓惯例是同级的 PascalCase `GetWrittenLength()`）。④`StepUtilityTest.cpp` 那句"本批唯一一处线上字节与 sprintf 时代不同"里的"本批"已指代不明，改为点名 `WriteString(std::string)`。⑤`_MinValues` / `_MaxValues` 新增断言处的注释归因不准（"涨过 `StepMaxHeaderLen` 等于全链路失败"描述的是生产行为，而 `HeadToStream` 放不下时返回 0，先红的其实是 `EXPECT_GT`），注释改写为实测上界（51 字节）与真实归因；**断言本身保留**——它与既有的 `HeadStreamRoundTrip:524` 同形，是"包头远小于闸门阈值"的不变量记录，删掉会让三条往返用例不一致。顺带消掉 `PackageSerializationTest` 里那处**既有**的 `(int)(sizeof(HeadField) + sizeof(TailField))`。
    - **未采纳 2 条**：①`ExpectHexStepField` 只有 1 个调用点、按 §5 字面属过度封装——**保留**：两个 helper 是成对骨架，用户在批准计划时已明确采纳"14 处、各缩成一行"这个形状，且它们上方共用一段说明"缓冲容量取 64"的注释，拆开就成两份。②`HeadToStream` 的失败契约应写进公开头文件，以及 `head == nullptr` 是否也加闸门——**登记待批**：前者要往公开头加注释（§4 例外需用户点名），后者是**行为变更**（原来解引用空指针崩、改成返回 0），且 `MakePackage` 侧不会传空。
    - **审查的两处数字与实测不符，以实测为准**（均已定点复核）：①审查称 `Templates` 有 7 个文件共 10 处 C 风格 cast（列出 `MdbStructs.cpp.tpl×3`、`SpiMiddle.cpp.tpl×2`、`ApiMiddle.cpp.tpl×1` 等）——**定点复核这四个文件形态命中均为 0**，全树宽口径重扫仍是 **4 文件 5 处**；②审查称 `GetDebugString` 的 `sprintf` 在模板 `:308`，实测是 **`:311`**（`:308` 附近无 `sprintf`）。审查方也自承"136+128"复现不出，故这两处属同类转述偏差。
    - **审查另记两条已确认的正面结论**：①`HeadToStream` 的失败模式变化**仓外不可见**——截断需要 `size >= 128` 却放不下某字段，而整头最多 51 字节，故新旧两版在所有可达输入上产出同一串字节；旧版"截断时仍返回正长度"是静默坏帧，新版返回 0 严格更好（SAMS 的生成物是更早世代、根本不调 `HeadToStream`）。②黄金串是 `{:d}` 对 `bool` 的**唯一**字节级守卫——`HeadToStream` 的 `MessageChain` 直接走 `AppendField("{:d}={:d}")`、**不经过** `WriteString` 的 `BoolType` 重载，所以几条 `WriteString_Bool` 救不了它；该翻译点一旦退化，整串比对与 `HeadStreamRoundTrip_MaxValues` 的 `parsed.MessageChain == 1` 会同时变红。
    - **审查后的复验（含采纳项重新编译）**：Windows x64 Debug **385/385**、x64-Release **385/385**、WSL GCC Debug **384/384**，零错误零新增告警；`cmake --install out/build/x64-Release` 已重刷 `Libs/Spark/x64-windows`（`bin/Network.dll` 更新、`Network.lib` up-to-date，新增的 `FixedFrameOverhead` 已随 `ProtocolVersion.h` 进安装副本——审查待办 4 一并收口）；QuantTrading 在全量构建仍卡 `MdbStatic` 的前提下，`touch src/Packages/Packages.cpp` 后单独构建 `--target PackagesStatic` **通过**。**`Libs/Spark` 只有 `x64-windows` 一个三元组**，审查提到的 `x64-linux` 在本机不存在，Linux 侧仍是「未编译验证」。
  - **QuantTrading 未编译验证（既有破坏、非本批引入）**：全量构建在 `MdbStatic` 失败——`Libs/DBAdapters/x64-windows/…/Schema.h` 仍是 **2026-08-20** 的旧安装副本，枚举只有 `{Int, Int64, Double, Char, Bool}`，缺 DBAdapters 源码 commit `422e92a`「FieldType 扩展：Int 改名 Int32，新增窄整数与无符号整数」带来的 11 个成员，导致 `src/Mdb/MdbStructs.cpp` 里 102 个 `FieldType::Int32` 未声明。**上一批已登记同一处阻塞**。退一步的实证：`touch src/Packages/Packages.cpp` 后单独构建 `--target PackagesStatic` **通过**，证明本批的模板改写对消费方干净；QuantTrading 的生成物已随新模板更新（`(int)` / `(XxxType)(atoi` / `(UInt16Type*)` 三形态同样归零）。
  - **未收口 / 登记待批（均超出本批范围）**：①`GetDebugString` 的无界 `sprintf`（模板 `:311`，生成物 264 / 71 处）——写入目标是 `thread_local char t_DataStringBuffer[10240]`，`offset` 跨全包字段累加、**没有任何上界**，与上一批刚修掉的 `ToStepStream` 越界写同等级；**本批不修**是因为它的输出形态是 `Name:field:[value], …` 而非 STEP 的 `key=value\x01`，`StepWriteCursor` 装不进去，需要一个独立的「返回 `const char*` 的有界格式化」方案。②`FromXtpStream` 的读边界（模板 `:293` 的 `memcpy` 在 `while (offset < endIndex)` 只保证 1 字节时可过读，目前靠读完后 `return offset == endIndex;` 兜）——加边界判断会改变行为（原来过读、现在返回 false），属另一批。③`Templates` 残留 C 风格 cast **4 文件 5 处**（`Mdb/InitMdbFromCsv.cpp.tpl:154 (bool)`、`Mdb/MdbTableRegistry.cpp.tpl:53 (int)`、`Mdb/MdbTables.cpp.tpl:203 (int)` 与 `:230 (void*)`、`Mdb/ModuleTableList.h.tpl:21 (int)`），消费方含 `D:/Gitee/Mdb` 与 QuantTrading，爆炸半径远超本批。**计划里关于 `ApiTest/ApiMiddle.cpp.tpl:155`、`ApiTest/SpiMiddle.cpp.tpl:132,152`、`Mdb/MdbStructs.cpp.tpl:168,176,183`、`KernelGen.cpp.tpl:48,58` 的记载已过时**——这四个文件现在都是 0 处（`MdbStructs.cpp.tpl` 在 `307f334` 被重写为查表、留 1 处 `static_cast`）。④仓内手写代码 **105 处 / 24 文件**（`StepUtilityTest` 28、`PackageReaderTest` 17、`PackageSerializationTest` 11、`MD5Test` 8、`UtilityTest` 7、`TimeUtility` 5、`SingleShm` 4，其余各 1~2），另开一批。⑤`Libs/DBAdapters/x64-windows` 是 2026-08-20 旧副本、缺 `422e92a` 的 FieldType 扩展，导致 QuantTrading 全量构建在 `MdbStatic` 失败（见上）。
  - **§8.1 滚动（随本批落地，分两步、脚本驱动、可重入）**：本批条目入区后 ✅ 区达 7 批、超 5 批上限，先按规则把最旧的**两条**移入 `PROGRESS-archive.md`——`同步更新中英文 README` → `D.13`、`按 test/ 真实用法重写 README 三个代码示例` → `D.12`（ID 按「越旧号越低」补号，归档内按 ID 倒序，故 D.13 在 D.12 之上）。此时主文件 67929 → **66422 字节**，**仍超 50 KB 目标 30%**，实测差距几乎全部来自一条：`UInt64Type` 那条独占 **38635 字节（53%）**。§8.1 的 ✅ 区保留区间是「最近 **3**–5 批」，故在合法区间内继续下探到 3 批——再移 `UInt64Type …` → `D.15` 与 `README 去掉单元测试用例数` → `D.14`（**用户 2026-09-14 在两选项里选定「滚动到 3 批」**）。最终主文件 73241 → **34183 字节**（**达标**）、归档 39466 → 78726 字节；**复核**：搬移前主文件的 197 个非空行全部仍逐字存在于两个新文件中（未命中 0 行）。**留在主文件的是本批的直接前身 `STEP 写路径收口`（11109 字节）**，被移走的 `UInt64Type` 那条里的结论（如 13 个 `WriteString` 重载不得合并、`%13d` 前导空白是载荷需求）日后须 grep 归档才能引用。
  - **§4 注释例外（已在代码后向用户说明的两处）**：`Package.cpp` 那 2 行（解释闸门为何必须排在指针与容量运算之前——`ToXtpStream` 是公开纯虚函数、仓外实现不保证先比容量再写，属外部契约的坑）；`StepUtilityTest.cpp` 两个 helper 上方 2 行（说明缓冲容量取 64 的理由，以及「容量边界与截断语义另有专门用例、不走这里」——防后来者误用 helper 去测边界）。

- **PROGRESS.md 归档拆分（2026-09-14，用户批准"现在做，脚本驱动"）**：主文件此前 **87262 字节**、超 §8.1 的 50 KB 目标 70%，归档层自机制建立以来一直没做过。按 §8.1 用脚本从 `git HEAD` 取原文、对条目边界整体搬移（只移动不删改、可重入）：已完成区保留最近 5 批，其余 **11 条**按 `D.01`–`D.11` 搬进 `PROGRESS-archive.md`；`❓` 区只留 2 条真正未决的，3 条已了结的搬为 `Q.17`–`Q.19`，其中两条仍属活约束（宿主收尾时序、P5 握手不做）在主文件新增的 `## 备注` 里各留一行短版。主文件 87262 → **55911 字节**（仍略高于 50 KB 目标，差额是最近两批自身的体量，等它们按滚动规则轮换出去即可达标）。**验证方式**：逐行比对旧文件每一非空行是否在两个新文件中逐字存在，274 行中仅索引行 `Q.14` 需还原原文后缀，其余全部命中。

- **STEP 写路径收口：有界游标 + `std::format` + 写侧类型拼写（2026-09-14，用户逐字批准；含一处推翻计划的风险断言）**：
  上一批把字段 ID 收窄成 `UInt16Type` 并补齐 19 处格式实参 cast，但**写路径本身仍无上界**——
  `WriteString` / `WriteHexString` 往 `char*& ppos` 里 `sprintf`，调用方 `ToStepStream(char* buff, int size)`
  拿到 `size` 却**从不引用**，`Package.cpp` 的长度校验是**事后**的（判失败时越界写早已发生）；
  第二处无界写是 `ToXtpStream` 的裸 `memcpy`。读路径早已逐个收 `endIndex`，本批把写侧对称收口。
  - **新增 `StepWriteCursor`（放 `StepUtility.h`，形状由用户从三处选型里选定）**：`AppendField(std::format_string<Args...>, Args&&...)`
    是**唯一**算容量的地方——`writableLength = GetRemainingLength() - 1`（为 SOH 预留 1 字节），
    `std::format_to_n` 之后以 `result.size > writableLength` 判截断，成功才推进游标并补 SOH；
    一旦截断即**闩住**，此后所有字段写入直接返回 `false`（计划外的扩充：不闩会出现"前几个字段已落、
    后面继续写"的中间态）。15 个调用点各剩一行 `cursor.AppendField("{}={:d}", key, value);`，
    §5 那 14 处逐字节相同的尾两行随之消失。
  - **状态存 `begin + capacity + written`（三个 `int`）而非 `char* end`**：`Package.cpp:42` 算 XTP 的
    `bodyCapacity` 时**未校验正性**，`size` 很小时为负，`buffer + 负数` 属越界指针运算；存 `int capacity_`
    后比较全在 `int` 域内，负容量自然落到"写不下"。
  - **`sprintf` → `std::format_to_n`，13 个格式串一律显式写表示类型**：`{}={:d}`×9、`{}={:c}`、`{}={:.6f}`、
    `{}={:s}`、`{}={:04X}`。**不能写裸 `{}`**：`BoolType` 的 `{}` 在 MSVC 出 `true`/`false`、在 libstdc++ 13
    出 `1`/`0`（线上字节会变），`{:d}` 两平台一致。`StepUtility.h` 原有 4 条钉 `Int32Type` / `Int64Type` /
    `UInt64Type` / `DoubleType` 的 `static_assert` **随之删除**——世上已无 printf 格式串，类型适配改由
    `std::format_string<Args...>` 的 `consteval` 校验在编译期完成。`<cstdio>` 移出该头；`StepUtility.cpp`
    补 `<stdio.h>`，因为 `::snprintf` 仍在用，而它此前**仅靠 `<format>` 的传递包含**才编得过。
  - **我错了一处·"截断时一个字节都不写"是错的（写测试时才发现）**：`std::format_to_n` **会把能放下的前缀
    留在缓冲里**。两个新用例当场暴露：`WriteString_ReservesByteForSoh` 拿到 `"32769=X"`、
    `WriteCursor_StopsAfterTruncation` 拿到 `"4109=0123456789\0"`。**已按事实改写类注释与这两个用例**，
    契约改为：不推进游标、绝不越过 `capacity`、`[begin, begin + GetWrittenLength())` 始终是完整字段序列，
    **但截断时调用方必须丢弃整个包体**。该更正同时推翻计划"风险"一节里的同一句断言。
  - **写侧类型拼写收口**：13 个 `WriteString` 重载 + `WriteHexString` + 兜底模板的 `int key` → `UInt16Type key`；
    四个 `Get*` 一并改别名拼写。属公开 API 变更，用户已批准（并顺带更正了本文件里"四个 `Get*` 的 `key` 是 `int`"
    这半句错误，见上）。
  - **兜底模板改编译期拒绝**：`static_assert(std::is_same<T, const char*>::value || std::is_same<T, char*>::value, "WriteString 只覆盖 Types.h 调色板里的类型与字符串指针；裸 long / size_t 请先转成对应别名")`。
    动机正是用户点的坑——`sprintf` 时代传整型是按 `%s` 解引用（崩），换 `std::format` 后同一处会**静默按数字写出**、
    线上格式悄悄变化，比崩溃难发现得多。**两个负向探针都按设计触发**：scratch TU 里
    `WriteString(cursor, 0x0001, 42L)` 报 `error C2338` 并打出该文本（而非静默输出 `42`）；
    `AppendField` 传错类型报 `error C7595`（证明 `consteval` 格式串校验确实生效）。
  - **模板与调用处**：`Templates/Cpp/Protocol/Packages/Packages.cpp.tpl` 的 `ToStepStream` 改用游标、截断返回 `-1`
    （直接落进 `Package.cpp` 已有的 `bodyLen < 0` 判定，**契约不变**，事后校验退回"真正的兜底"）；
    `ToXtpStream` 每次 `memcpy` 前加 `offset + fieldSize > size` 判断、越界 `return -1`。两个活消费方重新生成：
    `Spark/test/Packages/Packages.cpp`（+4203/−2131）与 `QuantTrading/src/Packages/Packages.cpp`（+1217/−654）。
  - **顺序是硬约束，计划里就写明了**：`QuantTrading/CMakeLists.txt:51` 是
    `find_package(Spark REQUIRED PATHS "../Libs/Spark/x64-windows")`，吃的是**安装副本**而非源码树，
    故必须"改 Spark 源码 → 重生成 Spark 生成物 → `cmake --install out/build/x64-Release` 刷新
    `Libs/Spark/x64-windows` → **才**重生成 QuantTrading"。刷新 `Libs` 不产生提交（`Libs/Spark/` 整体
    未被该仓跟踪）。
  - **实证**：Windows x64 Debug **381/381**、x64-Release **381/381**、WSL GCC Debug **380/380**
    （差 1 条仍是 `DirTest.cpp` 的 `#ifdef _WIN32`），全部零编译告警；逐条核对 `.obj` 时间戳新于源码
    以证明真的重编（本工程无 `/W` 等级，格式失配编译期静默）。`StepUtilityTest` 51 → **59**，
    `PackageSerializationTest` 16 → **19**。生成物**幂等**：用提交后的模板重跑 pump，五个输出**逐字节相同**；
    `PackageFactory.{h,cpp}` 零 churn（本批不动模型）。`QuantTrading` 的 `Packages.cpp.obj` **编译通过**
    （`[25/153]`，目标文件新于源文件），**但整仓构建被一个与本事无关的既有破坏挡住**：
    `src/Mdb/MdbStructs.cpp` 用了 `FieldType::Int32`，而 `Libs/DBAdapters` 的 `Schema.h` 里 `FieldType` 是
    `{Int, Int64, Double, Char, Bool}`（**没有 `Int32`**，且该枚举属 DBAdapters 不属 Spark）。
    故 QuantTrading 记为"**签名变更对消费方干净，但整仓未编译验证**"。
  - **本轮代码审查（2026-09-14）**：严重 **0** / 高 **0** / 中 **2** / 低 **3**，净结论"无阻断项，两条中属护栏与注释层面的收口遗漏"。
    - **采纳·我错了一处（中）：注释声称的宽度兜底不存在**。删那 4 条 `is_same` 时我给的理由是"世上已无 printf 格式串"，
      并把注释里的兜底改指到"Types.h 那几条钉宽度的 `static_assert`"——**这半句是假的**：`Types.h:1075-1079` 断言的是
      `sizeof(long long)` / `sizeof(unsigned long long)` / `sizeof(double)` / `sizeof(bool)`，全是**字面类型、不引用别名**，
      对别名漂移恒真；而 `is_same<Int64Type, long long>` 这类绑定断言**没有等价替代**。**已补回 4 条**（`Int32Type`/`Int64Type`/
      `UInt64Type`/`DoubleType`），理由换成真实的那个：别名漂移成同宽异型时 `std::format` 不响、`sizeof` 断言也不响，
      而 XTP 路径按 `sizeof(别名)` 走 `memcpy`，宽度一变线上格式就错位。**探针**：把 `Types.h:97` 的 `typedef long long Int64Type;`
      临时改成 `long` → `StepUtility.h:29` 报 `error C2338: static assertion failed: 'Int64Type 必须是 long long，XTP 按 sizeof 取宽'`，
      **正是删掉断言后会被放行的那条路径**；随后撤回，`Types.h` 相对 HEAD 零差异。
    - **采纳·我漏了一处（中）：`GetRemainingLength() - 1` 在 `capacity == INT_MIN` 时有符号下溢**。`remaining` 为 `INT_MIN` 时
      该减法 UB（补码回绕成 `INT_MAX`），`writableLength < 0` 判不出来，`format_to_n` 拿到约 2^31 的上界、截断条件永远不成立。
      仓内不可达（Step 分支保证 `bodyCapacity >= 1`），但 `ToStepStream` 是 `Package` 的**公开虚函数**、`StepWriteCursor` 是
      `NETWORK_EXPORTS` 导出类，仓外调用方可传任意 `size`。**改法比审查建议的构造函数闩锁更小**：先 `if (remainingLength <= 0)`
      再减 1，减法两侧都非负、且不留死代码。用例已扩到 `{ 0, -1, std::numeric_limits<int>::min() }`；**反向探针**把护栏改回旧写法后
      第三轮迭代拿到 `"32769=1\x1"`（`IsTruncated()` 为假、缓冲被写花）而失败，确认用例真咬得住。
    - **采纳·我错了一处（低）：测试注释给的理由仓内不成立**。原文写"`Package::MakePackage` 的 Xtp 分支不校验 `bodyCapacity` 为正"，
      但 Xtp 分支走 `ToXtpStream`、**从不构造游标**；唯一构造游标的 Step 分支（`Package.cpp:69`）前面有
      `headLen <= 0 || headLen + StepTailLen >= size` 的判断，保证 `bodyCapacity >= 1`。**护栏本身站得住，理由换成"仓外调用方可传任意 size"**；
      同一条错误理由也支撑过"游标不存 `char* end`"那段设计说明，一并改成同样措辞。
    - **未采纳·留待后续批次（低）**：`Packages.cpp.tpl:137` 那行 `(int)!!$fieldName!!->!!@name!!` 的 C 风格 cast——审查建议"顺手清掉"，
      但计划里已定为另开一批（改它要重跑两个消费方的生成、再刷新 `Libs` 安装副本并重验），仍延后。**实测规模远比计划里写的"1740 行"小**：
      两份生成物共 **322 处**（Spark `test/Packages/Packages.cpp` 246、QuantTrading 76）。
    - **审查另记三条信息**：①13 个 `WriteString` 重载**不应合并**（9 个 `"{}={:d}"` 看着像纯重复，但其存在理由就是给每个别名一个
      精确的非模板匹配；合并后裸 `long` 会命中新模板、兜底 `static_assert` 失效）——§5"拒绝过度封装"的正当例外；②截断闩锁被判定
      "冗余但恰当的防御，建议保留"；③`HeadToStream` 的 `size - len` 连乘模式当前可证安全（入口保证 `size >= StepMaxHeaderLen(128)`、
      包头最大约 60 字节，故 `snprintf` 永不截断、`size - len >= 68 > 0`），但这条不变量没有任何静态断言钉住。
    - **审查后的复验**：Windows x64 Debug **381/381**、x64-Release **381/381**、WSL GCC Debug **380/380**；
      强制重编（`touch` 后重建）Windows 与 GCC 两侧告警计数均为 **0**。
  - **明确排除**：`SAMS` 与 `LibTest` 是已废弃项目，不在改动范围；`Templates/Cpp/Protocol/Step/` 与
    `.../Xtp/` 两个**死模板目录由用户自行删除**（已核实 0 个 pumplist 引用、0 份生成物），本批不碰它们，
    **提交时必须只 stage `Cpp/Protocol/Packages/Packages.cpp.tpl` 这一个文件**。
  - **未收口（另开批次）**：`Packages.cpp.tpl:137` 的 `(int)` C 风格 cast（§6 禁止；实测两份生成物共 322 处，
    不是计划里估的 1740 行）；`GetDebugString` 在 `:264` 生成的调用点仍用无界 `sprintf`；
    计划里提过的 `ToXtpStream` → `PutBytes` 整合延后；测试文件剩余 9 处样板重复；
    `size_t` 在 Windows 上就是 `UInt64Type`，故兜底模板那条断言实际上平台相关。


## 🔄 进行中

- 无

## ❓ 待讨论 / 待决策

- **`ProtocolVersionValue` 要不要按协议类型拆成两个（2026-09-14 由代码审查提出，用户未决）**：
  该常量由 **XTP 与 Step 共用**，两者都在报文头里写它、也都在读侧校验它。本批 Step 的线格式
  破坏性变更把它从 2 升到 3，而 **XTP 的线格式本批没有变化**（`MsgSeqNum`/`CheckSum` 与相邻字段
  同宽同位，`memcpy` 出来的字节逐位相同），却会跟着升到 3，于是 XTP 对端也必须同步升级，
  否则在"协议版本不匹配"上被直接判死。本批按**共用版本号 + 注释说明**落地，未拆分。

  | 方案 | 代价 |
  | :--- | :--- |
  | (i) 保持共用（本批现状） | XTP 对端需"无理由"升级一次；无代码改动 |
  | (ii) 按协议类型拆两个版本号 | 要改两处校验分支；两类协议日后可独立演进 |

- **Step 头上的 `Reserved` 字段暂不上线（本轮决策，待复核）**：`Head.xml` 里 `HeadField` 有 7 个字段（含 `Reserved`），但 Step 的文本包头只序列化 6 个（`HeadItemCount = 6`），`Reserved` 仅 Xtp 分支有。理由：`Reserved` 的定义是"保留字段，必须为 false"，缺省即 false，上线只会让包头多 12 字节；如果希望两端能校验"对端没乱用保留位"，需要把它加进 `HeadToStream`/`HeadFromStream` 并把 `HeadItemCount` 改成 7。

- **文本协议反序列化对 `uint16`/`int32` 仍走 `else: atoi`（2026-09-13 记，待决定）**：
  `Templates/Cpp/Protocol/Packages/Packages.cpp.tpl` 的整数分支是
  `elif $type in ('uint8','int8','int16','uint32','uint64')` + `else: atoi(value.c_str())`，即 `uint16s`
  与 `int32s` 这两个段**掉进 `else`**，拿不到上一批新加的 `StepUtility::ParseInteger` 范围检查。补两个标签
  进去改的是文本协议反序列化路径，且是**行为变更**（从"小值正确、超范围静默截断 / `atoi` 溢出 UB"变成
  "拒绝并报错"），需用户点头。并存项：`atoi` 的溢出 UB 对**本就走在 `else` 里**的既有族是既有问题，
  非本批引入。同一处的 `formats` 对 `uint16s` 三花脸（`%hu`/`%d`/`%u`）属纯风格，可一并收口。

## 备注

- 宿主必须显式调用 `Logger::Stop()` + `Join()` 收尾，否则最后一次缓冲必丢；这是进程退出时序的**定论**，不是可以靠改析构语义绕过的缺陷——见归档 `Q.17`
- P5 握手（协议版本协商）已决定**不做**，日后若要做的入口是 `Protocol::OnConnect`——见归档 `Q.18`

## 归档索引

- `D.15` `UInt64Type` 由 `uint64_t` 改为 `unsigned long long`（含随后的 WriteString 重载、`Items.h`、16 位护栏一串）
- `D.14` README 去掉单元测试用例数
- `D.13` 同步更新中英文 README（`README.md` / `README.en.md`）
- `D.12` 按 `test/` 真实用法重写 README 三个代码示例（中英文同步）
- `D.11` 更新 `docs/environment-setup.md` / `environment-setup.en.md`
- `D.10` Logger 退出路径补最后一次落盘 + 停止后写入判空
- `D.09` Logger 析构兜底不成立
- `D.08` 环境事件
- `D.07` Logger：超长单行越界写 + 空 `FILE*` 落盘 + 建日志目录抛异常
- `D.06` 线协议改造
- `D.05` 协议改造连带清理
- `D.04` 协议尺寸常量收口
- `D.03` 类型宽度显式化
- `D.02` 类型调色板补齐 8/16/32/64 位
- `D.01` 类型 label 逐族统一
- `Q.19` 生成器不认 `size`，包体越界写没有写前防护（2026-09-14 关闭，本次拆分时移入）
- `Q.18` P5 握手（2026-09-14 关闭，本次拆分时移入）
- `Q.17` 设计约束：宿主必须显式调用 `Stop()`+`Join()`，否则最后一次缓冲必丢（2026-09-14 关闭，本次拆分时移入）
- `Q.16` README 上的单元测试用例数是否继续标注（2026-09-14 关闭）
- `Q.15` 8 位整型是否补进类型模板（2026-09-13 关闭）
- `Q.14` Step 协议两端实现已分叉（2026-09-13 关闭）——见 `PROGRESS-archive.md`
