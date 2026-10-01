# STEP 文本字段的解析口径

本文记录 STEP 报文**包体字段**从文本到内存的解析口径：四族字段各自收什么、拒什么、拒了以后会怎样，
以及为什么收紧到这一层为止。相关的类型定义、生成链与测试落点一并写在这里。

本仓代码不写注释（Harness §4：命名即文档），`test/Packages/Packages.cpp` 更是生成物，注释写进去下次 `pumpall.py` 就没了，
故把「为什么」集中到本文。

---

## 一、落点：改模板，不改生成物

| 环节 | 位置 |
| --- | --- |
| 模板（**唯一可改处**） | `D:\Gitee\Templates\Cpp\Protocol\Packages\Packages.cpp.tpl`（独立 git 仓） |
| 生成器 | `pumpall.py` → `pump.py`（都在 Spark 仓根） |
| 模型 | `test/Model/Protocol/Packages.xml` + `../Model/Types.xml`（后者在兄弟仓 `D:\Gitee\Model`） |
| 产物 | `test/Packages/Packages.cpp`（文件头写明「请勿手改」） |

`pumplist.xml` 决定哪些产物由哪个模板生成；`NeedPump` 按 mtime 判断是否需要重生成，故改完模板必须跑一次
`python pumpall.py`（在 Spark 仓根运行），否则生成物还是旧的。

**两仓必须同批提交**：模板在 Templates 仓、产物在 Spark 仓，只提交一侧就只剩下一次「下次 pump 回退掉本批改动」的
定时炸弹。本批即分两笔提交，模板一笔、Spark 一笔。

复核办法（本批实测过）：`touch` 模板后重跑 `pumpall.py`，产物 `sha256` 不变，即工作区里的产物确实是模板的产物。

模板把字段按类别分流，类别来自 `types[@name]`：

```
!!entry bools!!   → types[@name] = 'bool'
!!entry doubles!! → types[@name] = 'double'
!!entry enums!!   → types[@name] = 'enum'
!!entry strings!! → types[@name] = 'string'
!!entry int32s!!  → types[@name] = 'int32'   （int8/16/64 与无符号同形）
```

## 二、四族口径

解析分支在模板 `Packages.cpp.tpl:217-262`（`FromStepStream` 的 `switch (itemId)` 内），一个字段项一段：

| 类别 | 现读侧 | 本批之前 | 收下的文本 |
| --- | --- | --- | --- |
| 整型（`uint8`…`int64`，294 处） | `ParseInteger`（`std::from_chars`，本批未动） | 同左 | 裸十进制（前导 `0` 允许，`+`、空格、尾随垃圾、越界一律拒） |
| 枚举（123 处） | `ParseEnum` | `static_cast<T>(atoi(value.c_str()))` | 整型范围内的一切文本 |
| 布尔（12 处） | `ParseBool` | `atoi(value.c_str())` | `0` 或 `1` |
| 双精度（187 处） | `ParseDouble` | `atof(value.c_str())` | 有限值的十进制/科学计数文本 |
| 字符串（584 处） | 容量校验后 `memcpy`（本批改） | `memcpy` 静默截断到容量 | 长度 < 目标数组容量的一切文本（`N-1` 个字符照收，第 `N` 个起拒） |
| `char`（0 处） | 未动 | 未动 | 取首字节 |

计数取自当前产物：`ParseInteger` 294 + `ParseEnum` 123 + `ParseBool` 12 + `ParseDouble` 187 + 字符串容量校验 584 = 1200，
与产物里 1200 处 `Out Of Range` 日志逐一对上（每族各有一句同形的拒收日志）。字符串族那句印的是
`Length:%zu, Capacity:%zu` 而非字段值——`Password` 之类的字段不许进日志，这条口径全族统一遵守。

三个助手的实现都在 `include/Spark/Network/Protocol/StepUtility.h`（`ParseEnum:131`、`ParseBool:144`、`ParseDouble:154`）。

**为什么枚举不能直接复用 `ParseInteger`**：`ParseInteger` 带
`static_assert(std::is_integral<T>::value, "ParseInteger 只接受整型")`，而枚举类型本身不是整型——本题所有枚举都是
`enum class X : int32_t`（`Types.h`），**底层类型**是 `int32_t`，类型本身不是。故 `ParseEnum` 只是把
`std::underlying_type_t<T>` 递给 `ParseInteger`、再 `static_cast` 回枚举，没有第二套解析逻辑。
布尔是 `bool`（整型），能过那条 `static_assert`，但它要的是「只认 0/1」而不是「照收任意整数」，故单独一道。

**为什么失败要拒整包，而不是跳过该字段**：字段对象在解析前被 `memset` 成 0，跳过等于把一个**看似合法的默认值**
（枚举落成 0、金额落成 0）静默交给业务；拒收是唯一可被观测的处置，且与整型分支的既有行为一致
（`WriteLog(LogLevel::Warning, "Xxx Out Of Range. Value:%s", ...)` + `return false`）。
日志里只有字段名与线上原文，不含内部路径、地址或表结构（Harness §6 脱敏）。

## 三、`nan` / `inf` 的裁定

写侧 `StepUtility.cpp:222` 是 `cursor.AppendField("{:04X}={:.6f}", key, value)`：`std::format` 对非有限值忽略精度，
`nan` / `inf` / `-inf` **原样写出**，不报错也不替换。旧读侧 `atof` 照单全收，非有限值就此静默流进金额、价格这类字段，
再顺着后续计算传染（NaN 参与的比较与算术全部为假/NaN）。

裁定：**读侧拒绝**（`!std::isfinite(parsed)` 即 `return false`）。理由是非有限值在业务上没有意义，
而拒收可观测、有日志。代价单独列在第七节。

## 四、枚举只校验整数范围（本批的已知界限）

`ParseEnum` 只要求「文本是整型且落在底层类型范围内」，**不校验它是不是具名值**。实测：

| 输入 | 结果 |
| --- | --- |
| `AccountType=1`（`AccountTypeType::Sub`） | 收下 |
| `AccountType=9`（不是 `AccountTypeType` 的任何具名值） | **也收下**（9 在 `int32_t` 范围内） |
| `AccountType=x` | 拒收（非整型文本） |

不做具名值域的原因有两条，都在模板这一侧：

1. 上界得按名去**另一个模型**（`Types.xml` 的枚举成员表）里查，模板语言是否支持跨模型按名取成员数尚未探过；
2. 即便查得到（本题所有枚举从 0 连续编号，成员数即等价上界），一旦以它为界，**对端加值就会变成加载失败**：
   CTP 侧新增一个业务枚举值、本端还没升级，报文就从「收下一个未知值」变成「整包拒收」。严格性在这里反噬兼容性。

故本批只把「非整型文本静默落成 0」这一条堵上，具名值域**定为协议口径、不再改**（2026-10-01，见第六节）。

## 五、写侧与读侧的对应

| 类别 | 写侧（`StepUtility.cpp` 的 `WriteString` 重载） |
| --- | --- |
| 枚举 | `{:d}`（模板里先 `static_cast<int>`） |
| 布尔 | `{:d}` |
| 双精度 | `{:.6f}` |
| 整型 | `{:d}` |
| 字符串 | `{:s}` |

判据是**自洽**：写侧的输出必须落在读侧能收的集合内，否则自家报文自己解析不了。测试里为此各留一条断言
（例如「枚举写侧必须写成裸十进制整数」）。

字符串族的自洽性正是第六节第三条新判据成立的前提：写侧在 `strlen(...) >= sizeof(...)` 时把目标数组第 `N-1`
个字节强制置 `0`（`Packages.cpp.tpl:167-170`），故它写出去的字符串**最长 `N-1` 个字符**，恒落在读侧
「长度 < `N`」的集合内——按旧口径能收下的帧，新口径一个都不会挡。这条对称性由 `step_e2e` 的 89 帧实测印证。

一处例外值得知道：写侧 `{:d}`／`{:.6f}` 对非有限值的输出（`nan` / `inf`）**不在**读侧能收的集合内——
自家写出 nan 就会自家解析失败。这不是缺陷，正是第三节裁定的结果；用例也正用这一点构造（把 `Asset` 置 nan 后
直接 `MakePackage`，报文里就会出现 `Asset=nan`，无需改写文本）。

## 六、未修项（登记）

1. **写侧 `{:.6f}` 的精度上限**：小数点后第 7 位起在写出时被舍入，读回的值与内存中的值不再逐位相等。这是写侧固有损失，本批未动。
2. **写侧不拒 `nan` / `inf`**：仍会原样写出（读侧已拒，故后果是自家报文解析失败而非静默传播）。
上面 ①② **已于 2026-10-01 定为协议口径、不再改**。

第三条（字符串分支静默截断）当时以「动它要先定『截断算不算错』」登记为未决，**同日裁定「算错」并已修**：
584 处改为容量校验后拒收，不再是未修项。改动落点、判据与理由见第二节表、第七节用例、第八节。

## 七、测试与实测

助手级（`test/unittest/Network/StepUtilityTest.cpp`）：

| 用例 | 拒收输入 | 收下输入 |
| --- | --- | --- |
| `ParseEnumAcceptsTheIntegerRangeOnly:1027` | `""`、`+1`、`" 1"`、`"1 "`、`1abc`、`1.0`、`-`、`--1`、`abc`、`0x1`、`2147483648`、`-2147483649` | `0`、`8`、`00123`、`-1`、`4`、`-2147483648`、`2147483647` |
| `ParseBoolAcceptsOnlyZeroAndOne:1046` | `""`、`-1`、`2`、`" 0"`、`"0 "`、`true`、`false`、`0x1`、`1abc`、`+1`、`2147483647` | `0`、`1`、`00`、`01` |
| `ParseDoubleRejectsMalformedAndNonFinite:1061` | `""`、`+1.5`、`" 1.5"`、`"1.5 "`、`1.5abc`、`.`、`-`、`1e`、`abc`、`nan`、`NaN`、`-nan`、`inf`、`-inf`、`1e999`、`-1e999`、`0x10` | `0`、`123.456789`、`-1.5`、`1e3`、`1E-3`、`0.000001`、`1.7976931348623157e+308` |

包级（`test/unittest/Network/PackageSerializationTest.cpp`，行号一律记**用例的 `TEST` 宏那一行**）：

| 用例 | 输入 | 输出 |
| --- | --- | --- |
| `ParseStepFrame:859` | —— | 「一帧解析一遍」助手；返回 `false` 时 `parsed` 必为 `nullptr` |
| `StepRoundTrip_EnumFieldRejectsNonIntegerText:900` | `RspQryCapitalPackage`（0x100A）的 `Capital->AccountType` 由 `0` 等宽改成 `x`（改后重算报尾） | `ParsePackage` 返 false、包为 nullptr；同一帧改成 `9` 仍解析成功（钉住第四节的界限） |
| `StepRoundTrip_NonFiniteDoubleFieldIsRejected:935` | `Capital->Asset` 分别置 `nan` / `+inf` / `-inf` 后 `MakePackage` | 三条都返 false；对照帧 `100.5` 正常解析。用例先断言帧里确有 `nan` / `inf` / `-inf` 文本 |
| `StepRoundTrip_BoolFieldRejectsNonZeroOne:966` | `NotifyComponentConnectStatusPackage` 的 `IsConnected` 由 `1` 改成 `2` | 返 false；对照帧为 true |
| `StepRoundTrip_StringFieldLongerThanCapacityIsRejected:989` | `RspQryCapitalPackage` 的 `Capital->AccountId`（`char[32]`）由 `Xunmeng001` 改成 31 个 `A`（容量内最长）与 32 个 `A`（超一个）。文本变宽会挤动后文，故同时改写包头 `BodyLen` 并重算报尾 | 31 个 `A`：解析成功、`AccountId` 长度 = 31；32 个 `A`：返 false、包为 nullptr |

四条包级用例都带一条**对照帧**断言（同一帧不改值先解析一次），以免「失败」其实来自别的原因。

助手契约：`ParseStepFrame` 一帧解析一遍，**返回 `false` 时 `parsed` 必为 `nullptr`**——与 reader
在解析失败时的约定一致（帧已被弹掉、包已归还池，见第八节），故用例只需断言这一个返回值。

**A/B 实测**：把生成物换回 `HEAD` 版（模板改动留着）、并把三个助手的判据临时去掉后重跑，上表七条**全部失败**——
四条包级用例在 `ParseStepFrame` 上实测返 `true`（旧代码收下了 `x` / `nan` / `inf` / `-inf` / `2`，以及 32 个 `A` 的
`AccountId`），助手级三条里 `ParseDouble` 实测收下 `1e999` 并得 `-inf`、收下 `0x10` 并得 `0`。恢复后七条全绿。

字符串那条 A/B 的具体形态（本轮实测）：换回 `HEAD` 生成物后，用例仍**只在最后两条断言上失败**——
31 个 `A` 的对照帧照旧解析成功、`AccountId` 长度 31（说明这条用例拒的是长度本身，不是"全拒"），
32 个 `A` 那帧则被旧代码**照收**（`ParseStepFrame` 返 `true`）。这就是该用例的判别力所在。

单测：本批 MSVC Debug **500**（上一批四档为 MSVC Debug 493 / Release 494、WSL GCC Debug 488 / Release 489，
本批 +1）。**Release 与 WSL GCC 两档按 2026-09-30 验收口径欠到整体改完时补跑**，本批未跑。

冒烟（改动触及 `TestServer` / `TestClient` 编译到的代码，故照纪律逐条单独跑）：Shm 3 轮（仅出现已知的
`Sem UnLock Failed.`，见下）、Tcp-Select 与 Tcp-Iocp 各 1 轮通过、`tools/step_e2e.py` 单独跑 40 秒通过
（服务端解析 89 帧 / 客户端 8 帧、字段全吻合、0 ERROR）。**`step_e2e` 这条同时是判据对称性的实测证据**：
它走的是 `Protocol::Send` 的真实成帧路径，写侧产出的每一个字符串字段（含定长的 10 字符 `AccountId`）都被
读侧照收——新判据没有把自家写侧的合法值挡住。

## 八、已知未覆盖与未决

- 枚举具名值域（第四节）：只堵了「非整型文本」，越界的具名值仍被收下——**2026-10-01 定为协议口径、不再改**，
  理由即第四节那两条（上界要跨模型按名取成员数；以成员数为界会让对端加值从「收下一个未知值」变成「整包拒收」，
  严格性反噬兼容性）。
- 写侧 `{:.6f}` 精度上限、写侧不拒 `nan` / `inf`（第六节）：**2026-10-01 定为协议口径、不再改**。前者：6 位小数
  就是本协议 double 字段的线格式精度，是规格而非缺陷——改它等于改线格式，须连同 `ProtocolVersionValue` 与对端
  一起升。后者：写侧原样写出 `nan` / `inf` 是第三节裁定的直接结果，且正是包级用例的构造手段（第五节末段）。
- 字符串超容量（第六节第三条）：**2026-10-01 裁定「算错」并已落定**——584 处由静默截断改为拒收。四条理由：
  (1) 读侧其余各族越界一律拒收，字符串是最后一处静默改值；(2) ①② 的「严格性反噬兼容性」搬不过来——被截断的
  字符串在 `char[N]` 里**根本表示不出来**，而未知枚举值是**可表示**的，读侧对表示不出来的整数本来就拒（第四节
  那两条只约束「可表示但未知」）；(3) 对称模型下这条分支不可达，走到即说明两侧模型已经分叉，正是要知道的时候；
  (4) 代价不对称——一条点名字段的 `Warning` 对一份下游看不见的错数据。**判据钉在容量上**：`N-1` 个字符
  （写侧能产出的最长值）照收，第 `N` 个起拒，故按旧口径收下的帧行为不变。
- `ParseDouble` 不认 16 进制浮点（`0x10` 被拒）——`std::from_chars` 的浮点重载本就不接受 `0x` 前缀，
  与 `strtod` 不同；线上格式由自家写侧产生，不含这种形式，故不补。
- 包头的 `BodyLen` 走 `ParseInteger(value, head->BodyLen, 16)`（base 16），与包体的十进制口径不同，本批未动。
- 拒收之后由谁重传不在本文范围。`PackageReader.cpp:237-245` 的次序是「先 `PopFront(整帧)`、再判 `FromStepStream`
  的返回值」：拒收时这一帧已从缓冲里弹掉、包对象也已归还池，只留一条 `FromStepStream Failed.` 的 `Warning`。
- **四类收窄已定为最终口径、不再放宽**（2026-10-01 定）：前导 `+`、前后空白、尾随垃圾、越界这四类，
  本批前是**静默取值**、之后是**整包拒绝**加一条 `… Out Of Range` 警告。收窄面仅此四类——
  `0` / `123` / `00123` / `-1` / `-0` 与各宽度边界值都照收（`from_chars` / `strtol` 同族、前导零照收），
  故不存在「对端发补位被拒」的问题。**无对端口径待确认**：仓内没有任何写侧能产出这四类
  （写侧是 `{:04X}={:d}`，纯十进制、无正号），而唯一记过的对端 `SimExchange` 已废弃
  （`PROGRESS-archive.md` 的 `Q.14`）。此条自「未决」转为「既定」；唯一已知的不对称是写侧 `{:.6f}`
  仍会写出 `nan` / `inf`（读侧已拒，见第三节）。
