# 项目进度

> 本文件用于跨会话状态记录，新会话开始请先阅读。

## ✅ 已完成

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

- **`UInt64Type` 由 `uint64_t` 改为 `unsigned long long`（2026-09-14，用户决策"全改"）**：昨天把类型调色板统一到明确位数的整型时，`Int64` 因"Linux 上 `int64_t` 是 `long`，会让全仓 `%lld` 变成格式不匹配"而**特意保留 `long long`**，但无符号侧漏了同一条——`uint64_t` 在 Linux 上是 `unsigned long`，`%llu` 同样不匹配。本轮补齐这个对称性。
  - **起因是"没有消费方"这个事实**：`UInt64Type` 全仓（`grep` 整个 Spark）**只有 typedef 那一行本身**，零消费者；跨仓扫描只有 `DBAdapters/test/TestDB/TestDB.cpp:799` 一个同名的 `char UInt64Type[16]` 列，与该别名无关。**唯一真正消费 64 位无符号的地方是 `StepUtility::WriteString(char*&, int, uint64_t)`**，而它为了配 `%llu` 已经写了 `static_cast<unsigned long long>(value)` —— 这正是"别名选错，代价落在每个格式点上"的现行证据。
  - **改动（两仓，共 5 处）**：`../Templates/Cpp/Spark/Types.h.tpl` 的 uint64s 段 `typedef uint64_t` → `typedef unsigned long long`，尾部断言块补一条 `static_assert(sizeof(unsigned long long) == 8, …)`（原本只有 `long long` / `double` / `bool` 三条）；`pump.py` 重新生成 `include/Spark/Types.h`；`StepUtility.h` / `StepUtility.cpp` 的 `uint64_t` 重载改为 `UInt64Type` 并**去掉那个 `static_cast<unsigned long long>`**。
  - **只改别名是个必崩路径，重载必须同步改（已实证）**：若别名动了、重载停留在 `uint64_t`，则 Linux 上 `UInt64Type`(`unsigned long long`) → `uint64_t`(`unsigned long`) 属整型转换，而文件末尾 `template<typename T> WriteString(char*&, int, T)` 是**精确匹配**——更好的转换序列胜出，模板会赢，于是走 `sprintf("%d=%s", key, value)` 把整数当 `char*` 解引用。生成器对所有 uint64 字段发的那行 `WriteString(ppos, Items::X, field->X)`（`Templates/Cpp/Protocol/Step/StepPackages.cpp.tpl:164-204`）正好落在该路径上。**反向验证（Windows 上同样成立）**：临时注释掉该重载并重建，新增用例报 `SEH exception 0xc0000005`（访问违例，gtest 捕获后记为 FAILED）；恢复后 Debug/Release 各 367/367。
  - **Windows 侧完全无感，且因此"回退抓不住"**：`uint64_t` 与 `unsigned long long` 在 MSVC 上是同一类型（`unsigned __int64`），故无 ABI/行为变化、无重载分派变化，**本次改动的影响面只在 Linux**。推论已实测：临时把 `Types.h` 的别名改回 `uint64_t` 后重建，**Windows 上编译通过、367/367 依旧全绿**——即新增的用例与断言在 Windows 上都抓不住这次回退，**只有 Linux 侧能咬**。这正是下面补类型同一性断言的原因。
  - **补类型同一性断言（代码审查发现后新增）**：`Types.h` 里那条 `static_assert(sizeof(unsigned long long) == 8, …)` **守不住本次回归**——`sizeof(uint64_t)` 在 Linux 上也是 8，把别名退回 `uint64_t` 该断言照样通过。真正需要钉住的是**类型同一性**，故在 `include/Spark/Network/Protocol/StepUtility.h`（已有 `<type_traits>`，且这里正是 `%llu` 动机所在）文件作用域加了 `static_assert(std::is_same<UInt64Type, unsigned long long>::value, "UInt64Type 必须是 unsigned long long，%llu 才匹配")`；同文件补 `#include <Spark/Types.h>` 满足"头文件自包含"（原先靠 `Head.h` 传递包含）。**同一条断言在 Windows 上恒真、不可能触发**（同类型），它是 Linux 专用护栏。
  - **Linux 侧实证（GCC，`out/build/WSL-GCC-Debug` 已配置好，源目录就是 `/mnt/d/Gitee/Spark`）**：`cmake --build` 全量 103 目标零错误；`./bin/Debug/UnitTests` **366/366 通过**，其中 `StepUtilityTest.WriteString_UInt64` 单独跑也通过。**366 而非 367 是既有差异、与本改动无关**：`test/unittest/Core/DirTest.cpp:12/37` 有 `#ifdef _WIN32` 的用例，Windows 多这 1 条。
  - **新增用例 `StepUtilityTest.WriteString_UInt64`**：key 取 `0x9999`（不占用任何 `Items::` ItemID——模型里没有 UInt64 字段，故无真实字段可标，注释已写明"只验证重载分派"），值取 UInt64 上限 `18446744073709551615`，断言输出等于 `39321=18446744073709551615< SOH>`。断言取**精确文本**而非"非空"（同文件既有 8 个 WriteString 用例只断言"非空"，钉不住分派）——精确值同时钉住"落到 `%llu` 重载"和"没被 32 位重载截断"。
  - **实证（同机，x64）**：Debug 全量 61 目标、Release 全量重建零错误；`UnitTests` Debug **367/367**、Release **367/367**（改前 366，新增 1 条）；`StepUtilityTest` 47 → **48**。README 里的用例数先由 47 同步为 48，随后按用户决定**整体去掉**（见下条），故最终两份 README 都不再标注用例数。
  - **没动的地方（已核对，非疏漏）**：`../Templates/Cpp/Mdb/*.tpl` 里的 `'uint64_t'` 字面量（如 `MdbStructs.cpp.tpl:74`、`MdbPrimaryKeyComp.cpp.tpl:53`）是 `types[@name]` 映射表的值，**只用于 `== 'string'` 判断**，不生成 C++ 类型文本；真正的类型由 `std::hash<@type!!Type>()` 走别名，跟着 `Types.h` 走。该表里 `int64s` 早就写着 `'int64'`，同样不是 C++ 类型名，说明这张表的约定本来就是"分类标签"而非"类型文本"。
  - **顺带统一 `WriteString` 整套重载的整数拼写（2026-09-14，用户要求"这太乱了，需要统一"）**：用户发现改完 `UInt64Type` 后这套重载的整数类型"有 int8_t / int16_t / unsigned short / int / long long"，要求按 `int8_t…uint32_t + long long / unsigned long long`（选项 A）**或**全用别名 `Int8Type…`（选项 B）二选一。**关键事实：两个选项在类型上完全等价**——调色板本来就是 `Int8Type=int8_t … Int64Type=long long / UInt64Type=unsigned long long`，所以只差"写哪个名字"，不差语义。**选 A 的理由是失败模式而非风格**：调色板哪天被人改了宽度时，A 下没有重载能匹配、调用落到那个 `%s` 模板重载上**当场崩**；B 下重载照样匹配（名字没变）而格式串与真实宽度脱节，变成**静默的格式不匹配 UB**——正是本轮一直在打的那类 bug。A 更吵，所以更安全。
    - **改动**：`StepUtility.h` 的 13 个 `WriteString` 重载按宽度重排并统一拼写（`unsigned short` → `uint16_t`、`int` → `int32_t`、`UInt64Type` → `unsigned long long`）；**同一个类型两个写法混用**是现状的真实问题——`unsigned short` 就是 `uint16_t`、`int` 就是 `int32_t`。`StepUtility.cpp` 的 13 个定义同步重排。另把 4 个 `Get*` 函数与 2 处局部变量里的 `unsigned short`（都是 16 位字段 ID，即 `UInt16Type`）一并改为 `uint16_t`——用户在"乱"的清单里点名了 `unsign short`，这些出现在同一文件同一语义位置，留着就还是两个写法。`WriteHexString` 同步。**全部是同一类型的换写法，两平台符号名不变、零行为变化**（`unsigned short`≡`uint16_t`、`int`≡`int32_t`）。
    - **重排的机械验证（因为 4 个重载没有用例）**：`int8_t` / `uint8_t` / `int16_t` / `uint32_t` 这 4 个重载在 `StepUtilityTest` 里**没有对应用例**，重排时若把签名字和函数体配错，编译器不报、测试也抓不住。故先把改动前后的"签名 → 格式串"配对表各自抽出来（`grep -A2 '^void StepUtility::Write...' | paste - -`），把 `unsigned short`/`int`/`UInt64Type` 归一化后 `sort` + `diff`——**完全一致**，证明是纯移动。
    - **未动**：`int` 用作 `buff` 下标（`startIndex` / `endIndex` / `sohIndex`）与 `key` 参数——那是缓冲区偏移不是线上宽度，改成 `int32_t` 属于把宽度语义硬套到下标上，无收益；`ToProtocolStream` 返回 `int` 同理。`ParseInteger` 模板不受影响。
    - **实证**：Debug / Release 各 **367/367** 通过（`StepUtilityTest` 仍 48）。`Types.h` 的 `Int64Type` / `UInt64Type` 类型同一性断言增至两条（`long long` 与 `unsigned long long` 各一，见上条）。
    - **统一这轮的代码审查（2026-09-14）**：严重 **0** / 高 **3** / 中 **7** / 低 **5**，净结论"可合并，前提是高危三项在提交前落地"。审查**独立复核**了 4 个无用例重载（`int8_t`/`uint8_t`/`int16_t`/`uint32_t`）的签名↔格式串配对——没有沿用我的归一化 diff 法，逐行核对后确认 13 条全部正确；变参提升规则也确认处理正确（`uint8_t` 是 `unsigned char`、提升为 `int`，`%u` 不加 `static_cast<unsigned int>` 必错，两处都加了；`int8_t`/`int16_t` 提升为 `int` 与 `%d` 匹配，那两处 cast 冗余但无害）。
      - **采纳·我写错的一处断言（高）**：`:22` 原写"8/16/32 位不需要同款断言：两个目标平台上同宽度只有一种类型可选，宽度相等即类型同一"——**这句是错的**。Windows 上 32 位有 `int` / `unsigned int` / `long` / `unsigned long` **四种**基本类型，`int32_t` / `uint32_t` 只覆盖前两种；`WriteString(ppos, key, 某 long)` 的 `long` 与 `int32_t`(= `int`) 虽同宽但异型，模板实例化为**精确匹配**、胜过需要整型转换的非模板重载，于是同样落到 `%s` 模板上。`size_t` 更微妙：Windows 上是 `unsigned long long`（被新重载接住），Linux 上是 `unsigned long`（落模板），**同一份源码两平台行为不同**。这类"虚假的安全保证"比不写注释更糟，会把后来者引向"32 位已安全"的错误结论。已改写为准确表述：断言只能钉别名、管不住调用点传什么，所以**调用点只准传调色板里的类型**。
      - **采纳·其余三条**：删掉 `:40-41` 与 `:19-22` 同义重复的注释、测试里两行注释压成一行（§4 解释性注释非例外）；补 `#include <cstdio>`（该头里有 `sprintf` 却不自包含，而本轮恰好为"自包含"加了 `<Spark/Types.h>`）；`WriteHexString` 与 `GetNextFieldZone` 里 `WriteLog` 的 `%X` 补 `static_cast<unsigned int>`（`uint16_t` 提升为 `int`，`%X` 要 `unsigned int`——同一文件里 `WriteString(uint16_t)` 就规规矩矩写了 cast，属本轮确立的不变量）。
      - **高危第三项的澄清**：审查指出"这批全部是同类型换写法、两平台符号名不变、零行为变化"这一说法**就整个未提交 diff 而言不成立**——diff 里含上一轮的 `UInt64Type` 由 `uint64_t` 改 `unsigned long long`，那在 Linux 上**不是**同一类型（`unsigned long` vs `unsigned long long`）且删掉了 `WriteString(..., uint64_t)` 的 `NETWORK_EXPORTS` 符号。**该结论本身没错，只是适用范围要对**：统一这一轮（此时 `UInt64Type` 已是 `unsigned long long`）确实零行为变化；别名那一轮不是。两轮在本文档里本就分开记录（ABI 影响见下条），提交信息不能笼统写"零行为变化"。
      - **未决·需用户处理（高）**：`Types.h` 是生成物，本轮改动的**源头**在兄弟仓 `D:\Gitee\Templates\Cpp\Spark\Types.h.tpl`，该模板**尚未提交**。只提交 Spark 的话，新克隆/CI 重新 pump 会把 `UInt64Type` 回退成 `uint64_t`；届时 `UInt64Type` 侧有 `is_same` 断言能在 Linux 上编译报错，`Int64Type` 侧无此保护。**两个仓需一起提交**。
      - **已落地·`TryReadHexField` 出参由 `long long&` 收窄为 `uint16_t&`（2026-09-14，用户直接指示）**：用户指出"FieldID 本就是 16 位，你整个 `long long` 临时变量完全没必要"——确实，原实现在 `GetFieldStart` / `GetFieldEnd` 两个调用点各声明一个 `long long parsed` 再 `static_cast<uint16_t>`。出参改成 `uint16_t&` 后两处临时变量与 cast 全部消失，**并顺带把上条审查挂起的 ① 号截断缺陷一次收口**（范围检查落在这个函数内唯一那处转换旁）。检查同时拒绝 `parsed < 0`，因为 `strtoll` 允许前导符号、`6=-1` 会被转成 `0xFFFF`。**`GetFieldEnd` 里把"找尾部 SOH"提到了"读字段值"之前**——**事后证明这步是语义中性的 no-op，我给的理由不成立**（见下方审查结论）。**属行为变更**：畸形帧 `6=10000` / `6=1FFFF` 过去被静默截断，现在直接判非法。实证 Debug / Release 各 **370/370**（`StepUtilityTest` 48 → **51**）。注意 `TryParseInteger` 仍保留 `long long&`：它与 `HeadFromStream` / `TailFromStream` 共用，那边的 `MsgSeqNum`(int32) 与 `CheckSum`(可达 0xFFFFFFFF) 确实需要更宽。
      - **这轮出参收窄的代码审查（2026-09-14）**：严重 **0** / 高 **1**（DRY，待批）/ 中 **2**（均为既有代码），结论"本轮改动本身未发现缺陷，可以合入"。
        - **我错了一处·换序的理由**：我判断"丢掉临时变量后出参会被直接写，换序才能保住'失败不写出参'"，审查证明**这条路径根本不可达**——`GetNextSoh(buff, i, endIndex)` 与 `TryReadHexField` 内部那次 `GetNextSoh(buff, i+2, endIndex)` 找的是**同一个** SOH（`buff[i]='7'`、`buff[i+1]='='` 都不是 SOH，故 `[i, endIndex)` 内首个 SOH 必然 `>= i+2`），所以"读值成功而外层找不到 SOH"不可能发生；旧代码那个临时变量也从来不是为这个性质服务的。**换序无害但是多余的**，理由建立在不可能路径上。审查另核实：调用方（`Packages.cpp` 13 处）在失败分支只写日志、从不读 `fieldID`，所以该性质本就只是按函数成立的宽松保证、非被依赖的契约。**补充一条精确边界**：`GetFieldStart` / `GetFieldEnd` 成立"失败不写出参"，但 `GetNextFieldZone` 在 ID 不匹配这条失败路径上**会**写出参（Start/End 已各自赋值），只是调用方不读——别再当成全局不变量。
        - **我错了一处·第二个用例名实不符**：加范围检查后它在 `GetFieldStart` 那步就失败了，**走不到**注释里写的"截断后碰巧相等"路径，实际只覆盖了与第一个用例同一件事，而 `GetFieldEnd` 一侧完全没覆盖。**已换成 `GetNextFieldZone_FieldEndIDOutOfRange`**：起始 `6=100D` + 结束 `7=1100D`，结束 ID 截断后是 `0x100D`、与起始 ID 相等——旧代码确实会放行这个畸形帧（实测：去掉范围检查后该用例变红），新代码拒绝。这才是结束侧别名 bug 的真实演示。
        - **我错了一处·缺正向边界断言**：原先两个用例只断言 `EXPECT_FALSE`，若有人把检查写成 `parsed >= 0xFFFF`（越界一个、误拒合法的 `0xFFFF`）会全部照样通过。**已补 `GetFieldStart_MaxFieldID`**（`FFFF` 应 `true` 且 `fieldID == 0xFFFF`）。**三次反向探针**都做了：拿掉范围检查 → 两个负向用例变红、正向边界用例正确保持通过（369/370）；把上界写成 `>= 0xFFFF` → 只有 `MaxFieldID` 变红（369/370）。两轮探针都恰好命中设计的那几条，证明用例与检查是双向咬合的。
        - **审查另发现两个既有解析洞（非本轮引入，不阻塞，待批）**：①`TryParseInteger` 用 `*end != '\0'` 校验"停在第 1 个 NUL"而非"停到 `text.size()`"，而 `text` 是由 `std::string(buff + startIndex, buff + sohIndex)` 构造的**定长**串、中间 NUL 不会截断它，于是 `6=100\0垃圾` 会被当成合法的 `0x100` 接受、SOH 前的垃圾被静默吞掉——**本轮新增的范围检查挡不住它**（不越界）。最小修复是一行：改成 `end != text.c_str() + text.size()`。**切勿**改用 `from_chars`/`ParseInteger<T>` 代替：`HeadToStream` 用 `"%13d"` 空格右填充写 `MsgSeqNum`，`HeadFromStream` 正是靠 `strtoll` 跳前导空格才能读回自己的输出，换解析器会直接打破 `HeadStreamRoundTrip`——**所以"前导空白"在这里是载荷需求而非松懈**。②`GetNext` 里 `key = atoi(buff + startIndex)` 把 `int` 静默截断进 `uint16_t`（`69645` → `0x100D`、`65536` → `0` 即 Magic），让非规范编码静默别名到合法 key，既违反 `CLAUDE.md` §6，也与 `StepUtility.h` 自己那句"窄类型直接 atoi 会静默截断，所以必须走这里"的注释**自相矛盾**。
        - **DRY（高，待批）**：本轮新增的范围检查让"解析 + 范围检查 + 窄化"这个模式凑满**三段**（`TryReadHexField` 的 FieldID/16 进制/`uint16_t`、`HeadFromStream` 的 BodyLen/10 进制/`UInt16Type`、`TailFromStream` 的 CheckSum/16 进制/`Int32Type`），差异只有进制与上界，触发 §5 的"三段即封装"阈值。修法是把匿名命名空间的 `TryParseInteger` 改成带界模板、用 `std::numeric_limits<T>::min()/max()` 生成边界，调用点写 `TryParseInteger<uint16_t>(text, 16, fieldID)`，可一次消掉三处手写检查（注意 `T=uint64_t` 时上界会溢出 `long long`，当前无此调用点）。**该改动会触及 `HeadFromStream` / `TailFromStream`（本轮之外），按 §1 需先批**。审查建议它与上面两个解析洞**合并成一次独立小改动**处理。
      - **WriteString 重载改用调色板别名（即 B 方案，2026-09-14，用户决定）**：用户问"我的类型定义里本就没有 `long` 和 `unsigned long`，是不是用自定义类型来提供这些函数更合理"——**这个判断是对的，而且我上一轮推荐 A 的核心论据站不住**。核实 `Types.h` 的别名只有 `bool` / `int8_t` / `uint8_t` / `int16_t` / `uint16_t` / `int32_t` / `uint32_t` / `long long` / `unsigned long long` / `double`，**没有 `long`、没有 `unsigned long`**（也没有 char / string 别名）——`long` 在这个系统里不是一种存在的类型。于是审查那条"补 `long` / `unsigned long` 重载以闭合 32 位同宽异型缺口"的建议**方向就是错的**：它在给系统里不存在的类型加重载。问题根源不是缺重载，而是 A 把重载写成裸类型拼写、让重载集合看起来是一个**开放的** C++ 类型集合（于是必然被问"那 `long` 呢"）。
        - **我错在哪**：推荐 A 时说"B 下重载照样匹配、格式串与真实宽度脱节 → **静默**的格式不匹配 UB；A 下当场崩、更吵所以更安全"。两点都不成立：①格式串是**字面量**，`sprintf(ppos, "%d=%lld", key, value)` 里 `value` 一旦不是 `long long`，GCC 的 `-Wformat`（含在 `-Wall`）与 MSVC 的 C4477 都会**在编译期报格式不匹配**；②Linux x86-64 上 `long` 与 `long long` 共用同一个 64 位寄存器，`%lld` 读 64 位**碰巧是对的**。真正兜底的是 `static_assert`，而 A、B 共用同一个断言——"响 vs 静"的对比是我编出来的。
        - **改动**：`.h` 的 13 个重载形参 + `.cpp` 的 13 个定义形参由裸类型改为别名（`bool`→`BoolType`、`int8_t`→`Int8Type` … `long long`→`Int64Type`、`double`→`DoubleType`）；`char` / `std::string` / `char*` **保持裸类型**——它们不在调色板里，因为它们是**文本**而非"整数宽度"，与数值宽度不是一类事。**函数体逐字节未动，无行为变化**（别名本来就是那些类型，每个既有调用点绑定的仍是同一个重载）。
        - **断言由两条增至四条**：B 之下声明说的是调色板的话，断言就负责"这个别名必须正是格式串要求的那个类型"。判据是**体内没有 `static_cast` 或默认提升来吸收别名变化**：`Int32Type`（`%d`，体内无 cast）、`Int64Type`（`%lld`）、`UInt64Type`（`%llu`）、`DoubleType`（`%.6f`——`float` 会被 varargs 提升吸收，`long double` 不会）。**这第四条是补我上一轮的漏**：原注释称"32 位不加同款断言"，但 `Int32Type` 恰恰是那个体内没有 cast 的，该说法又一次范围写错。
        - **两个探针**：①把 `.h` 末尾那个兜底模板临时加上 `static_assert(!std::is_arithmetic<T>::value)` → 全仓编译干净、370/370 通过，说明**没有任何既有调用点落到那个 `%s` 模板上**（这是挂起提案"给模板加 `static_assert` 拒绝整型"安全性的直接证据）。②把生成文件里的 `Int32Type` 临时改成 `long`（MSVC 下同为 4 字节但**不同类型**，正是宽度断言抓不到的情形）→ `StepUtility.h:25` 报 `error C2338: static assertion failed: 'Int32Type 必须是 int，%d 才匹配'`——**这是几轮以来第一次观察到这套同类型断言真的触发**（此前同类断言在 Windows 上因类型同一而从未被看到响过）。两个探针均已撤回，恢复后 Debug / Release 各 **370/370**。
        - **注释重写（§4 例外，已报用户确认）**：文件开头那段原本是专为 A 的拼写写的，B 之下前提变了，改为"重载形参一律用调色板别名，重载集合与 Types.h 的别名一一对应……代价是别名改了类型时重载照样匹配、只有 .cpp 里的格式串会失配……本套重载只覆盖调色板里的类型，传裸 `long` / `size_t` 会落到末尾那个模板重载上按 `%s` 把整数当 `char*` 解引用"。
      - **已落地·`Items.h` 的 `unsigned int` 改为 `UInt16Type`（2026-09-14，用户指示"模板改 + 加 include + 重新生成 + 补 8 处 cast + 重验"五步全部做完）**：用户在 B 方案之后接着指出 `Items.h` 里硬编码的 `unsigned int` 应该用 `UInt16Type`——**判断是对的**，ItemID 在线路上就是 16 位（`WriteHexString` 用 `%04X` 写、`GetNext` 用 `uint16_t` 接、上一轮刚加的上界就是 `0xFFFF`）。**但 `Items.xml` 里每条自带的 `type=` 不能拿来用**：那是**值的类型**而非 ID 的类型（`MessageChain` 写着 `type="Bool"` 而 ID 是 `0x0003`，照 `type` 走会生成 `static constexpr bool MessageChain = 0x0003;` = `true`），与 Mdb 模板里 `'uint64_t'` 只作分类标签是同一个模式。
        - **改动（两仓）**：`../Templates/Cpp/Spark/Network/Protocol/Items.h.tpl` 第 11 行 `static constexpr unsigned int !!@name!! = !!@id!!;` → `UInt16Type`，并在 `#pragma once` 后**紧跟**加 `#include <Spark/Types.h>`（与同目录既有生成头 `Head.h` / `ProtocolVersion.h` 逐字一致——初版我插了一个空行，核对后去掉）；重新 pump 生成 `include/Spark/Network/Protocol/Items.h`（`+299 / -298` = 298 条常量 + 1 行 include；生成物纯 LF、无残留 `!!` 占位符、重复 pump 幂等，审查侧在临时目录重跑生成比对**逐字节相同**，证明无手改）；`StepUtility.cpp` 7 行共 **8 处** `Items::*` 实参加 `static_cast<unsigned int>`（`:159`、`:258`~`:263`、`:346`）。Debug / Release 各 **370/370**（`StepUtilityTest` 51），两配置零编译告警。
        - **模型文件位置（我记错了一次）**：`Items.xml` **不在 Spark 仓内**，在 `D:\Gitee\Model`（Spark 的兄弟目录，自成 git 仓，含 `Items.xml`/`ShortItems.xml`/`Types.xml`）；`D:\Gitee\Spark\model\` 是另一个目录，只有 `Head.xml` 与 `parselist.xml`。按 pump 真正读取的 `../Model/Items.xml` 重核：298 条、最小 `0x0`、最大 `0xA00E`、**零条超 0xFFFF**、零重复 ID。
        - **我错了一处·"格式不匹配编译期会被抓住"**：我先前说这类失配 `-Wformat`/C4477 会在编译期报出来。**在本工程不成立**：`CMAKE_CXX_FLAGS` 被覆写为 `/DWIN32 /D_WINDOWS /EHsc`、**没有任何 `/W` 等级**（MSVC 默认 /W1，而 C4477 是 level 4），`CMakeLists.txt`（只加 `/utf-8 /bigobj`）与 `CMakePresets.json` 里也没有 `-Wall`。**反向探针实证**：拿掉 `:258` 的 cast、并确认该 TU 确实重编（`[1/6] Building CXX ... StepUtility.cpp.obj`）之后，MSVC **零诊断**。审查用本机 MSVC 14.44 独立复测并补全：`%u` 配 `int`、`%d` 配 `unsigned int`、`%u`/`%X` 配 `uint16_t`、`%c` 配 `unsigned int` 在 `/W1`、`/W4`、`/Wall` **全部静默**，只有**宽度/种类**不匹配（如 `%lld` 配 `unsigned int`）才报 C4477。**所以这 8 处 cast 的动机成立**（C/C++ 要求变参类型与转换格式一致，GCC 的 `-Wformat` 会报 `int` 对 `%u`），**但在本工程的编译矩阵里没有任何一级告警能验证它**——收口应单开一次改动给两平台打开 `/W4`（并评估 `/WX`）与 `-Wall`，否则是"既要加 cast 又无法证明后续不退化"；该改动会牵出存量告警、规模需先评估，故不塞进本轮。
        - **我错了一处·探针未植入**：第一次反向探针按 CRLF 定位替换，但该文件是**纯 LF**，断言 0 处匹配后我仍继续跑了构建——那次跑的是未改动代码，结论无效（我最初还把那个"无告警"当成了证据）。已按行号重做。
        - **载荷性实证（头文件自包含）**：只 `#include <Spark/Network/Protocol/Items.h>` 的独立 TU 编译通过（`cl exit=0`）；把那一行 include 注释掉立刻 298 个 `C3646`/`C4430`。原先 5 个 include 它的文件（`StepUtility.cpp`、`Network.h`、`Packages.cpp`、`PackageSerializationTest.cpp`、`StepUtilityTest.cpp`）**没有一个**自带 `Types.h`，原先全靠传递包含。
        - **收窄影响面（无真实风险）**：全仓 3508 行 `Items::`——1746 个 `case` 标签、1740 个 `WriteString`/`WriteHexString` 的 `int key` 实参、12 行 `MakeStepField(unsigned int key, ...)`、1 处 `std::to_string(Items::Version)`；无移位 / `sizeof` / 模板实参 / 数组维度 / 取地址等危险形态。审查补扫了唯一会**改变重载选择**的形态 `WriteString(ppos, SomeKey, Items::X)`——在 Spark / `Templates` / `QuantTrading` / `SAMS` 四棵树里 **0 处**。`std::to_string(Items::Version)` 由 `to_string(unsigned)` 改选 `to_string(int)`，值 8、输出文本相同。
        - **本轮代码审查结论**：严重 **0** / 高 **1**（即下面的 16 位护栏，审查建议作为本轮末尾一步落地）/ 中 **5** / 低 **3**，结论"本轮改动本身干净、可合并；生成物可复现、类型收窄对所有现存用法保值、下游无源码破坏"。**审查另指出一个我完全漏掉的同源问题**：`%c` 的实参 `SOH` 是 `constexpr unsigned int`（`StepUtility.h:18`）而 `%c` 要 `int`，全仓共 **8 处**（`:159` 两处、`:258` 两处、`:259`~`:262` 各一处；审查**原报 9 处**、把格式串是 `"%u=%d"`（根本没有 `%c`）的 `:263` 也算了进去，见下条勘误）。**审查也确认了 8 处 cast 不属 §5 DRY 违规**（同一个运算符作用于 8 个不同操作数，不是"仅参数不同的相似逻辑"；抽 `ToUnsigned()` 反属过度封装），但它顺带指出同文件里 6 行同形状 `snprintf` 才是够得上 §5 形状的重复，且正解方向是让这些字段改走 `WriteString`/`WriteHexString` 重载而非把 cast 集中。
        - **已落地·16 位护栏（2026-09-14，用户批准落地）**：今天**没有任何东西在守"ItemID 必须落 16 位"**——`= 0x10000` 这种复制初始化在**本工程现有 flags 下完全静默**，连 `/W4` 都只给 `C4305`/`C4309` 告警（且无 `/WX`），`Items.xml` 里写错一个 ID 构建照样通过。**后果不是"常量值不对"而是静默线格式损坏**：ID 同时是写在线上的 key（`WriteString` 的 `%d`、`HeadToStream` 的 `%u`），`0x10000` 截断成 `0x0000` 就与 `Items::Magic` 撞成同一个 key，而接收侧刚把 key 上界钉在 `0xFFFF`、**永远还原不出 `0x10000`**。**修法**：模板第 11 行由复制初始化改为列表初始化 `static constexpr UInt16Type !!@name!!{!!@id!!};`——**一处改动、零新增行**。备选是逐条 `static_assert(!!@id!! <= 0xFFFF, ...)`，**必须用原始字面量**——写在已被转换后的常量上是**恒真**的（审查实测该断言从不触发，属装饰品），故未采用。**端到端实证（真模板 + 改坏的模型，全程在临时目录、未触碰任何仓库文件）**：把真实 `../Model/Items.xml` 复制一份、只把 `Magic` 的 `id="0x0000"` 改成 `id="0x10000"`，用真实模板 pump 出 `static constexpr UInt16Type Magic{0x10000};`，按工程同款 flags 编译 → `error C2397: 从"int"转换到"const UInt16Type"需要收缩转换`（该头第 9 行）、`cl exit=2`。**正向控制**：真实 `Items.h`（298 条花括号常量）两配置编译干净，边界 `0x0000`/`0x0008`/`0xA00E`/`0xFFFF` 均零诊断。**代价**：与 `Head.h.tpl` 那两行 `static constexpr UInt16Type FieldID = 0x0001;` 写法不再统一（用户已接受）；`Head.h` / `ProtocolVersion.h` 的 `FieldID` 同属 16 位 ID，是否也收待定。
        - **待批·同源 16 位护栏缺口（2026-09-14 复审发现，属既有问题、需先批）**：护栏只落在 `Items.h` 上，**同一个未加护栏的 `= !!@id!!` 形态还有三个模板**，其中两个的暴露面大于 `Items.h`，数字均经我独立复核：①`Templates/Cpp/Protocol/Packages/Fields.h.tpl:9` → `test/Packages/Spark/Fields.h` **174 条** `FieldID`，且是**真实暴露**——`Packages.cpp.tpl:132,152` 用 `WriteHexString(..., XField::FieldID)` 配 `%04X` 写成 16 位、`:262` 用 `memcpy(buff + offset, &XField::FieldID, sizeof(UInt16Type))` **取地址、按 2 字节读**、`:171`/`:280` 还把 `FieldID` 当 `case` 标签；②`Templates/Cpp/Protocol/Packages/Packages.h.tpl:30` → `test/Packages/Packages.h` **188 条** `PackageID`，各包的 `ToStepStream` 里 `Head.PackageID = PackageID;`（实测 **188 处**，与常量条数一一对应）随后由 `HeadToStream` 用 `%04X` 写出；③`Templates/Cpp/Spark/Network/Protocol/Head.h.tpl:12` → `Head.h:10,23` 两条 `FieldID`——**本工程内暂无消费方**（`grep HeadField::FieldID` / `TailField::FieldID` 命中 0），属潜在而非现实暴露。**跨仓爆炸半径大于 `Items.h`**（原写"**5 个 pumplist**"，2026-09-14 随写路径批次复核后**更正**）：`grep -rn "Fields.h.tpl\|Packages.h.tpl" --include=pumplist.xml /d/Gitee` 命中 5 个仓 16 条，但真正指向**存在的**模板的只有 **2 个仓**——`QuantTrading/pumplist.xml:31,32` 与 `Spark/test/pumplist.xml:2,3`，路径是 `../Templates/Cpp/Protocol/Packages/`。其余 3 个仓（`LibTest/src:15,19`、`SAMS/Api:12,21,30,39`、`SAMS/Source:70,71`）写的是 `../Templates/Cpp/Packages/`，而 `D:\Gitee\Templates\Cpp\` 下**没有 `Packages` 目录**（只有 `Protocol/` 等），属**死引用**；且这 3 个仓本就是已废弃项目。`Items.h.tpl` 只被 `Spark/pumplist.xml` 一个引用，这一条仍成立。改法与本轮同构（模板改列表初始化 + 重生成，一处改动、零新增行），但同样要**逐仓重生成并成对提交**，规模需先评估。
        - **已落地·同批同源 cast 一并收口（2026-09-14，用户批准"两类全补"）**：①`head->Version` / `head->PackageID` / `head->BodyLen`（皆 `UInt16Type`）配 `%u`/`%04X`/`%05u` → `static_cast<unsigned int>(...)`，**3 处**。**这一项属既有问题**（`Head.h` 本轮未改、字段本来就是 `UInt16Type`，上一版就一直不匹配），补它是因为不补就是**半收口**——同一行里前一个实参被刻意 cast、后一个同类型同问题的实参原样留着，下一个读者会以为这一类已经清完。②`SOH`（`StepUtility.h:18` 的 `constexpr unsigned int`）配 `%c` → `static_cast<int>(SOH)`，**8 处**（`:159` 两处、`:258` 两处、`:259`、`:260`、`:261`、`:262`）。**我据实更正了审查的计数**：审查报 9 处、把 `:263` 也算了进去，但该行格式串是 `"%u=%d"`、根本没有 `%c`；`:173` 的 `sprintf(ppos, "%d=%c", key, value)` 里 `value` 是 `char`、本来就配 `%c`，不需改。故同源补的是 **3 + 8 = 11 处**。按意图**未加 cast** 的两处已逐个核对 `Head.h`：`:262` 的 `head->MsgSeqNum` 是 `Int32Type`(`int`) 配 `%d` ✓、`:263` 的 `head->MessageChain` 是 `BoolType`(`bool`) 经默认提升配 `%d` ✓。文件内 cast 现况：`Items::` 8、`head->` 3、`SOH` 8、`tail->CheckSum` 1。**实测**：两配置各 **370/370**（`StepUtilityTest` 51）、零编译告警；Debug 侧确认真实重编 8 个目标（`StepUtility.cpp`、`Packages.cpp`、`PackageSerializationTest.cpp`、`StepUtilityTest.cpp` 等），包头往返那组用例（`HeadStreamRoundTrip` / `_MinValues` / `_MaxValues` / `HeadFromStream_BodyLenOutOfRange` 等）全绿。**行宽**：本批 5 行落在 158~171 字符——`cpp-style.md` §3 的 150 阈值只约束函数声明/定义的参数列表，这些是调用语句；`src` 下既有最长行 206 字符、`.editorconfig` 未设 `max_line_length`，故保持单行不折。**审查另提的那个备选已于下一批落地**：`StepUtility.h` 的 `constexpr unsigned int SOH = 1u;` → `constexpr char SOH = '\x01';`。改格式串之后那 8 处 `%c` cast 连同 `sprintf` 一起消失，不再需要。改前已核实**不存在** `WriteString(..., SOH)` 形态的调用（否则会换到 `char` 重载、线上文本由 `1` 变成 `\x01`），故当时那 28 处引用全是 `char` 语境、安全；测试里的 `kSOH` 改为直接用头里的 `SOH`。
        - **已落地·`int key` 形参拼写（2026-09-14，随写路径批次落地；含一处已提交错误的更正）**：上文原写"`WriteString` / `WriteHexString` / 四个 `Get*` 的 `key` 形参是 `int`"——**后半句是错的**。`git show HEAD:include/Spark/Network/Protocol/StepUtility.h` 逐行核对：四个 `Get*`（`GetNext` / `GetFieldStart` / `GetFieldEnd` / `GetNextFieldZone`）的 `key` / `fieldID` **早已是 `uint16_t&`**，`GetNextSoh` / `GetNextEqual` 也只收下标；真正是 `int` 的只有**写侧**——`WriteString`（13 个重载 + 兜底模板）与 `WriteHexString`。故本批把写侧全部改为 `UInt16Type`、四个 `Get*` 一并改成调色板别名拼写（同类型、零行为差异），1740 个 `Items::*` 实参零改动、测试里的 `int` 字面量实参（如 `0x9999`）也无需改。属**公开 API 变更**，用户已批准。
        - **跨仓提交耦合（同 `Types.h` 那条）**：`Templates` 的 `Items.h.tpl` 与 Spark 的 `Items.h` 是同一变更的两半，**必须成对提交**；只提 Spark 一侧的话，下次在旧模板上重跑生成会把类型改回 `unsigned int`。下游 `QuantTrading`、`SAMS\Api\*` 直接 include `Libs/Spark` 安装副本里的这个公开头（用法同样只有 `case` + `key` 实参两种形态，无源码级破坏），建议合并后各重建一次。审查另提一个可选 CI 思路：既然"重跑生成 = 逐字节一致"已验证，可加一条"重生成后 diff 为空"的检查，同时防手改生成物与模板脱节。
        - **登记·既有偏差（本轮不动）**：生成物 298 行全是 TAB 而 `.editorconfig` 写 `indent_style = space`；`pump.py` 以 `UTF-8-SIG` 输出故 `Items.h` **带 BOM**（`Head.h` 同，属生成器行为、不是手改痕迹）；`.tpl` 是 CRLF 而 `.editorconfig` 要求 LF（输出侧是 LF，故生成物仍合规）。要改就是全量生成物重排，按 §1 属大规模改动需先批。
      - **待批（均超出本批范围）**：①~~14 处逐字节相同的尾两行~~**已落地**，且收口方式与这里原设想的不同：不是抽出私有 `AdvanceStepField`，而是把"写入 + 补 SOH"整体搬进 `StepWriteCursor::AppendField`，于是 15 个调用点各剩一行，重复连同容量上界一起消失（**仍未**把 13 个重载并成一个模板——理由不变）。②~~`PROGRESS.md` 超 §8.1 的 50 KB 目标、归档层一直没做~~**已落地**（2026-09-14，用户批准"现在做，脚本驱动"），见本区顶部那条。**原 ②（补 `long` / `unsigned long` 重载）随 B 方案落地而作废**——那条建议本意是补 A 拼写的缺口，而 `long` 根本不是本系统的类型。
      - **明确不在本批处理**：参数/局部用 camelCase（`fieldID`/`startIndex`/`sohIndex`/`ppos`）、常量 `SOH`/`StepTailLen`/`StepMaxHeaderLen` 缺 `k` 前缀、include 分组未按字母序且项目头与标准库头间无空行——均为全仓系统性既有偏差，改名会牵动上千处调用点。
  - **代码审查结论**：严重 / 高危各 **0** 项，结论"本轮改动可以合并"。审查同时确认了三条我原先的判断——Mdb 模板里的 `'uint64_t'` 确实只作标签、无遗漏消费方、注释属 `CLAUDE.md` §4 允许的例外（语言/重载分派坑）。**已采纳**：类型同一性断言、`<Spark/Types.h>` 显式 include、用例 key 由 `0x0002`（就是 `Items::BodyLen`，语义冲突）改为 `0x9999`、上条跨项目影响的更正。**未采纳/待用户决定**：①`WriteString` 用 `sprintf` 且不带容量（**既有违规、非本轮引入**；修它要给公开 `WriteString` 加容量参数，按 §3.1 属公开 API 变更，需先问）；②`StepUtilityTest` 里 `char buff[64]…*ppos='\0';` 这段样板已重复 **9 次**（§5 DRY；抽 helper 要改既有 8 个用例，按 §1 禁止擅自重构，待批）；③给末尾那个兜底模板加 `static_assert` 拒绝整型/浮点（把"漏补重载"从运行期 SEH 提为编译期错误，属行为变更，待批）。**明确不改**：用例名 `WriteString_UInt64` 保留——它命名的就是调色板项（`Model/Types.xml` 的 `UInt64`），而相邻的 `_UnsignedShort` / `_LongLong` 命名的是裸类型重载，两者命名对象本就不同。
  - **跨项目影响（原记录有误，2026-09-14 代码审查后更正）**：先前写成"QuantTrading / QuoteHub 等共用仓，其他项目下次 pump 后 `Types.h` 也会变"——**这是高估**。实测 `grep -rln "Templates/Cpp/Spark/Types.h.tpl" --include=pumplist.xml /d/Gitee` **只命中 `./Spark/pumplist.xml` 一条**；Mdb / DBAdapters 等是经 `find_package(Spark PATHS "../Libs/Spark/x64-windows")` 消费**预编译安装包**，自己并不 pump 这个模板。改模板因此不会自动改变其他项目的生成物。**真正需要留意的跨项目点**是 ABI：`WriteString(char*&, int, UInt64Type)` 是 `NETWORK_EXPORTS` 类的静态成员，Linux 上参数类型由 `unsigned long` 变 `unsigned long long` 会**改符号签名字节串**，凡链接 `Libs/Spark/x64-linux` 预编译包的项目需重新安装 Spark 包（Windows 侧同类型、无 ABI 变化）。该符号是上一提交刚引入的，当前无外部消费者。两个仓都**未提交**，留待用户决定提交时机。
- **README 去掉单元测试用例数（2026-09-14，用户决定；关闭原 `Q.16`）**：`README.md` / `README.en.md` 的 Network 表四行删掉括号里的用例数（`StepUtilityTest` 48、`ProtocolUtilityTest` 9 + 7、`PackageReaderTest` 12、`PackageSerializationTest` 15），只保留覆盖内容描述，两份同步改。理由是该数每次增删用例都要手工同步、已出现滞后。`Q.16` 原文与关闭说明见 `PROGRESS-archive.md`。**未一并去除**（不在本次决定范围）：`README.md` 的"共 **23 个测试文件**"与两份 README 目录树里的文件数。
- 同步更新中英文 README（`README.md` / `README.en.md`），与当前工程状态对齐：
  - 构建徽标与前置要求：`CMake 3.10+` → `CMake 3.20+`（与 `CMakeLists.txt` 一致）
  - 修正构建产物说明：库文件输出至 `lib/<Config>`，可执行文件输出至 `bin/<Config>`，不再指向 `build` 目录
  - 修正测试可执行文件路径：`test/unittest/UnitTests` → `bin/<Config>/UnitTests`
  - 目录结构树更新：移除已不存在的 `CMakeSettings.json`（改为 `CMakePresets.json`），修正 `test/TestCommon/` 路径，补充 `Packages/`、`TestMD5/`、`bin/`、`lib/`、`out/`、`.workflow/`、`Install.sh`
  - 修正单元测试用例数：StepUtilityTest 36、ProtocolUtilityTest 8、PackageReaderTest 14（PackageSerializationTest 仍为 6）
  - 脚本说明表改为实际存在的脚本（移除 `geninc.py`、`copyheader.py`、`copymodel.py`）
  - Core 模块补充 `ConfigStructs` 组件说明
  - 构建章节补充 CMake Presets 用法提示
- 按 `test/` 真实用法重写 README 三个代码示例（中英文同步）：
  - 日志示例：删除不存在的 `LOG_INFO/LOG_DEBUG/LOG_ERROR` 宏，改为真实的 `WriteLog(LogLevel::..., "printf格式", ...)`，并补全 `Init(argv[0]) / SetLogLevel / Start / Stop / Join` 完整生命周期
  - JSON 示例：改用尖括号 include，补充基于 `CharReaderBuilder` 的反序列化示例
  - Step 协议客户端示例：`{}` 格式串 → `%lld/%s/%d`，`IOModelType::Epoll` → `Select`（跨平台），对齐 `TestStepClient.cpp` 的包工厂、`ObjectPool` 分配、`Utility::Strcpy`、字段填充与 `Prepare/Send/Deallocate` 真实用法

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
