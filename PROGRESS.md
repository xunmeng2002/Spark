# 项目进度

> 本文件用于跨会话状态记录，新会话开始请先阅读。

## ✅ 已完成

- **Spark 批 3 子集 · IO 缩写规范化：`IOxxx` → `Ioxxx`（类名/文件名/目录名同批原子改，2026-09-16，`0c95abb`，合并 `9e12009`）**：
  **为什么三类必须同批**：§1「缩写按普通单词处理，只首字母大写」的原文示例就是 `IoBase`，§2 又要求「文件名与类名一致」——只改类名或只改文件名，都是从违反一条规范变成违反另一条。故类名、文件名、目录名只能一个 commit 原子改。
  - **范围（44 文件，166 增 / 166 删，完全对称 = 纯改名特征，与批 1 的 854/854 同型）**：符号 `IOBase`/`IOFactory`/`IOThread`/`IOCompletePort`/`IOSubscriber`/`HandleIOEvent`/`GetIO`/`GetIOThread`/`ClientIOSubscriberImpl`/`ServerIOSubscriberImpl` + 4 处局部变量；目录 `include/Spark/Network/IO`→`Io`、`src/Network/IO`→`Io`；文件 12 个（含 `test/TestClient`、`test/TestServer` 各 2）；7 处「自指类名」日志字面量；中英文 README 代码示例。`IOUtility` 不是符号只是文件名——6 处命中全是 `#include`。
  - **两处我此前的测量是错的，已更正**：①`HandleIOEvent` 在原始清单里根本没有——用「以 `IO` 开头」的前缀式 grep，**结构上不可能匹配到它**（它不以 `IO` 开头），漏了 8 处，含 `IoBase.h:40` 的公开纯虚（被 `ShmBase`/`SingleShm`/`TcpBase` override）。②`GetIOThread`/`GetIO` 同理必须各自单列规则——`\bIOThread\b` 匹配不到 `GetIOThread` 里那一段。**与 `PROGRESS.md:17 ⑧`「扫描口径写窄 = 假绿灯」同型**：判据本身失明时，「0 命中」毫无意义。
  - **批量替换的三道自保（承批 1 那 20 处字面量误改的教训）**：①**状态机屏蔽**——按区间标出字符串/字符/注释，只在 `code` 区间求替换位置再按原偏移回写，`#include` 行是**唯一**白名单出口；②**等长不变式**——所有替换只翻一个字母大小写，断言 `len(new) == len(old)`，把偏移漂移这一整类 bug 变成不可能；③**15 例自测语料先行**（`IOBaseXXX`/`MyIOBase` 边界、`"IOBase"`/`'I'`/`// IOBase` 屏蔽、转义引号、`AppPlatformType::IOS`、`TimeConditionType::IOC`、`1'000'000` 数字分隔符），语料全绿才上真实文件。
  - **假阳性清单（全部实测原样保留）**：`FIONBIO` 6、`ERROR_IO_PENDING` 5、`ERROR_OPERATION_ABORTED` 1、`SIO_GET_EXTENSION_FUNCTION_POINTER` 4、`CreateIoCompletionPort` 2，以及 `AppPlatformType::IOS`（Apple 平台）与 `TimeConditionType::IOC`（即时成交或撤销）各 2。**后两条是「绝不允许写裸 `IO` 前缀规则」的死证**——前缀规则会把它们改成 `IoS`/`IoC`。
  - **刻意保留（附理由）**：①`IoFactory.cpp:45` 的日志**字段标签** `IOType`/`IOModel` 未改（规则管标识符，代码里从不存在叫 `IOType` 的标识符）；但规范拼写确由上游 `Model/Types.xml:222,227` 定为 `IoType`/`IoModel`，且同行 `ServerType` 恰是「类型名去尾 `Type`」的写法，故它是这行里唯一拼法落后于类型名的标签——**登记待决**。②中文散文里的 `IO`（`Types.h` 的 `//IO模型`/`//IO类型`、`TcpBase` 的「IO 循环」、`Protocol.cpp` 的 `static_assert` 消息、README 的「IO 线程」）保留，并作为闸门 3 的「应保留」正样本核验。③**`types.h`/`EnumString.h` 里那些 `IO` 不是手写文本而是生成物**（源在 `Model/Types.xml`，首行自述「请勿手改」），在 Spark 单仓改会被下次 `pumpall.py` 静默回滚——保留是唯一正确选择，不只是风格判断。
  - **六道闸门（全绿）**：①旧名残留 0，假阳性反向核验全部原样；②字符串字面量多重集比对 **13 删 / 13 增**且逐条可枚举（6 条 include basename + 6 条日志 + 1 条 `"Create IoCompletePort Failed."`），`"IOS"`/`"IOC"`/`"FIONBIO"` 无混入；③陈旧注释 0，中文正样本保留；④`pumpall.py` rc=0 且 **232 个跟踪文件逐字节零 churn**、无 `.pumptmp` 残留；⑤MSVC `x64-Debug`/`x64-Release` 0 error、`UnitTests` **392/392 ×2**；⑥`TestServer` + `TestClient` 端到端冒烟 **Tcp 与 Shm 两条路径均通过**（覆盖 `HandleIoEvent` 纯虚在 `ShmBase`/`SingleShm` 与 `TcpBase` 的实现）。
  - **本批最强的一条证据（新增闸门 1d）**：把 `git show HEAD:<旧路径>` 原文**按改名规则机械变换后**与工作区逐字节比对——**44 文件全等**。这比看 diff 强：它直接证明「除改名外无任何其他改动混入」。独立审查 agent 用自己的映射独立复现了同一结论，并额外做子串级普查（确认 `SERIALIZATION`/`VERSION` 这类**词内** `…ION…` 未被误伤成 `SERIALIZATIoN`）。
  - **另加的一道硬校验（Linux）**：`WSL-GCC-Debug` 构建 rc=0 / 0 error，`UnitTests` **391/391**（差 1 个平台专属用例）。对「只改大小写」这类改动，**大小写敏感的 Linux 构建才是硬校验**——Windows 上 `#include <Spark/Network/Io/IoBase.h>` 与残留的 `IO/IOBase.h` 会互相匹配，可能静默编到旧头从而绕掉闸门 1。（用户已定：仅 Windows 需通过，Linux 不投入。）
  - **顺带修掉一处既有构建缺陷（`e523f91`，独立于本批）**：`CMakeLists.txt:5` 用 `"$ENV{VCPKG_ROOT}/scripts/..."` 直接拼接工具链路径，而本机 `VCPKG_ROOT` 在注册表里是反斜杠形式（`D:\Github\vcpkg`），CMake 把它原样写进 `CMakeSystem.cmake` 的 `include()`，`\G` 被当非法转义 → **任何一次全新 configure（`cmake --fresh`、或他人 clone 后首次构建）都失败**。此隐患此前被既有缓存掩盖，是 `--fresh`（计划里 `rm -rf out/build` 的合规替代）把它逼出来的。已用 `file(TO_CMAKE_PATH)` 规范化，两个 preset 均 rc=0。
  - **清理的会话副产物（均非仓库文件）**：`parselist.xml`（0 字节，`touch` 时误建，仓库本无此文件）、冒烟产生的 `log/*.log` 与 Shm 落盘文件 `TestShm`（8 MB）。**顺带发现两处 `.gitignore` 缺口**：`log/` 与 `TestShm` 都未被忽略，一次 `git add -A` 会把日志与 8 MB 文件一并提交进去——**登记待决，本批未动**。
  - **仓外认知更正（此前记载有误）**：**LibTest 与 DAG 不是 Spark 的消费方**——LibTest 引的是自带的 `"PersonalLib/Network/IO/IOBase.h"` 且调用 `IOFactory::CreateIO`（该名在 Spark 的 `HEAD` 早已不存在），DAG 的 `DAGDemo/PersonalLib/` 是自带 `.lib` 的冻结 vendored 快照且在 DAG 仓里未被 git 跟踪。**两仓本批零动作**。**唯一活跃消费方是 QuantTrading**（16 处 / 10 文件，走 `find_package(Spark PATHS "../Libs/Spark/x64-windows")`），且它**当前已经编译不过**（调 `Protocol::SetIOThread`，该名只存在于 Spark 的 `HEAD`）——与本批无关，须先建基线才能归因。
  - **踩到并记录的 Git 索引陷阱**：`git mv` 会**立即把重命名写进索引**。我为把 CMake 修复拆成独立 commit 而做了 `git reset`，索引退回旧路径——在大小写不敏感文件系统上 **git 再也看不见重命名**，它把磁盘上的 `Io/IoBase.h` 当成索引里 `IO/IOBase.h` 的「同名文件」判为修改（`git status` 打印的是**索引里的旧大小写**，与磁盘不符）。此刻 `git add -A` 会把内容写进旧路径条目，**大小写改名静默丢失**；恢复办法是按原样重跑两步 `git mv`。**独立审查又抓到同源的第二个疏漏**：提交前索引只装了 14 条重命名、42 个文件的内容改动一条未入，裸 `git commit` 会产出「新路径装旧内容 + include 指向不存在目录」的编译不过的树——已按审查意见 `git add -A`，并在提交后复验索引旧名归零。

- **六仓 C++ 规范对齐 · 批 2b：Templates 残留 C 风格 cast 清零 + 生成失败路径加固（2026-09-15）**：
  批 2 至此只剩 `namespace Spark`/`mdb` → PascalCase（公开 API，须授权、与批 7 原子改）。
  - **① C 风格 cast 清零：17 处 / 11 文件，分两轮**（`%TEMP%\tpl-ccast-fix.py` 10 条 + `%TEMP%\tpl-ptrcat-fix.py` 3 条，逐条带命中数断言、幂等可重入）。
    第一轮 **11 处 / 8 文件**：两类形态，C 关键字类型（`(int)`/`(bool)`/`(void*)`/`(const char*)`）与模板变量类型（`(!!@type!!Type)`）；`enum` 分支沿用 `Packages.cpp.tpl` 已用过的条件三元组写法，不另创。
    **第二轮补 6 处 / 3 文件（`(T*)ptr` 指针形态，是独立审查抓到的漏网，见 ⑧）**：`Cpp/Api/ApiImpl.cpp.tpl:48,56`、`Cpp/Api/GbkApiImpl.cpp.tpl:60,81`、`Cpp/Protocol/Kernel/KernelGen.cpp.tpl:48,58`，一律 `((!!$packageName!!*)package)` → `static_cast<!!$packageName!!*>(package)`。**选 `static_cast` 而非 `reinterpret_cast` 已核实**：`Packages.h.tpl:16` 为 `class !!$className!! : public Package`（单一、非虚、公有继承），模板上下文里 `package` 声明为 `Package* package`，故下行转换合法且与 C 风格等价。本轮 churn：**只 QT 变**（Spark/Mdb 重 pump 无输出——这 3 个模板不喂它们），产物里 `static_cast<XxxPackage*>(package)` **107 处 / 8 文件**，旧形态在该 8 文件归零。
  - **② 举证是受控对照，不是看 diff**（`%TEMP%\ccast-verify5.py`）：10 条编辑整体回退 → pump → 基线，正向应用 → pump → 现状。**168 个产物变更 22 个**（Mdb 5 / QT 17 / Spark 0——Spark 0 可解释：这 8 个模板没有一个喂 Spark）。三重判据：变更集合符合预期、**每文件行数不变**、**两侧 cast 语法都摘掉后剩余逐字节相等** → **越界 0 处**；往返一轮 SHA-256 回同值、`.pumptmp` 残留 0。
  - **③ `pump.py` 加固（承 ❓ 区同名条目的方向 (i)；三仓各一份、md5 一致）**：usage 分支 `sys.exit(2)`；生成失败**不删目标**、改留 `<目标>.pumptmp` 并 `sys.exit(1)`；注释符由「`.sql` 否则 `// `」改成**扩展名白名单**，未知扩展名在写任何东西之前响亮失败；`pumpall.py` 的 `DoPump` 裸 `exit()` → `sys.exit(1)` + 中止信息。端到端五例（`pump-e2e2.py`）：A 正常 rc=0；B 模板坏 / C 未知扩展名（**在写之前**就退出）/ D 参数不足（**旧版会打印 usage 后继续覆盖目标并 rc=0**）三条 rc≠0 且**目标原样**；E `os.replace` 被挡 → 退回就地重写、rc=0 且内容更新；并证**字节中性**（加固后强制全量重泵：168 个产物内容变化 0、缺失 0、残留 0）。
  - **④ 其中一处是我引入的回归，由 `code-reviewer` 抓到**：`os.replace` 在 Windows 上当目标被别的句柄打开会抛 `WinError 5`，而它替代的旧写法 `open(dest,'w')` 只需写权限——于是「能就地重写」的场合变成**未捕获异常**，绕过我自己写的 `pump failed:` 并把 `pumpall.py` 停在半路。已加 try/except + 二进制就地重写回退。
  - **⑤ 三仓重建重测全绿**：六配置 MSVC rc=0、0 错误；Spark `UnitTests` **392/392** ×2；Mdb `TestDb.exe` rc=0 ×2（Sqlite+Duckdb）；QT `UnitTests` **101/690** ×2；QT 5 个集成 exe **10/10 rc=0**。**⑥ `.gitignore` 对齐（关闭 `:18` 登记）**：`__pycache__/` 补进 Spark 与 QT、Mdb 的 `/__pycache__` 归一为 `__pycache__/`（覆盖子目录）、新增本批引入的 `*.pumptmp`；三仓 `git check-ignore -v` 逐条核过。
  - **⑦ 本批我犯的两个错（留证）**：①首轮 C-cast 扫描**只认 C 关键字类型**，把「自定义类型」这一族整个漏了（审查在 `Mdb/InitMdbFromCsv.cpp.tpl:173` 抓到，且已落进两仓产物）——与此前批评的「没有全仓重扫」**是同一类错误**；②给 Mdb/QT 复制 `pump.py` 时**行尾写成了 CRLF**（HEAD 是 LF），而 **git 看不见**——全局 gitattributes 的 `*.py text eol=lf` 让 `git hash-object` 比的是**规范化后**的字节；靠逐仓比**工作区** md5 才发现。**跨仓复制文件不能用 `git hash-object` 判等。**
  - **⑧ 本批第三个错，也是最重要的一条：扫描口径写窄 = 假绿灯。** 第一轮收尾时我用 `%TEMP%\tpl-ccast-rescan.py` 复扫，报「真实 C 风格 cast **0 处**」，我据此把 ③ 记为**已关闭**并写进本文档。**独立审查用不同口径扫出 6 处真实残留**（上条第二轮）。根因不是漏看，是**判据本身失明**：`CAST_OLD` 长了一张**类型白名单**（`(const )?char * | void * | unsigned int | int | bool | <X>Type`），而漏掉的那一族类型名是**占位符** `!!$packageName!!`，**结构上不可能被白名单匹配**——白名单对「生成式类型名」天然失明，而本仓 96,612 行产物**全是生成式类型名**。改法：`%TEMP%\tpl-ccast-full.py` 改为**故意过收**——不预判哪些像类型，凡「括号紧跟操作数起始」一律列出、由人判读（93 个 `.tpl`、候选 158 处、`cast_like` 24 处，逐条判读后真 cast 恰为 6，其余是形参声明 / 函数指针 typedef / `offsetof` 限定名 / C# 越界）。**纪律**：本仓的扫描器一律「过收 + 人工判读」，不得再用类型白名单结案；「0 命中」在报告之前必须先证明**判据本身能命中正样本**（我没做这一步，是这次翻车的直接原因）。
  - **⑨ 顺带扫出、但不属本批的 4 处 `(T*)ptr`**：`QuantTrading/src/BackTest/SimExchange.cpp:287,290,293,296`，**手写**文件（非 pump 产物）、**HEAD 即已存在**（逐行比对确认非本批引入）→ 归**批 6**，已登记 ❓ 区未动。
  - **登记（本批未动，见 ❓ 区）**：生成物 `*TableList.h` 的 5 头 / 10 个公开常量须授权；工具链三处既有问题（`os.system` 拼串 / 头注释嵌模板路径 / 产物无体检）；**新登记 6 条**（`.cu` 白名单缺口、Mdb README 的 `tradingDay` 未定义示例、pump.py 回退路径四处、README 注释列位）见 ❓ 区同名条目。

- **六仓 C++ 规范对齐 · 批 2a 三项收口：模板有界化 / `using namespace std;` 清零 / 生成物「勿手改」头（2026-09-15）**：
  用户定的三项一次做完再动 Spark（「合并成一轮做，churn 只翻一次」）。**批 2a 至此收口**，批 2 只剩 `namespace Spark`/`mdb` → PascalCase 那一段（公开 API，须与批 7 原子改）。
  - **① `sprintf` 有界化（安全类，本批价值最高的一项）**：最后一处无界写是 `Templates/Cpp/Protocol/Packages/Packages.cpp.tpl` 的 `GetDebugString`——写目标是 `thread_local char DataStringBuffer[10240]`，`offset` 跨全包字段累加、**没有任何上界**。新增文件内 helper `AppendDebugString(int offset, const char* format, ...)`：用 `vsnprintf` 按剩余容量写入并返回新 offset，越界只截断不越写；**264 处**调用点由 `offset += sprintf(...)` 改为 `offset = AppendDebugString(offset, ...)`。同时补 `#include <cstdarg>` / `<cstdio>`。**§5 取舍**：一个 helper 对 264 处调用点，而不是逐点加护栏（`offset` 是跨字段累加的，逐点判断等于把同一段判断抄 264 遍）。至此 `Templates/` 内 `sprintf` **0 处**。
  - **② `using namespace std;` 清零**：5 处全清（`BackTestApiImpl.cpp.tpl`、`Config.cpp.tpl`、`ServerConfig.cpp.tpl`、`InitMdbFromCsv.cpp.tpl`、`InitMdbFromDb.cpp.tpl`）。随之把因去 `using` 而失去限定的 `printf`/`fstream`/`vector` 改为显式限定，或补 `#include <cstdio>`（`Config.cpp.tpl`、`ServerConfig.cpp.tpl`）。
  - **③ 生成物「勿手改」头：落在 `pump.py`（三仓各一份），不是改 66 个模板**。`sys.argv[2]` 就是真实模板路径，按目标扩展名选注释符（`.sql` → `-- `，其余 `// `），产物首行写 `// 本文件由 <模板路径> 生成；请勿手改，改动请改模板后重跑 pumpall.py`。落盘编码沿用 `UTF-8-SIG`。**这条是堵历史隐患的**——本仓 96,612 行生成物此前没有任何「勿手改」标记。
  - **③ 的证明（`pump-header-proof2.py`，两个 168/168）**：①把每个产物的首行去掉后，与「用删掉该块的 `pump.py` 重新生成的无头版本」比对——**168/168 逐字节相同**（即多出来的确实只有那一行）；②恢复后再次强制重 pump——**168/168 逐字节相同**（幂等）。
  - **churn 逐条核对（不接受「反正是 pump 出来的」）**：首次全量重 pump 后 168 个产物全变（多一行头）。以「头之外是否还有差异」为 oracle：恰好 **20 个**文件有实质变化，且逐一对应到本批改过的模板（`MdbStructs.h/.cpp` ×4、`InitMdbFromCsv/Db` ×4、`Config.cpp` ×9、`BackTestApiImpl.cpp`、`Packages.cpp` ×2）。收尾再 pump 一轮只变 **2 个**（QT 与 Spark 的 `Packages.cpp`）——正是 `Packages.cpp.tpl` 单独的爆炸半径，**反证** `RiskIndex`/`ServerConfig` 的改动只落在 SAMS（不在 pump 范围内）。
  - **三仓重建重测全绿**：Spark Debug rc=0 / `UnitTests` **392/392** / Release rc=0 / WSL GCC **391/391**；Mdb Debug+Release rc=0、`TestDb.exe` 的 Sqlite + Duckdb 两段通过；QuantTrading Debug+Release rc=0、`UnitTests` **101 用例 / 690 断言全过**、5 个集成 exe **RC=0** 且线程干净退出。**QT 这次能构建，顺带关闭了一条旧登记**：`Libs/DbAdapters/x64-windows` 已由 2026-08-20 的旧副本刷新为 **2026-09-14 09:41**（6321 字节，含 `422e92a` 的 FieldType 扩展），`MdbStatic` 不再失败。
  - **字符串字面量多重集比对**（承批 1 教训的必跑闸门，本轮覆盖 212 个改动文本文件）：**只有 3 个文件有差异，且全部是「只删不加」**——`MdbStructs.cpp` 三份里 `GetSqlString` 的 SQL 格式串（用户授权的删除，跨仓后果见 ❓ 区）。
  - **独立审查（`code-reviewer`）严重 4 / 高 4 的处置**：严重 3（新增的 `AppendDebugString` 可能未使用）→ 加 `[[maybe_unused]]`（C++20 已确认，`Spark/CMakeLists.txt:15-16`），WSL GCC 构建复验通过；严重 1（`GetSqlString` 删除）→ 用户已明确授权「删掉该函数」，保留；严重 2（本文档把批 2a 记作「非公开 API 部分」）→ **本条目即更正**，公开成员这轮一并改了；严重 4（批 2a 不是纯改名、无法原子拆分）→ 属实，用户已定「合并成一轮」。高 2 / 高 4 → `ServerConfig` 的 `instance_` → `instance`、`in_file` → `inputFile`；另 `RiskIndex.h.tpl` 的 9 处公开成员 `m_*` → camelCase。
  - **两处我自行判断的改动（附回退面）**：①`RiskIndex.h.tpl` 公开成员改名，援引的是**同仓既有裁决**「公开成员一律 camelCase 去前缀」（该裁决由用户答；批 2a 的授权原文亦为「连公开成员一起改名」）。回退面：模板 9 处 + SAMS 侧消费点，无逻辑变更。②`Mdb/test/TestMdb/TestDb.cpp:274` 保留 `//TestMysql();` —— HEAD 的第 275 行 `//TestMariadb();` 本来就是注释，属该文件既有约定；且 Mysql 段需要 33060 的活服务，本机没有。回退面：1 行。
  - **登记（本批未动）**：`m_Protocol` **8 处**保留——它只声明在手写的 `QuantTrading/src/Apis/ApiBase.h`，不在任何模板里，属批 6；`TestDb.cpp` 的 `t_tradingDay`/`t_exchange`/`t_account` 等局部蛇形名属批 5；`LibTest` 与 `SAMS` 消费了本批改过的模板但**未重 pump**（一个已废弃、一个不在范围），存在语义漂移、**无编译错误**；`__pycache__/` 在 Spark/Mdb/QT 三仓未被忽略（`pumpall.py` 的运行副产物），宜补进 `.gitignore`。
  - **⚠️ 证明阶段 `Mdb/src/Mdb/MdbTableRegistry.h` 被删过一次（741 字节，git 状态 ` D`）**：根因是 `pump.py` 生成失败时**先 `os.remove(out_file_name)` 删目标**，而 `pumpall.py` 的 `DoPump` 用裸 `exit()`（**返回 0**）把失败静默吞掉。已用 `%TEMP%\pump-guard.py` 恢复；随后连续 4 轮全量 pump **未复现**。**这是 `pumpall.py` 的既有缺陷，三仓都有，已登记待决（见 ❓ 区）。**
  - **本文档的滚动**：本条目入区后 ✅ 达 6 批，按 §8.1 移最旧三条入归档（`D.19`/`D.18`/`D.17`）。搬运对条目边界、脚本从 `git HEAD` 取原文、只移动不删改；半关闭条目（`D.18`/`D.19`）的未决 bullet 抽出后留在 ❓ 区，不随历史一起埋掉。

## 🔄 进行中

- **六仓 C++ 规范对齐**：批 1（Beacon）**已完成并提交**（`bd95bd4`，条目已入归档 `D.20`）；批 2 前置（Mdb 生成物补齐）**已提交**（`0bb5fa6`，条目已入归档 `D.21`）；**批 2a + 批 2b（Templates：有界化 / 公开成员改名 / C 风格 cast 清零 / 生成失败路径加固）已完成**。**批 3 已落地一个子集**：IO 族 `IOxxx` → `Ioxxx`（类名/文件名/目录名同批原子改）**已完成并合并**（`0c95abb` + `9e12009`），见上方 ✅ 条目——该子集能先落地，是因为它**不触碰任何跨仓公开契约**（符号只在 Spark 内部 + 两个测试程序）。**批 3 剩余部分仍阻塞于授权**：`namespace Spark` 112 处、小写访问器与方法约 1,295 处、C 风格 cast 134 处、`k` 前缀 221 处、`g_` 66 处、`enum CSV_PARSER_ERROR` → `enum class`，见 ❓ 区②③。**批 2 剩余部分**仍是 `namespace Spark`/`mdb` → PascalCase——它生成的是被 Mdb / DbAdapters / QT 通过 `find_package` 消费的公开头，**须授权并与批 7 原子改**（见 ❓ 区①）。批 4–7 **未开工**，阻塞见 ❓ 区。**注意**：`Cpp/Mdb/*.tpl` 被 Mdb 与 QT 共用，落地即产生三仓生成物 churn，**须分仓、分批 commit**。

## ❓ 待讨论 / 待决策

- **登记待批（承自归档 `D.19`「STEP 协议数字化收口」，原文照录；均超出该批范围）**：
  - **登记待批（均超出本批范围）**：①C# 对端 `SharpLibrary/Network/StepProtocol/`（8 文件）同步；②`DAG/DAGDemo/PersonalLib/include/Protocol/StepUtility.h` 声明副本同步；③**`Package::Prepare(SessionIdType, int messageChain, int msgSeqNum)` 的 `msgSeqNum` 形参仍是有符号 `int`**——改它要动公开签名，且 QT 约 50 个调用点传的是 CTp API 的 `int requestId`；负值会静默回绕成 2^32 附近的大值（今天则存成负数），两种都原样保留调用方的位模式；④`docs/wire-protocol-revision-plan.md` 补 v3 变更记录；⑤`TailToStream` 现在与 `HeadToStream` 形状相同（同样有定长宽度校验），可以同样迁移到 `StepWriteCursor`——本批按批准的计划保留了 `snprintf`。
      - **登记待办（本批不动）**：审查提议若日后希望生产与测试**共享**版本文本，可在 `ProtocolVersion.h` 比照 `ProtocolMagicText` 增设 `constexpr std::string_view ProtocolVersionText`；当前测试本地用 `const std::string` 派生是合理替代（`std::format` 非 `constexpr`），不做也不算问题。

- **未收口 / 登记待批（承自归档 `D.18`「协议写路径收尾」，原文照录）**：**其中 ①（`GetDebugString` 无界 `sprintf`）已于 2026-09-15 批 2a 关闭、⑤（`Libs/DbAdapters` 旧副本）已随安装副本刷新而关闭、③（`Templates` 残留 C 风格 cast）已于 2026-09-15 批 2b 关闭**，下列原文保留以存历史，动手前请以本行为准：
  - **未收口 / 登记待批（均超出本批范围）**：①`GetDebugString` 的无界 `sprintf`（模板 `:311`，生成物 264 / 71 处）——写入目标是 `thread_local char t_DataStringBuffer[10240]`，`offset` 跨全包字段累加、**没有任何上界**，与上一批刚修掉的 `ToStepStream` 越界写同等级；**本批不修**是因为它的输出形态是 `Name:field:[value], …` 而非 STEP 的 `key=value\x01`，`StepWriteCursor` 装不进去，需要一个独立的「返回 `const char*` 的有界格式化」方案。②`FromXtpStream` 的读边界（模板 `:293` 的 `memcpy` 在 `while (offset < endIndex)` 只保证 1 字节时可过读，目前靠读完后 `return offset == endIndex;` 兜）——加边界判断会改变行为（原来过读、现在返回 false），属另一批。③**（已于 2026-09-15 批 2b 关闭）`Templates` 残留 C 风格 cast**：原文记作「**4 文件 5 处**」并断言 `ApiTest/ApiMiddle.cpp.tpl:155`、`ApiTest/SpiMiddle.cpp.tpl:132,152`、`Mdb/MdbStructs.cpp.tpl:168,176,183`、`KernelGen.cpp.tpl:48,58`「**现在都是 0 处**」——**这段记载本身是错的**。真实残留是 **11 处 / 8 文件**（原记载漏了 `Error/Error.cpp.tpl:12` 的 `(const char*)u8"…"`；`ApiMiddle`/`SpiMiddle`/`MdbStructs.cpp.tpl:168,176` 并非 0 处；只有 `MdbStructs.cpp.tpl:183` 与 `KernelGen.cpp.tpl:48,58` 是真的已消失）。现已全仓重扫 **0 处 / 0 文件**（`%TEMP%\tpl-ccast-rescan.py`，排除 `sizeof(double)`/`sizeof(bool)` 两处假阳性），消费方 Mdb / QuantTrading 已随重 pump 跟进。**错因**：当时是拿「计划里写过的那几处」逐条核对，**没做全仓重扫**——与批 1 那次「grep 模式写窄了」同一类错误。详见 ✅ 区批 2b 条目。④仓内手写代码 **105 处 / 24 文件**（`StepUtilityTest` 28、`PackageReaderTest` 17、`PackageSerializationTest` 11、`MD5Test` 8、`UtilityTest` 7、`TimeUtility` 5、`SingleShm` 4，其余各 1~2），另开一批。⑤`Libs/DbAdapters/x64-windows` 是 2026-08-20 旧副本、缺 `422e92a` 的 FieldType 扩展，导致 QuantTrading 全量构建在 `MdbStatic` 失败（见上）。
  - **§4 注释例外（已在代码后向用户说明的两处）**：`Package.cpp` 那 2 行（解释闸门为何必须排在指针与容量运算之前——`ToXtpStream` 是公开纯虚函数、仓外实现不保证先比容量再写，属外部契约的坑）；`StepUtilityTest.cpp` 两个 helper 上方 2 行（说明缓冲容量取 64 的理由，以及「容量边界与截断语义另有专门用例、不走这里」——防后来者误用 helper 去测边界）。

- **`GetSqlString` 删除的跨仓后果（2026-09-15，需决策）**：
  用户在批 2a 明确授权「删掉该函数」（`Templates/Cpp/Mdb/MdbStructs.cpp.tpl`）。对 `D:/Gitee/Mdb` 与 QuantTrading 是**闭包**的——两仓生成物已随模板重 pump 并各自编译通过。但仓外还有三处**不受本模板管辖**的副本（已实测）：
  - `DbAdapters/test/TestDb/MdbStructs.{h,cpp}` 与 `SAMS/Source/{HistoryDb,SyncDb}/Mdb/MdbStructs.{h,cpp}` 是**各自独立、已分叉的自带实现副本**（各自的 `.cpp` 里就有 `GetSqlString` 的定义），故**没有编译影响**；
  - `SAMS/Source/{HistoryDb,SyncDb}/{MysqlDb,SqliteDb}.cpp` 有约 **296 处**调用点（`n += (*it)->GetSqlString(m_SqlBuff + n);`）；
  - **`SAMS/Source/pumplist.xml:55` 指向一个不存在的 `../Templates/Cpp/Db/Mdb/MdbStructs.h.tpl`**（真实路径是 `Templates/Cpp/Mdb/...`，没有 `Db/` 这一层），所以 SAMS **从来不会被重 pump**——这**掩盖了分叉而不是消除它**。

  | 方向 | 代价 |
  | :--- | :--- |
  | (i) 给两处副本各补一份自有实现 | 改动落在 SAMS/DbAdapters 的冻结副本上，与模板继续脱钩 |
  | (ii) 恢复模板里的 `GetSqlString`、改由调用方有界化 | 推翻本次删除；`GetSqlString` 自身是无界的 `sprintf` 拼接，等于把安全项又还回去 |
  | (iii) 明确这两份副本为**手写冻结**、与模板脱钩并登记 | 最省；但须承认 SAMS 已不在模板治理范围内 |

- **`pumpall.py` 静默吞掉生成失败（2026-09-15 暴露，**已于批 2b 按方向 (i) 落地**）**：
  用户选定「`exit()` 改 `sys.exit(1)` 并让 `pump.py` 失败时不删目标」这一最彻底方案，三仓 `pump.py`/`pumpall.py` 均已改。**原记载里有一处失实**：旧版**并非**「失败后继续往下泵」——实测 `site.Quitter` 抛 `SystemExit(None)`、循环确实停了，旧版唯一的真实缺陷是**退出码为 0**，于是「一个模板炸了」在上层表现为「全部成功」。另外 `Mdb/src/Mdb/MdbTableRegistry.h` 那次消失的根因是 `pump.py` 失败分支**自己**在 `os.remove(out_file_name)`（不是 pumpall 继续跑），该行已删。
  - **未决部分（本批未动）**：同一缺陷在**其余 9 个仓**的副本里原样存活——`Docs`/`LibTest`/`Libs`/`Lottery`/`Python`/`SAMS`/`SAMSSharp`/`SharpLibrary`/`alg_common_gen`（其中 `alg_common_gen` **不是 git 仓**，改丢了无法回退）。是否把这套 diff 一并推送过去，或干脆把 **12 份副本收敛成单点**（`Templates/` 已是共享目录，工具链也可以），**待定**。已知现成靶子：`Docs/pumplist.xml` 的产物是**无扩展名的 JSON**、`SAMSSharp`/`Lottery` 有 `.razor`——本批新加的注释符白名单会让这两类**响亮失败**而不是静默写出坏文件。
  - 现状缓解仍在：`%TEMP%\pump-guard.py`（强制 `touch` 模板/模型后再 pump，并对产物做清点比对）。

- **六仓 C++ 规范对齐：批 2–7 的授权（2026-09-15 提出，全部未决）**：
  按 Harness §3，**公开 API 改名与高风险改动须逐仓单独取得授权**，不得援引批 1 Beacon 的授权（计划决策⑤）。批 1 的授权只覆盖 Beacon。以下五项各自独立，可以只批其中几项：
  - **①批 2 Templates 的 `namespace Spark`/`mdb` → PascalCase**：这两个命名空间生成的正是被 Mdb / DbAdapters / QT 通过 `find_package` 消费的**公开头**，改名会让三个消费方立刻编译失败。**必须与批 7 原子改、单独 commit**。`Cpp/Libs/PBApi` 另喂含已废弃 LibTest 的 Libs 仓。
  - **②批 3 Spark 的公开 API 改名**：`namespace Spark` 112 处 / 112 文件、小写访问器与方法约 1,295 处（最大头是 `length` 1,192 处，需先甄别哪些是我方方法、哪些是标准库——`std::` 一族已滤掉但跨库同名须人工确认）、C 风格 cast 134 处、`k` 前缀 221 处、`g_` 66 处、`enum CSV_PARSER_ERROR` + `CPE_*` → `enum class`。
  - **③批 3 Spark 的 §3 高风险项（须单独确认，与②可分开批）**：裸 `new` 46 处 / 裸 `delete` 20 处 → 智能指针（`LockFreeQueue.h`/`ObjectPool.h` 是分配原语内部，**按设计需保留并登记豁免**，其余可改）；`volatile` **6 处 / 2 文件** → `std::atomic`（§6 禁 `volatile` 作同步；**须先确认这 6 处是否真用于同步**——这是本批唯一的高风险项）。
  - **④批 6 QuantTrading 的公开 API 改名**：`namespace quanttrading` 87 处 / 83 文件 → `QuantTrading`（`mdb` 4 处须与 Mdb 仓同步）；`m_` 1,215 处 / 64 文件（`m_Mdb` 一个名字就 110 处）；`strcpy` 173 处 → 有界替代；裸 `new`/`delete` 81/8 → 智能指针。`namespace std` 的 `hash` 特化**不要动**，`py` 是别名**不动**。
  - **⑤B0-a（DbAdapters 的前置决策，非授权类）**：该仓 5 个生成脚本（`pump.py`/`pumpall.py`/`parseall.py`/`ParseTableModel.py`/`ParsePackageModel.py`）处于**未暂存的删除态**，且该仓**没有 `pumplist.xml`**——其生成的 `MdbStructs.h/.cpp`（1,105 行，占全仓 20%）**没有任何可复现的生成路径**。必须先定：**保留生成**（补 pumplist 并把脚本提交回来）、**转为手写**（删脚本、把 MdbStructs 标记为手写）、还是**从 Spark 复制工具链**。此决定不做，批 4 无法开工。

- **批 2b 登记待决①：生成物 `*TableList.h` 里的两个公开常量（2026-09-15，须授权）**：
  `Templates/Cpp/Mdb/ModuleTableList.h.tpl` 生成 `<module>TableIds`（带 `k` 前缀）与 `<module>TableList` 两个**命名空间作用域的 `inline const`——即公开符号**，都违 §1「常量 PascalCase、去 `k` 前缀」。实际命中 **5 个生成头 / 10 个符号**：Mdb 的 `test/TestMdb/FullTableList.h`（`kfullTableIds`、`fullTableList`），QT 的 `src/BackTest/BackTestTableList.h`、`src/MdOffer/MdOfferTableList.h`、`src/SimExchange/SimExchangeTableList.h`、`src/SimExchangeInit/SimExchangeTableList.h`（各一组；模块名在模板里被整体小写，故写作 `kbacktestTableIds`/`backtestTableList` 等）。使用点合计约 37 处。**须授权**——改法只在模板一处，但会同时改 Mdb 与 QT 两仓的生成物，属公开 API 变更。**注意**：批 2b 只把同一行里的 `(int)` 换成 `static_cast<int>`，**符号名一个没碰**。

- **批 2b 登记待决②：工具链的三处既有问题（未动）**：①`pump.py`/`pumpall.py` 用 `os.system` + `%` 拼串 + 硬编码 `python`（违 `python-style.md` §7「禁 `os.system`，应用 `subprocess.run(args, shell=False)`」，也踩 Harness §6 注入防护口径）——改它会动到三仓的调用方式（`model` 是空格分隔的多文件，现在靠 cmd.exe 切分），**风险大于收益，留到工具链收敛时一起做**；②「勿手改」头注释里嵌的是**调用时原样传入的模板路径**——`pumpall.py` 传的是 pumplist 里的固定串故稳定，但手工直调会产出不同首行，制造假 `git diff`；③`pump.py` 对产物**不做任何内容体检**，`os.replace` 成功即算成功——本批已用**扩展名白名单**堵住最坏的一类（写错注释符会产出语法非法的文件而退出码仍是 0）。

- **`InitDb` 全量快照的池化对象无人归还（2026-09-15 修 Mdb 时发现，**既有缺陷**，需跨仓决策）**：
  模板生成的 `TradingDayTable::InitDb`（11 张表同型）从 `ObjectPool` 取一批对象塞进 `std::vector` 交给
  `OnRecordBatchInsert`，但**没有任何路径把它们还回池**。根因在上游 `DbAdapters`：
  `AsyncDbWriter::OnRecordBatchInsert` 把它们换进 `DbOperateImpl`，而 `DbOperateImpl::DeallocateRecord()`
  对 `Insert`/`BatchInsert`/`Truncate` 三类**提前 return**（只对 `Update`/`Delete` 归还）。
  放大效应：`Mdb::OnDbConnected` **每次重连都会重跑一遍 `InitDb`**，故池是被逐步抽干的。
  这不在"命名 + 安全"范围内，且修它要动**跨仓的所有权契约**，**未动**。
  三个可选方向：(i) 由 `DbAdapters` 侧在 `BatchInsert` 后归还（改上游契约，波及全部消费方）；
  (ii) 由 `InitDb` 侧记录并负责归还（改模板，影响 Mdb 与 QT 的生成物）；
  (iii) 接受现状并加注释说明这是有意的所有权转移（若上游确实打算长期持有）。

- **`InitDb` 在订阅者为空时静默返回、无任何日志（2026-09-15 修 Mdb 时发现，**既有**，模板级）**：
  重 pump 带出的 `if (m_MdbSubscriber == nullptr) { m_DbInited = true; return; }` 修掉了 HEAD 上的
  **空指针解引用崩溃**，但**只是静默返回**——没有 `LOG_WARN` 一类记录，违反 Harness §6「错误路径不得
  无声吞掉」的精神（那里约束的是 `catch`，但同一意图适用）。改它属**行为变更**（会开始打日志），
  且模板级改动会波及 **QT 的 5 处**生成物，**须与 QT 一起重 pump**，故登记未动。

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

- **批 2b 审查登记（全部未动，2026-09-15）**：
  - **①（高）SAMS 处于「假安全」态**：批 2a 的 `sprintf` → `AppendDebugString` 加固与 `RiskIndex.h.tpl` 的公开成员改名**都到不了 SAMS**——`SAMS/Source/pumplist.xml:76` 指向**不存在的** `../Templates/Cpp/Risk/RiskIndex.h.tpl`（真实路径是 `Cpp/Ams/Risk/`），另有条目指向不存在的 `Cpp/Packages/`、`Cpp/Db/`。手写侧 `AccountRiskCheck.cpp:252,261,274,289,298,311,326,335,348` 有 **9 处**调用点仍写旧名。**不是「SAMS 已废弃所以没事」**——它照旧编得过，因为模板根本不在任何 pumplist 里。
  - **②（中）`snprintf` 返回值未判**：`Templates/Cpp/Mdb/InitMdbFromCsv.cpp.tpl:124`——截断静默，违 §6「数值安全」。
  - **③（低）模板 DRY 噪音**：`MdbTables.cpp.tpl:49,51 / :57,59 / :193,197` 相邻重复的 `!!nameLower = …!!` 赋值。产物中性（该变量无副作用），属模板可读性。
  - **④（低）`::memcpy` 与未限定 `memcpy` 并存**：`MdbTables.cpp.tpl:244` 写 `::memcpy`，同文件 `:95,149,212` 写 `memcpy`。
  - **⑤（低）残留前缀（非公开 API）**：`m_Protocol` 8 处（只声明在手写的 `QuantTrading/src/Apis/ApiBase.h`，属批 6）、`s_!!$prefix!!Api`/`SpiWrap`（`ApiTest/CApi.cpp.tpl`）、`int result` 局部（`ApiImpl.cpp.tpl:87`/`GbkApiImpl.cpp.tpl:131`）。
  - **⑥（不纳入）** include 顺序（`<cstdio>` 位置）按决策②「格式类不纳入」排除。
  - **⑦（中，前向风险）`pump.py:177` 注释符白名单漏 `.cu`**：只列 `{.c,.cpp,.h,.hpp,.cs,.sql}`。三仓现状全覆盖、本批不会当场炸；但 `alg_common_gen/pumplist.xml:3` 有 `dest="…cuda_tensor_implement_gen.cu"`，`.cu` 就是 CUDA C++（注释符 `// `）。**这套 `pump.py` 一旦推过去，会在该条抛 `MyException` → `sys.exit(1)` → 整仓 pump 中止**；而已登记的靶子只有 `.razor` 与无扩展名 JSON，`.cu` 不在其中。**决定前不动**——补 `.cu`/`.cuh` 会预设"要推"（推送本身尚未决）。
  - **⑧（中，HEAD 即存在）Mdb 两份 README:281 示例用了未定义的 `tradingDay`**：`mdb->capital->EraseByTradingDayIndex(tradingDay->PreTradingDay);` 全篇无定义。**非本批引入**——`git show HEAD:README.md` 同行已写 `tradingDay`，本批只把 `t_Capital` 改成 `capital`（那处是对的）。需补一行 `TradingDay* tradingDay = mdb->tradingDay->primaryKey->Select(pk);`。
  - **⑨（低）`pump.py` 细节四处**：`:353-356` 回退失败时把旁路产物也删了（此时目标可能已被 `open(...,'wb')` 截断 → 新旧一起没；窄窗口，宜保留并打印路径）；`:357` 回退成功仍 `rc=0`，编排层分不清「降级写」与「干净成功」；`:338→:358` 未包 `finally`（Ctrl-C 留 `.pumptmp`，已 gitignore）；`:175-176` 注释说"写任何东西之前"，但检查在 `:179` 而 `:145` 已把 `pumptemp.py` 截成 0 字节（整块上移即符合注释）。
  - **⑩（低）DoPump 仍是未加引号的 `%` 拼接**：`pumpall.py:53` 的 `os.system("python pump.py %s %s %s %s")`——XML 字段含空格即断词，`python` 亦硬编码。宜改 `subprocess.run([...], shell=False)`。
  - **⑪（低）Mdb 两份 README:233 注释列位偏 2 列**：`t_Exchange` → `exchange` 短了 2，与同块 `:232` 收尾不再对齐。纯观感。
  - **⑫（批 6 范围）QT 手写 4 处 `(T*)ptr`**：`src/BackTest/SimExchange.cpp:287,290,293,296`，HEAD 即存在、非本批引入，归批 6。

- **IO 族改名的仓外跟随（2026-09-16，需决策）**：本批在 Spark 仓内已收口，但对外可见性分两处，**都不在本批授权范围内**：
  - **①QuantTrading 的 16 处 / 10 文件机械跟随**：走 `find_package(Spark PATHS "../Libs/Spark/x64-windows")`。按 Harness §3 与计划决策⑤，**须在 QT 仓单独取得授权、单独 commit**，不得援引 Spark 本批的授权。**前置条件**：QT 当前**已经编译不过**——它调 `Protocol::SetIOThread`，而该名只存在于 Spark 的 `HEAD`（工作区与已发货 SDK 均已是 `SetIoThread`）。故必须先跑一次 QT 基线并**记录既有失败项**，否则「本批引入的失败」与「既有失败」无法区分。
  - **②已发货 SDK `D:/Gitee/Libs/Spark/x64-windows` 的重发**：该目录未被 git 跟踪，重发本身不是 git 操作，但它**改变了 QT 编译时所面向的头文件**，属对外可见动作，**须用户确认后再做**。**重发有一个 NTFS 陷阱必须处理**：`install(DIRECTORY)` 只增不删，在大小写不敏感的文件系统上会把新头写进旧 `IO/` 目录**并保持旧目录名**——Windows 侧被掩盖、**Linux 侧真实分裂成 `IO/` 与 `Io/` 两个目录**。合规做法是装前先把旧 `IO/` **单次 `mv`** 让位（`mv` 不是递归删除，符合 Harness §1），再用 `python os.listdir` 核对磁盘真名（`cmd /c dir` 与 `ls` 在本环境不可信）。
  - **③LibTest / DAG：本批零动作，且已确认它们不是 Spark 的消费方**（更正此前记载）——LibTest 引的是自带的 `"PersonalLib/Network/IO/IOBase.h"` 并调用 `IOFactory::CreateIO`（该名在 Spark 的 `HEAD` 早已不存在）；DAG 的 `DAGDemo/PersonalLib/` 是自带 `.lib` 的冻结 vendored 快照，在 DAG 仓里**未被 git 跟踪**。**据此销掉此前把这两仓列为「待跟随消费方」的登记。**

- **`IoFactory.cpp:45` 的日志字段标签 `IOType`/`IOModel`（2026-09-16 发现，待决）**：本批**未改**，理由是该行是日志文本、规则约束的是标识符，且代码里从不存在叫 `IOType` 的标识符。**但它与「本批改了 7 处日志字面量」并不矛盾**——那 7 处的判据是「字面量内容就是类名本身」（自指），**不是「字符串一律可改」**。规范拼写确由上游 `Model/Types.xml:222,227` 定为 `IoType`/`IoModel`，且同一行里 `ServerType` 恰是「类型名 `ServerTypeType` 去尾 `Type`」的写法，**故它是这行里唯一拼法落后于类型名的标签**。若改，需先与日志消费方（日志解析 / 告警规则）确认这些字段名有没有被当作键使用。**待决。**

- **`ServerIoSubscriberImpl.cpp:37-38` 的无界 `sprintf` + 格式化串注入（2026-09-16 扫描时发现，**既有缺陷**，本批未动）**：属 §6 明令禁止项（禁止 `sprintf`；外部输入不得直接拼接构造）。虽在 `test/` 下，但它是**端到端冒烟测试唯一走的收发回调**，改它与改生产代码同样要走闸门与授权。**登记，择批修。**

- **`TcpIocpCompletePort.{h,cpp}` 的文件名与它实现的类 `IoCompletePort` 不一致（2026-09-16 复核，**既有**违 §2，影响面 0）**：本批只改大小写、不改词根，故未一并处理（真改名会牵动 `include/` 与 `src/` 下 4 个文件 + 全部 include 行，属独立改动）。**需单独授权。**

- **`.gitignore` 的两处缺口（2026-09-16 发现，本批未动）**：`log/`（Logger 的运行时产物目录）与 `TestShm`（Shm 测试落盘文件）**都未被忽略**。证据：本批冒烟实测产出了 `log/TestClient.*.log`、`log/TestServer.*.log` 与一个 8 MB 的 `TestShm`，三者**既不在 `HEAD`、也不被忽略**——一次 `git add -A` 就会把日志与 8 MB 文件连同代码一起提交。**待决：补 `.gitignore`，还是把产物改到已被忽略的路径。**

- **CSV 族的改名（2026-09-16 复核，**本批刻意未动**）**：计划里 CSV 与 IO 同批，但用户明确要求「CSV 文件内容先别动」，故本批只做了 IO。**纯目录改名 `src/Serialization/CSV/` → `Csv/`（零内容改动）可随时执行**，但它与 `include/Spark/Serialization/Csv/` 的不一致属既存状态、非本批引入。**CSV 的类名（`CSVParser`/`CSVRecord`）、枚举（`CSV_PARSER_ERROR` + `CPE_*`）、测试文件名仍需授权**——见本区②。`MD5` 作为算法专名豁免，保留。

## 备注

- 宿主必须显式调用 `Logger::Stop()` + `Join()` 收尾，否则最后一次缓冲必丢；这是进程退出时序的**定论**，不是可以靠改析构语义绕过的缺陷——见归档 `Q.17`
- P5 握手（协议版本协商）已决定**不做**，日后若要做的入口是 `Protocol::OnConnect`——见归档 `Q.18`

## 归档索引

- `D.21` 六仓 C++ 规范对齐 · 批 2 前置：Mdb 停滞生成物补齐（`pumpall.py` 静默吞失败一节的根因，2026-09-15）
- `D.20` 六仓 C++ 规范对齐 · 批 1 Beacon 完成（含批 2 前置的三条教训、跨仓交付纪律，2026-09-14/15）
- `D.19` STEP 协议数字化收口：全定宽大写十六进制、`MsgSeqNum`/`CheckSum` 无符号化、两次 `HeadToStream` 塌缩成一次
- `D.18` 协议写路径收尾：C 风格 cast 清零、`MakePackage` 前置闸门、`HeadToStream` 改走 `StepWriteCursor`、测试脚手架去重
- `D.17` PROGRESS.md 归档拆分（已关闭条目的首次分层搬运）
- `D.16` STEP 写路径收口：有界游标 `StepWriteCursor` + `std::format` + 写侧类型拼写（含一处推翻计划的风险断言）
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
