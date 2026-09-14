# 项目进度

> 本文件用于跨会话状态记录，新会话开始请先阅读。

## ✅ 已完成

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
