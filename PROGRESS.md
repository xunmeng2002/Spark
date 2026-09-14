# 项目进度

> 本文件用于跨会话状态记录，新会话开始请先阅读。

## ✅ 已完成

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
      - **待批（均超出本批范围）**：①~~14 处逐字节相同的尾两行~~**已落地**，且收口方式与这里原设想的不同：不是抽出私有 `AdvanceStepField`，而是把"写入 + 补 SOH"整体搬进 `StepWriteCursor::AppendField`，于是 15 个调用点各剩一行，重复连同容量上界一起消失（**仍未**把 13 个重载并成一个模板——理由不变）。②`PROGRESS.md` 实测 **87262 字节（约 85.2 KiB）**、**已超 §8.1 的 50 KB 目标 70%**（74554 → 87262，本批又增 12708；用户 2026-09-14 决定"本轮只记录不拆"，故至今未动），需按 `D.xx` 编号把已关闭子项移入归档。**原 ②（补 `long` / `unsigned long` 重载）随 B 方案落地而作废**——那条建议本意是补 A 拼写的缺口，而 `long` 根本不是本系统的类型。
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
- 更新 `docs/environment-setup.md` / `environment-setup.en.md`（中英文同步）：
  - vcpkg 路径改为**系统环境变量 `VCPKG_ROOT`** 为准：删除旧的"回退路径 `D:/Github/vcpkg/`"方案，注明 CMake 经 `$ENV{VCPKG_ROOT}` 定位、Windows 下由 `restore_vcpkg_root()` 读取 User/Machine 级环境变量
  - 补充设置 `VCPKG_ROOT` 后需重启终端 / VS 的提示；WSL 下注明 CMake 直接读 shell 环境变量（`restore_vcpkg_root()` 仅 Windows 生效）
  - 新增 `reuse_git_proxy_for_vcpkg()` 自动复用 git 代理的说明（解决 vcpkg 拉取超时）
  - 3.2 常见问题中废弃的 `CMakeSettings.json` 引用改为 `CMakePresets.json`，并强调 `VCPKG_ROOT` 系统环境变量 + VS 重启

- **Logger 退出路径补最后一次落盘 + 停止后写入判空（2026-09-12，用户要求"在最合理的地方处理"）**：问题由 QuantTrading 侧排查 DBAdapters 写库失败可见性时暴露——`FlushBuffers()` 全项目只在 `Run()` 内被调用，`Logger::~Logger()` 是空体，故 **Logger 线程最后一次 `Run()` 之后写下的日志永不落盘**：各 `ThreadBase` 派生对象的 `ThreadExit` 行（含 Logger 自己的 `Thread:Logger Exit`）以及宿主在 `Stop()` 之后才写的收尾告警全部丢失，而 `LogData::~LogData()` 只是把未推送的 `CurrBuffer`/`LogBuffers` 直接 `Deallocate`。**改动**：`Logger::ThreadExit()` 在 `delete m_LogData` 前调新增的 `FlushRemainingBuffers()`（不做 `SwapInnerLogBuffers` 里那次最长 1s 的等待，只把 `CurrBuffer` 推入队列并落盘，之后 `FlushBuffers()`）；`WriteToLog()` 首行加 `m_LogData == nullptr` 判空——停止后的写入静默丢弃，不再解引用空指针。`test/TestCore` 去掉 `Stop()` 前那个 1s `sleep_for`（它正是本缺陷的事实 workaround），并在 `Join()` 之后追加一行 `WriteAfterStop` 作为判空契约的回归点。**实证（同机同 harness，改前/改后）**：改前 `bin/Debug/Log/TestCore.20260912-184800.log` 中 `Thread:Logger Start` 1 次、`Thread:Logger Exit` **0 次**；改后 `TestCore.20260912-184932.log` 两者各 1 次，且去掉 sleep 后紧邻 `Stop()` 的 `TestSpark Stop.` 也在文件里。判空：临时移除该守卫重建后 `TestCore.exe` **段错误（exit 139）**，恢复守卫后 exit 0 且 `WriteAfterStop` 只在控制台、不在文件。`UnitTests` 339/339 通过、x64-Debug 全量目标重建通过。
- **Logger 析构兜底不成立（2026-09-12 结论，代码已回退为空体析构）**：上一版曾按"宿主不调 `Stop()`/`Join()` 时由 `Logger::~Logger()` 补 `Stop(); Join();` 兜底落盘"的思路改过一版，实测不成立，已全部回退（工作区与 `9380bfa` 一致）。**实证（TestCore 去掉 `Stop()`/`Join()` 模拟提前 return 的宿主；每组 5 次）**：在 `~Logger()` 入口加 `WaitForSingleObject(m_Thread.native_handle(), 0)` 探针，**5/5 返回 `WAIT_OBJECT_0`**（线程仍存活时应为 258 `WAIT_TIMEOUT`）——即**析构函数执行时日志线程早已终止**。原因是 Windows 在 `ExitProcess` 中先终止除主线程外的所有线程、之后才走静态析构/DLL detach，所以 `Logger::ThreadExit()`（含收尾落盘）在该场景下永不执行，析构里的 `Stop(); Join();` 只是空转（`join()` 同 tick 返回，`joinable()` 仍为 1 但句柄已 signaled）。`Logger ctor` 全程只出现 1 次，排除 MSVC magic static 在析构期重入构造的猜想。**析构里 `delete m_LogData` 是唯一新增的崩溃路径**：段错误率 2/5（另一轮 4/10，exit 139），此时 CRT/堆已在拆解中，`~LogData()` 的 `fclose`+`Buffer::Deallocate()` 落到失效的运行时状态上；改成只 `Stop(); Join();` 不 delete 后 0/5，保持空体也是 0/5。回退后复验：`TestCore.exe` exit 0，日志文件同时含 `TestSpark Stop.` 与 `Thread:Logger Exit`，`TestSpark WriteAfterStop.` 只在控制台；`UnitTests` 339/339 通过。
- **环境事件（2026-09-12，非代码问题）**：本次排查中 `bin/Debug` 下所有新编译的 exe 一度全部无法启动（bash 报 `Permission denied` / 退出码 126/127），连一个新建的 `hello world` 也一样。定位为 **Windows 智能应用控制（Smart App Control）** 当天从"评估"自动切到"强制"（注册表 `HKLM\SYSTEM\CurrentControlSet\Control\CI\Policy`：`SAC_PreviousState=2` → `VerifiedAndReputablePolicyState=1`），CodeIntegrity 日志中 81 条 3033/3117 拦截事件、正文为 "Smart App Control Block Deteails"，最早一条 19:10:39 拦的正是 `TestCore.exe`。该机制**不支持任何排除项/文件夹白名单**，只能整体关闭（用户已关闭，`VerifiedAndReputablePolicyState=0`，之后执行恢复正常）。以后若再遇到"刚编译的程序无法启动"，先查 `Get-WinEvent -LogName Microsoft-Windows-CodeIntegrity/Operational` 的 3033/3118 事件，不要先去改杀软排除项。

- **Logger：超长单行越界写 + 空 `FILE*` 落盘 + 建日志目录抛异常（2026-09-12，Release 前置修复；代码已改，未提交，待用户确认）**：三项都在日志初始化/写入路径上，前两项由本仓自查发现，第三项是修第二项时实测连带发现。
  - **越界写（`Logger::WriteToLog`）**：`unsigned len2 = vsnprintf(t_LogBuffer + len1, MaxLogLineContentLength, format, va)` 把返回值直接当"已写入长度"用。MSVC 的 `vsnprintf` 返回的是**本该写入**的长度（负数表示编码错误），内容被截断时该值不会随之变小——单条日志超过 `MaxLogLineContentLength`（64512）时 `len2 > 64512`，下一行 `LogLineLength - len1 - len2 - 1` 发生**无符号回绕**，`std::format_to_n` 的写入目标被推到 `t_LogBuffer`（64KB，thread_local）之外。改为先接 `int formattedContentLength`，`std::clamp(formattedContentLength, 0, static_cast<int>(MaxLogLineContentLength) - 1)` 后再转 `unsigned`（负值归零、超限收敛到可写区间）。
  - **空 `FILE*` 落盘 + 写不了日志的处置（`Init` / `ThreadInit` / `Run` / `CreateLogFile` / `FlushBuffers`）**：`fopen` 失败原本只有一句 `assert`（Release 下断言被去掉），空 `FILE*` 会一路流进 `FlushBuffers` 的 `fwrite`/`fflush`。**处置策略（用户决策：写不了日志就是启动失败，不能让宿主"无日志跑起来"）**：启动期判失败即 `fprintf(stderr, …)` + `std::exit(EXIT_FAILURE)`——库内直接退出，宿主无需自己判 `Init` 返回值；运行期（跨日换文件）失败则不让进程退出，改为 `WriteLog(LogLevel::Error, …)` 继续跑（该 ERROR 至少还能到控制台），下个跨日或下次重启自动重试。为此把"能否写日志"的判定点从线程内 `ThreadInit` 前移到 `Init`：启动期就 `CreateLogFile()` 并检查 `m_LogData->LogFile`，`ThreadInit` 不再重复打开（只留 `m_LogData == nullptr` 判空）；`FlushBuffers` 仍按 `LogFile` 是否为空决定是否落盘（不落盘也必须 `Deallocate()` 缓冲区）。
  - **建目录抛异常（`CreateLogDir`，验证上一项时实测发现）**：原 `return std::filesystem::create_directories(path);` 用的是无 `error_code` 重载——当 `log` 这个名字已被一个**普通文件**占用时它会抛 `filesystem_error`，而调用点 `Init` 无人接，宿主直接起不来（实测 `TestCore` Debug 退出码 3 / Release 127，且无任何输出）。改用 `std::error_code` 重载；注意该重载**目录已存在时同样返回 false（且不置 `error_code`）**，故成功判据取 `!errorCode` 而非返回值，否则第二次启动必失败。失败语义由"抛异常"变为"返回 false"，由 `Init` 判失败后退出。
  - **实证**（同机同 harness，改前/改后）：`test/TestCore` 在 `Stop()` 前有回归点——70000 字符单行 + 紧随的哨兵行（`Canary after oversized line.`）。正常启动两配置 `TestCore` exit 0，日志最长行 **64569 字节**（截断而非越界），哨兵行与 `Thread:Logger Exit` 各命中 1 次；`log` 被普通文件占用 → 两配置 **exit 1** 且 stderr 为 `Logger: create log directory failed, process exit. Path:log, …`（改前为退出码 3 / 127 且无任何输出）；预先把同名日志文件占为**目录**使 `fopen` 失败 → 两配置 **exit 1** 且 stderr 为 `Logger: open log file failed. Path:log/TestCore.<时间戳>.log` + `Logger: cannot write log file, process exit. …`（改前该情形下进程继续运行、只写控制台）。`UnitTests` Debug/Release 各 **339/339**。
  - **风险标注（§7）**：本项目新增了 `std::exit`——库内直接终止进程，这是用户明确选择的语义（`Init` 在 `Start()` 之前，没有已建立的状态需要收尾，宿主也没有返回值可判）。副作用是 `std::exit` 不做栈展开，即宿主在 `Init` 之前打开的资源（如已建的数据库连接）由操作系统回收而非析构清理；若日后宿主需要在 `Init` 之前持有资源，此处要改成"返回失败由宿主决定"。运行期失败**不**做退出处理，避免把"暂时写不了盘"升级成交易进程中断。`Run()` 里的跨日失败分支调用 `WriteLog`——此刻未持 `m_LogData->Mutex`（`SwapInnerLogBuffers`/`FlushBuffers` 已返回），与其它线程走同一条写路径，未引入新的竞态面；`FlushBuffers` 的 `isLogFileOpened` 仍是函数内一次取值，`LogFile` 只在日志线程的 `CreateLogFile` 里改。

- **线协议改造（2026-09-13，方案 A：16 字节报文头 + CRC32C，已实现并全量自测通过，已提交 commit 7952ef1）**：计划见 `docs/wire-protocol-revision-plan.md`（已随该提交入库）。目标是让两端在格式不一致时能立刻发现，而不是按各自的解释读同一串字节。
  - **报文头重排为 16 字节**：`Model/Head.xml` 改为 `Magic(Int) / MsgSeqNum(Int) / PackageID(UShort) / BodyLen(UShort) / Version(UShort) / MessageChain(Bool) / Reserved(Bool)`，新增 `Tail` 的 `CheckSum(Int)`。原字节序下 `PackageID`/`BodyLen` 在前，现在魔术字落到偏移 0，接收端可以只凭前 4 字节判断"这是不是我说的协议"。框架从 14 字节变成 **20 字节**（头 16 + 尾 4）。
  - **`ShortItems.xml` 新增两个 tag**：`Magic`（tag = 0，值固定为文本 `SPK2`）与 `Version`（tag = 8）；`CheckSum` 由 `UShort` 改 `Int`。两个 id 都**显式写在文件末尾**——`ParseShortItem.py` 会给没有 `id` 的 `<item>` 顺序编号，插在开头会把后面的 id 全部顶掉（`BodyLen` 会撞上 `PackageID` 的 `0x0001`）。重新生成后用 diff 核对：`Items.xml` 只变了 3 行，`AdminUserID` 仍是 `0x1002`，没有 id 漂移。
  - **`ProtocolVersion.h`（新增，唯一真源）**：`ProtocolVersionValue = 2`、`ProtocolMagicValue = 0x324B5053`（线上小端为 `53 50 4B 32`，即 ASCII `SPK2`）、`ProtocolMagicText = "SPK2"`，并用 `static_assert` 钉住 `sizeof(HeadField) == 16`、`sizeof(TailField) == 4`、单帧开销 20 字节、小端、魔术字非 0、文本与字节镜像一致。这些断言放在**手写头**里而不是模板里：模板不动，重新 pump 后约束依然生效。
  - **校验和换 CRC32C**：`CalculateCrc32c` 用编译期生成的 256 项查表（反射多项式 `0x82F63B78`），覆盖报文头与包体。标准校验值 `"123456789" → 0xE3069283` 已作为用例固化。
  - **接收端重同步**：`PackageReader` 先对齐魔术字（找不到就丢弃无意义前缀，只保留末尾不足一个魔术字的字节等下一次收包，`m_DiscardLength > MaxPackageSize` 才判定为垃圾流断链）；校验失败只丢 1 字节重新扫描，**不再清空整段缓冲**。校验顺序改为版本 → 长度 → 校验和 → 包是否可收，版本不符直接返回失败断链（此时 `BodyLen` 的语义本身已不可信）。
  - **顺带修掉的三个缺陷**（前两个是本轮实现时自己引入/发现的）：`Protocol::OnRecv` 的接收 Buffer 在四条退出路径上都不归还（`ObjectPool` 的 free-list 语义下等于永久泄漏），并改用 `find` 而不是 `operator[]`（后者会插入空项）；`TcpBase::DoRecv` 在 `m_IOSubscriber` 为空时同样漏归还；`TailToStream` 用 `snprintf(buff, StepTailLen, "%u=%08X%c", …)` 时结尾的 SOH 会被 NUL 挤掉（线上报尾少 1 字节），改为 snprintf 只写 8 位十六进制、SOH 手工补。
  - **Step 头长不再手工镜像**：删掉 `StepHeaderLen`，改为 `HeadToStream` 返回实际长度 + `StepMaxHeaderLen = 128` 作为"超过它还没解析出包头就判非法"的上界。`HeadFromStream` 变为严格版：六个头字段全部解析且逐个校验通过才返回 true，值用 `strtoll` 的 `TryParseInteger` 解析（原来的 `std::stoi` 遇到对端送的脏字节会抛异常穿过线程入口直接终止进程）。
  - **锚点查找抽成一份（DRY）**：重同步要"在缓冲里找魔术字"，`StepUtility::GetPackageStart` 早就在做同一件事。新增 `ProtocolUtility::FindBytes` 作为唯一实现，`PackageReader::AlignToAnchor` 与 `GetPackageStart` 都调它（原来写在 `PackageReader.cpp` 匿名命名空间里那份已删除），`GetPackageStart` 顺带补上 `endIndex <= startIndex` 的判空。重写后 `AlignToAnchor` 改为直接调 `FindBytes` + `GetPackageStartAnchor`，`GetPackageStart` 已无生产调用方，已于 2026-09-13 删除。
  - **实证（同机，x64-Debug）**：`bin/Debug/UnitTests.exe` **365/365 通过（25 套件）**（Network 子集 90：`StepUtilityTest` 47、`PackageSerializationTest` 15、`PackageReaderTest` 12、`CalculateCrc32cTest` 9、`FindBytesTest` 7）；`cmake --build out/build/x64-Debug` 全量目标（含 `TestClient`/`TestServer`）零错误。新增用例覆盖：CRC32C 标准向量与"前导 0x00 不被跳过"、坏校验和/坏头被丢弃后**其后的好帧仍能完整解出**、魔术字跨收包边界一个字节不丢、版本不符判为致命错误、`BodyLen` 越界与值带尾巴（`2abc`）判失败。
  - **风险标注（§7）**：这是**线上格式变更**，`ProtocolVersionValue` 从 1 升到 2 后，新旧两端互连会在版本校验处断开——这正是本次改造要的语义，但升级窗口内必须两端同时换。`PackageReader::AlignToAnchor` 在垃圾流上会遍历全缓冲找魔术字（最坏 O(n)，n ≤ 128 KB），这是断链前的一次性开销，未做优化。`m_DiscardLength` 归零时机是"成功解出一帧"，即偶发坏帧不会累积计入上限。
- **协议改造连带清理（2026-09-13，已提交）**：按用户批准，删除协议改造后失去全部调用方的三处死代码。
  - `model/XtpHead.xml`：生成 `Head.h` 的旧模型，内容是 `Head(PackageID/BodyLen)` + `Tail(CheckSum UShort)`，即改造前的旧布局。`pumplist.xml` 早已指向 `model/Head.xml`，全仓无脚本引用它，留着容易被误当成现行定义。
  - `PackageReader::Shift`：重写后消费路径统一走 `PopFront` / `DiscardFront`。它与 `PopFront` 语义**不同**（只前移 `m_Data`，不 `memmove`、不回收前部空间），删掉同时消除了这个混用陷阱。
  - `StepUtility::GetPackageStart`：`AlignToAnchor` 改为直接调 `FindBytes` + `GetPackageStartAnchor` 后无生产调用方。
  - 连带删除的用例：`PackageReaderTest` 14 → 12、`StepUtilityTest` 51 → 47（含 `EmptyBuffer_AllFunctionsReturnFalse` 里那条 `GetPackageStart` 断言）；`ResetRestoresState` 中作为前置动作的 `Shift(2)` 换成语义等价且仍在的 `PopFront(2)`，不削弱该用例。`CalculateSum` 及其 8 个用例已于同批先行删除。
  - 清理后 `ProtocolUtility::FindBytes` 的生产调用方只剩 `PackageReader::AlignToAnchor`，但它仍是被删掉的那份匿名命名空间实现的唯一替代，保留；`StepUtility::GetPackageStartAnchor` 仍有 `AlignToAnchor` 与 3 个锚点用例在用，保留。

- **协议尺寸常量收口（2026-09-13，`ProtocolVersionValue` 保持不变仍为 2，用户决策）**：起因是上一批复审时发现"一帧最大多少"在代码里**没有单一出处**——`BodyLen` 的字段上限 65535、接收侧 `MaxPackageSize`（131072）、IO 层 `BuffSize`（65536）互不约束，其后果是**最大帧比发送缓冲大 19 字节**（65535 − 65516，即帧开销 20 − 1）。校验：改造前帧开销 14，这个洞当时是 13 字节 —— 是本次改造把它撑大的，不是新引入的。生成器 `ToXtpStream`/`ToStepStream` 不看 `size` 参数、`Head.BodyLen` 又是 16 位字段，所以一旦某个模型的字段长到越界，`MakePackage` 会先越界写 `Buffer<BuffSize>`、再把长度静默截进 `BodyLen`。当前没有这么大的包，属**潜伏缺陷**（不可达但无护栏）。
  - **新增两个协议常量**（`include/Spark/Network/Protocol/ProtocolVersion.h`，与 `ProtocolMagicValue` 同处）：`MaxFrameSize = 64 * 1024`（一帧总长上限，头 + 体 + 尾）、`MaxFrameBodyLen = MaxFrameSize − 20 = 65516`（包体上限）。配一条 `static_assert` 钉住"包体上限必须能写进 `BodyLen`（UShort）"，否则长度会被静默截断。
  - **三条尺寸关系用断言钉死**：`Package.h` 里 `static_assert(MaxPackageSize >= MaxFrameSize)`（读侧缓冲要放得下整帧）；`Protocol.cpp` 里 `static_assert(BuffSize >= MaxFrameSize)`（IO 层收发共用的 `Buffer<BuffSize>` 要放得下一帧——这正是 19 字节洞的入口，以前靠"两个 64KB 恰好相等"的巧合成立）。`BuffSize` 是 IO 层 `TcpBase::Send/OnRecv`、`ShmBase::Send`、`Connect` 共用的缓冲类型，不能由 Protocol 私自换尺寸，所以选择"断言约束"而不是"改尺寸"。
  - **写侧补长度契约（`Package::MakePackage`）**：两条分支都先算 `bodyCapacity = size − 帧开销`，判 `bodyLen < 0 || bodyLen > bodyCapacity || bodyLen > MaxFrameBodyLen`，命中即 Error 日志 + `return 0`；通过后才 `static_cast<UShortType>` 回填 `BodyLen`。Xtp 分支在写报文头之前判定，**缓冲一个字节都不动**；`bodyLen > bodyCapacity` 一条同时覆盖"装不下调用方缓冲"和"调用方给的 size 为负"（`bodyCapacity` 用 `int` 减 `static_cast<int>(sizeof(...))` 算，避免 `size_t` 回绕）。
  - **读侧收同一条边界（`PackageReader::IsBodyLenWithinFrameLimit`）**：两个解析分支（Xtp / Step）在版本校验之后各调一次，`BodyLen > MaxFrameBodyLen` 的报文头判为噪声、丢 1 字节重同步，不再为一个永远不会来的 65 KB 包体等到 CRC 才自愈。抽成私有函数是因为两条分支的判定与日志行完全相同（DRY：两处重复即违规）。
  - **版本号不递增的理由**：合法包体区间由 65535 收窄到 65516，严格说属于"字段语义变化"，但协议尚未发布、且全仓没有任何模型产生 64 KB 量级的包，故按用户决策保持 `ProtocolVersionValue = 2`。
  - **实证（x64-Debug）**：新增用例 `PackageSerializationTest.OversizedBody_RejectedBeforeWrite`——假包（`OversizedBodyPackage`）只回报 `MaxFrameBodyLen + 1` 而不写字节：Xtp 分支返回 0 且缓冲首字节仍是哨兵 `'X'`（证明判定在写之前），Step 分支返回 0。`bin/Debug/UnitTests.exe` **366/366 通过（25 套件）**，`PackageSerializationTest` 15 → 16；`cmake --build out/build/x64-Debug` 全量目标零错误（新断言全部成立）。
  - **下游影响（重要）**：QT 通过**安装树**消费 Spark——`Spark_DIR = D:/Gitee/Libs/Spark/x64-windows`，所以本批改动不进入 QT 的编译，QT 侧 ninja 报 `no work to do` 是正确的，不能当作"已生效"。要让 QT 用上新常量，需先重装 Spark（`cmake --install out/build/x64-Debug`，`CMAKE_INSTALL_PREFIX` 已配置为该安装树）。QT 全仓无 `MaxPackageSize`/`MakePackage` 引用，无需跟改。
  - **风险标注（§7）**：判定落在每帧一次的收发热路径上，只有整数比较，无性能影响。`MakePackage` 的检查是**事后**的——生成器不看 `size`，所以它能阻止非法帧发出、能阻止长度被截断，**但不能**阻止序列化器本身的越界写；写前防护需改 `../Templates`，见待决策条。

- **类型宽度显式化（2026-09-13，`Templates` + `Model` + Spark 生成物）**：线上是按字段宽度逐字节 memcpy，所以"别名的宽度"就是"线上格式"；此前 `typedef int IntType` / `unsigned short UShortType` 把线上宽度挂在"编译器把 `int` 当几位"上。改 `../Templates/Cpp/Spark/Types.h.tpl`：`unsigned short → uint16_t`、`int → int32_t`；`../Model/Types.xml` 的枚举容器 `basetype='int' → 'int32_t'`（一处覆盖全部 60 个枚举）。
  - **64 位保留 `long long`（用户决策）**：Linux/GCC 上 `int64_t` 是 `long`，改成它会让全仓 `%lld`（Spark 68 处 + QT 78 处）变成格式不匹配；`long long` + `%lld` 在两个平台都对。模板尾部因此新增三个宽度断言（`long long` / `double` / `bool`）——只有这三个不是标准保证的宽度，固定宽度类型自身已精确。
  - **改生成物必须先证明生成器能复现现状**：动手前用**未改**的模板重跑 `pump.py`，输出与仓库现状逐字节一致（空 diff）；改完再 pump，diff 只含 1 个 `uint16_t` + 21 个 `int32_t` + 60 个枚举 `: int32_t` + 末尾断言块；`EnumString.h` 未受影响，`long long` / `double` / `char[]` 段原样未动。
  - **`ProtocolVersion.h` 补字段偏移断言**：`offsetof(HeadField, Magic / MsgSeqNum / PackageID / BodyLen / Version / MessageChain / Reserved)` 依次 0 / 4 / 8 / 10 / 12 / 14 / 15，`offsetof(TailField, CheckSum) == 0`——只钉总长 16 / 4 会漏掉"字段重排但总长不变"。
  - **收掉 3 处 `%u` 配 `UShortType`**（`PackageReader.cpp` 的 `m_Head.BodyLen`、`m_Head.Version` ×2）→ `static_cast<unsigned int>`：窄整型进可变参数会先升格成 `int`，`%u` 在标准上不成立（GCC `-Wformat` 会报），而 Linux 是发布主战场。Network 目录之外的 `%u` 处数为 0。
  - **零语义变更的证据**：`cmake --build out/build/x64-Debug` 零错误零告警；`UnitTests.exe` **366/366 通过（25 套件）**，含逐字节线上往返用例——断言全部成立即证明本平台 `int32_t == int`、`uint16_t == unsigned short`，故不需要递增 `ProtocolVersionValue`。
  - **下游**：QT 无自己的 `Types.h`（`#include <Spark/Types.h>`），生成代码只引用别名名，**不需要改源码**；待重装 Spark 经安装树自动生效。
  - **未做**：WSL-GCC preset 从未构建过（当前只有 x64-Debug），建议在它上面用 `-Wformat -Wconversion` 全量扫一遍；根治格式化符问题的是把 `WriteLog` 换 `std::format`，但属 logger 热路径，需单独一批并计量。
  - **风险标注（§7）**：本批是类型别名重定义，无新增分支、无锁、无内存管理改动；`Types.h` 是全部生成物的公共头，下游重装后需整体重建。
- **类型调色板补齐 8/16/32/64 位（2026-09-13，`Templates` + `Model` + Spark/QT 生成物与手写源码；关闭原 ❓「8 位整型是否补进类型模板」，见归档 `Q.15`）**：原 ❓ 判断"要补就得四条链一起"，本批四条链都做了——别名容器、各模板的族映射、Step 的反序列化范围校验、MDB/SQL/C# 的类型名。别名按用户统一后的风格：`UInt8/Int8/UInt16/Int16/UInt32/Int32/UInt64/Int64`。
  - **段名统一**：容器段名 `ushorts` → `uint16s`、`ints` → `int32s`，与新增的 `uint8s/int8s/int16s/uint32s/uint64s/int64s` 并列。段名是模板与模型之间的约定，25 个模板里硬编码了这两个段名，全部同步。
  - **标签（label）按"谁消费它"逐个选，不照抄一套词表**：C++ 协议族的 label 同时是 printf 格式选择器，用 `uint8/int8/int16/uint32/uint64` 配 `%hhu/%hhd/%hd/%u/%llu`；Mdb 族的 label 会被直接塞进 `std::hash<LABEL>()` 并与 `!!@type!!Type` 拼类型名，因此用定宽 typedef 名（`uint8_t`…）；`InitMdbFromCsv` 与 `TestCases` 的取值链**没有 `else`**，写了链上不存在的标签会一声不响地跳过该字段，故映射到既有分支标签（`int`，64 位用 `int64`）；`RiskIndex` 的标签是函数选择器，映射 `int`；SQL 族的 label 就是 DDL 列类型（MySQL `tinyint unsigned`/`smallint`/`int unsigned`/`bigint unsigned`、Sqlite `int`、DuckDB `utinyint`/`smallint`/`uinteger`/`ubigint`）；C# 族是 C# 类型名（`byte/sbyte/short/uint/ulong`）。
  - **Step 反序列化的静默截断被堵住**：`Packages.cpp.tpl` 的分支链此前只认 `ushort/int/int64/…`，新族会落进 `else: atoi(value.c_str())`——编译通过、小值正确、超范围静默截断。新增 `elif $type in ('uint8','int8','int16','uint32','uint64')` 分支，改走新加的 `StepUtility::ParseInteger`（`std::from_chars`，格式非法或越界都返回 false），失败时写 Warning 日志（含字段名与原始文本）后 `return false`。C# 侧 `StepPackages.cs.tpl` 的链路有 `else` 会赋字符串，故补了 5 个显式 `Convert.ToByte/ToSByte/ToInt16/ToUInt32/ToUInt64` 分支。
  - **`WriteString` 补 5 个重载**：不补时新族会落到末尾那个模板重载，它按 `%s` 打印整数，`sprintf(ppos, "%d=%s", key, value)` 会把整型当 `char*` 解引用——**必崩**路径。`UShortType`/`IntType` 别名随 `Types.xml` 改名而失效，10 个模板里硬编码的 `UShortType` 一并改为 `UInt16Type`（`UShortType` 是 `Templates` 里唯一残留的旧别名，已与 `Types.xml` 的别名表逐名核对）。
  - **MySQL 的一个错值顺手修正**：`uint16s` 的原标签 `short` 不是 MySQL 类型，改为 `smallint unsigned`。该段此前没有任何字段在用，故无线上 DDL 变化。
  - **先证明生成器能复现现状再动手**：改 `Packages.cpp.tpl` 前，用未改的模板在临时目录重跑一遍，输出与已提交的 `test/Packages/Packages.cpp` **逐字节一致**（39995 行）；新分支另用探针模型渲染一遍，核对缩进、括号与 `%hhu` 调试格式。改完 `pumpall.py` 在 Spark 与 QT 各跑一次均 exit 0，生成物 diff 过滤掉别名改名后为空。
  - **实证（x64-Debug）**：`cmake --build out/build/x64-Debug` 零错误；`bin/Debug/UnitTests.exe` **366/366 通过（25 套件）**。`Templates` 34 个文件 +793/−65 行；QT 15 个文件全部是别名改名（+388/−388，即纯改名）。
  - **未覆盖的点（诚实记录）**：新族当前**没有任何模型在用**，所以新的反序列化分支与 5 个 `WriteString` 重载在现有用例里**不被执行**——366 与加调色板之前是同一个数字，不能当作"新路径已生效"的证据。要真正跑到它，需要模型里出现一个 `UInt8`（或其它新别名）字段 + 一条逐字节往返用例。
  - **风险标注（§7）**：`InitMdbFromCsv` 把 8/16/32 位族映射到 `int` 分支，落到 `(!!@type!!Type)csv_record.GetFieldAsInt(...)` 上——第一次有表用这些窄别名时会触发 `int` → 窄类型的收窄转换（MSVC C4244，告警级，不阻断编译）。Step/Xtp 的**旧方言**模板（各自项目本地一份 `StepUtility`，可能没有 `ParseInteger`）故意不加范围校验，新标签在那里仍走原有的 `atoi` 路径。`WriteLog` 仍是 printf 风格，本次新增的 `%hhu/%hhd/%hd` 只对窄整型合法，GCC `-Wformat` 需在 WSL preset 上复验（沿袭下条未做项）。
- **类型 label 逐族统一（2026-09-13，`Templates` 提交 `15677be` + `bce6ad7`，接在"类型调色板补齐"之后）**：
  上一批解决"段名与位宽齐不齐"，这一批解决"同一族的 label 各自为政"。
  - **Mdb 定宽族 5 个文件**：`uint16s: 'short' → 'uint16_t'`、`int32s: 'int' → 'int32_t'`，与同族既有的
    `uint8_t`/`int8_t` 等对齐。`int64s` 的 `'int64'` 保留不动——它是**哨兵**而非类型名：`MdbIndexComp`
    拿 label 去实例化 `std::hash<>`，Linux 上 `int64_t` 是 `long`，会让全仓 `%lld` 变成格式不匹配。
  - **`InitMdbFromCsv` 是例外（它的 label 是取值函数选择器，不是类型名）**：`'short'` 与 `'int'` 两个分支
    的函数体一字不差，故 `uint16s` 并入 `'int'` 并删掉重复分支（§5 DRY）。
  - **协议族 5 个文件**（`Protocol/Packages`、`Protocol/Step`、`Protocol/Xtp`、`ApiTest/ApiMiddle`、
    `ApiTest/SpiMiddle`）：`uint16s: 'short'|'ushort' → 'uint16'`、`int32s: 'int' → 'int32'`，改为与同文件
    兄弟段一致的裸名（`uint8`/`int8`/`uint16`…）。
  - **`TestCases.cpp.tpl` 同形，并顺带堵漏**：`uint16s: 'short' → 'int'`、删掉 `LoadOrders` 里的重复分支；
    真正的收益在 `LoadOrderCancels`——它的链里**没有** `"short"` 分支、链尾也没有 `else`，所以声明在
    `uint16s` 段、又被 `OrderCancel` 引用的字段，此前在 JSON 加载时被**静默跳过**（无报错，字段保持未初始化）。
    改后两条链与 `InitMdbFromCsv` 完全同形。这同时**修正了上一条记录**：它当时已写明"`TestCases` 映射到
    既有分支标签（`int`）"，但模板里实际还是 `'short'`，本批才真正对齐。
  - **逐改逐证（不半改）**：三个脚本各自断言段位置唯一、块第二行是 `!!travel!!`、段内恰好一行
    `!!types[@name]`、且旧值与预期逐字相同；再用 `difflib` 的 hunk 级 guard 只放行"目标行一对一替换"与
    "整块删除"，出现任何其它改动立即整体中止，故不存在"改了一半"的中间态。每批改完 `pumpall.py` 在 Spark
    与 QT 各跑一次 exit 0、`git status` 全空，即生成物逐字节未变。
  - **影响面（实测，非推断）**：`TestCases.cpp.tpl` 的唯一消费方在 **Offer** 仓（`Source/pumplist.xml:53`），
    不在本轮三仓范围内。只**读**了它的模型、未写 Offer：`Model/Offer/Types.xml` 仍是旧调色板（只有
    `bools`/`ints`/`int64s`，连 `ints` 这个段名都不在 12 段约定里），`uint16` 计数 0，故对该仓同样零输出。
  - **未覆盖的点**：C# 族与 SQL 族的 `short`/`ushort` 是**真类型名**，故意不动（C++ 侧已无残留）；
    `formats` 的 `uint16s` 仍三花脸（`Packages` 为 `%hu`、`StepPackages` 为 `%d`、其余 5 个为 `%u`），
    实测三者对 uint16 的打印结果**完全相同**（变参提升为 `int`），属纯风格问题，本批未动。
  - **风险标注（§7）**：只改模板 label 与一条死分支，未触及多线程、锁与内存管理；`TestCases` 那条改动的
    方向是"此前静默跳过的字段开始被赋值"，属**行为变更**（修漏），已确认对现有全部消费方零输出。

## 🔄 进行中

- 无

## ❓ 待讨论 / 待决策

- **设计约束：宿主必须显式调用 `Stop()`+`Join()`，否则最后一次缓冲必丢（2026-09-12 定论，非待修缺陷）**：进程退出时日志线程先被终止，任何晚于此的析构（`~Logger()` / `~ThreadBase()`）都无法补救——这是上一条实证得出的时序结论，不是可以靠改析构语义绕过的。宿主若确实无法在 `return` 前收尾，可考虑自行注册更早的收尾点（`std::atexit` 回调在 `ExitProcess` 之前执行，理论上仍能完成一次真正的 `Stop()`+`Join()`；**未实测**）。
- **Step 头上的 `Reserved` 字段暂不上线（本轮决策，待复核）**：`Head.xml` 里 `HeadField` 有 7 个字段（含 `Reserved`），但 Step 的文本包头只序列化 6 个（`HeadItemCount = 6`），`Reserved` 仅 Xtp 分支有。理由：`Reserved` 的定义是"保留字段，必须为 false"，缺省即 false，上线只会让包头多 12 字节；如果希望两端能校验"对端没乱用保留位"，需要把它加进 `HeadToStream`/`HeadFromStream` 并把 `HeadItemCount` 改成 7。
- **P5 握手（协议版本协商）未实施**：计划里本就建议**不做**——版本号已经能在第一帧的固定偏移上校验出来，握手只会把"不一致"的发现推迟到连接建立之后，且要新增一对报文。当前实现按此执行，若日后要做，入口是 `Protocol::OnConnect`。
- ~~**生成器不认 `size`，包体越界写没有写前防护**~~ **已关闭（2026-09-14）**：用户选定"携带上界的游标对象"且 `ToXtpStream` 一并收口，两条路都走了——`ToStepStream` 改用 `StepWriteCursor`（每字段写前比容量、截断返回 `-1`），`ToXtpStream` 每次 `memcpy` 前加 `offset + fieldSize > size` 判断。写前防护已到位，`Package.cpp` 的长度校验退回"真正的兜底"。**原文保留如下（仅作已关闭条目的历史记录，待归档时整条搬走）**：`ToXtpStream`/`ToStepStream` 的模板实现直接 memcpy/写文本，然后 `return int(ppos - buff)`，既不比对 `size` 也不返回负数。本批在 `MakePackage` 补的判定是事后检查：能拦住非法帧发出、能拦住 `BodyLen` 被截断，但包体真超限时序列化器已经把字节写到了缓冲之外（`Buffer<SIZE>` 的 `char m_Buffer[SIZE]` 后面就是它自己的 `m_Length`/`m_ReadPos`，溢出的破坏面是对象自身成员）。要做到写前防护，只有两条路：模板侧每写一个字段前比对剩余容量（改 `../Templates`，影响全部生成物与生成时间），或给 `ToXtpStream` 传一个带容量语义的可写游标对象。**需要用户决定是否做**（已于 2026-09-14 决定：采用携带上界的游标对象，并一并收口 `ToXtpStream`）；不做的前提是"没有任何模型会产出接近 64 KB 的包"，这条假设当前成立但无自动化守卫。
- **文本协议反序列化对 `uint16`/`int32` 仍走 `else: atoi`（2026-09-13 记，待决定）**：
  `Templates/Cpp/Protocol/Packages/Packages.cpp.tpl` 的整数分支是
  `elif $type in ('uint8','int8','int16','uint32','uint64')` + `else: atoi(value.c_str())`，即 `uint16s`
  与 `int32s` 这两个段**掉进 `else`**，拿不到上一批新加的 `StepUtility::ParseInteger` 范围检查。补两个标签
  进去改的是文本协议反序列化路径，且是**行为变更**（从"小值正确、超范围静默截断 / `atoi` 溢出 UB"变成
  "拒绝并报错"），需用户点头。并存项：`atoi` 的溢出 UB 对**本就走在 `else` 里**的既有族是既有问题，
  非本批引入。同一处的 `formats` 对 `uint16s` 三花脸（`%hu`/`%d`/`%u`）属纯风格，可一并收口。

## 归档索引

- `Q.16` README 上的单元测试用例数是否继续标注（2026-09-14 关闭）
- `Q.15` 8 位整型是否补进类型模板（2026-09-13 关闭）
- `Q.14` Step 协议两端实现已分叉（2026-09-13 关闭）——见 `PROGRESS-archive.md`
