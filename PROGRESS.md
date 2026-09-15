# 项目进度

> 本文件用于跨会话状态记录，新会话开始请先阅读。

## ✅ 已完成

- **六仓 C++ 规范对齐 · 批 2b：Templates 残留 C 风格 cast 清零 + 生成失败路径加固（2026-09-15）**：
  批 2 至此只剩 `namespace Spark`/`mdb` → PascalCase（公开 API，须授权、与批 7 原子改）。
  - **① C 风格 cast 清零：17 处 / 11 文件，分两轮**（`%TEMP%\tpl-ccast-fix.py` 10 条 + `%TEMP%\tpl-ptrcat-fix.py` 3 条，逐条带命中数断言、幂等可重入）。
    第一轮 **11 处 / 8 文件**：两类形态，C 关键字类型（`(int)`/`(bool)`/`(void*)`/`(const char*)`）与模板变量类型（`(!!@type!!Type)`）；`enum` 分支沿用 `Packages.cpp.tpl` 已用过的条件三元组写法，不另创。
    **第二轮补 6 处 / 3 文件（`(T*)ptr` 指针形态，是独立审查抓到的漏网，见 ⑧）**：`Cpp/Api/ApiImpl.cpp.tpl:48,56`、`Cpp/Api/GbkApiImpl.cpp.tpl:60,81`、`Cpp/Protocol/Kernel/KernelGen.cpp.tpl:48,58`，一律 `((!!$packageName!!*)package)` → `static_cast<!!$packageName!!*>(package)`。**选 `static_cast` 而非 `reinterpret_cast` 已核实**：`Packages.h.tpl:16` 为 `class !!$className!! : public Package`（单一、非虚、公有继承），模板上下文里 `package` 声明为 `Package* package`，故下行转换合法且与 C 风格等价。本轮 churn：**只 QT 变**（Spark/Mdb 重 pump 无输出——这 3 个模板不喂它们），产物里 `static_cast<XxxPackage*>(package)` **107 处 / 8 文件**，旧形态在该 8 文件归零。
  - **② 举证是受控对照，不是看 diff**（`%TEMP%\ccast-verify5.py`）：10 条编辑整体回退 → pump → 基线，正向应用 → pump → 现状。**168 个产物变更 22 个**（Mdb 5 / QT 17 / Spark 0——Spark 0 可解释：这 8 个模板没有一个喂 Spark）。三重判据：变更集合符合预期、**每文件行数不变**、**两侧 cast 语法都摘掉后剩余逐字节相等** → **越界 0 处**；往返一轮 SHA-256 回同值、`.pumptmp` 残留 0。
  - **③ `pump.py` 加固（承 ❓ 区同名条目的方向 (i)；三仓各一份、md5 一致）**：usage 分支 `sys.exit(2)`；生成失败**不删目标**、改留 `<目标>.pumptmp` 并 `sys.exit(1)`；注释符由「`.sql` 否则 `// `」改成**扩展名白名单**，未知扩展名在写任何东西之前响亮失败；`pumpall.py` 的 `DoPump` 裸 `exit()` → `sys.exit(1)` + 中止信息。端到端五例（`pump-e2e2.py`）：A 正常 rc=0；B 模板坏 / C 未知扩展名（**在写之前**就退出）/ D 参数不足（**旧版会打印 usage 后继续覆盖目标并 rc=0**）三条 rc≠0 且**目标原样**；E `os.replace` 被挡 → 退回就地重写、rc=0 且内容更新；并证**字节中性**（加固后强制全量重泵：168 个产物内容变化 0、缺失 0、残留 0）。
  - **④ 其中一处是我引入的回归，由 `code-reviewer` 抓到**：`os.replace` 在 Windows 上当目标被别的句柄打开会抛 `WinError 5`，而它替代的旧写法 `open(dest,'w')` 只需写权限——于是「能就地重写」的场合变成**未捕获异常**，绕过我自己写的 `pump failed:` 并把 `pumpall.py` 停在半路。已加 try/except + 二进制就地重写回退。
  - **⑤ 三仓重建重测全绿**：六配置 MSVC rc=0、0 错误；Spark `UnitTests` **392/392** ×2；Mdb `TestDB.exe` rc=0 ×2（Sqlite+Duckdb）；QT `UnitTests` **101/690** ×2；QT 5 个集成 exe **10/10 rc=0**。**⑥ `.gitignore` 对齐（关闭 `:18` 登记）**：`__pycache__/` 补进 Spark 与 QT、Mdb 的 `/__pycache__` 归一为 `__pycache__/`（覆盖子目录）、新增本批引入的 `*.pumptmp`；三仓 `git check-ignore -v` 逐条核过。
  - **⑦ 本批我犯的两个错（留证）**：①首轮 C-cast 扫描**只认 C 关键字类型**，把「自定义类型」这一族整个漏了（审查在 `Mdb/InitMdbFromCsv.cpp.tpl:173` 抓到，且已落进两仓产物）——与此前批评的「没有全仓重扫」**是同一类错误**；②给 Mdb/QT 复制 `pump.py` 时**行尾写成了 CRLF**（HEAD 是 LF），而 **git 看不见**——全局 gitattributes 的 `*.py text eol=lf` 让 `git hash-object` 比的是**规范化后**的字节；靠逐仓比**工作区** md5 才发现。**跨仓复制文件不能用 `git hash-object` 判等。**
  - **⑧ 本批第三个错，也是最重要的一条：扫描口径写窄 = 假绿灯。** 第一轮收尾时我用 `%TEMP%\tpl-ccast-rescan.py` 复扫，报「真实 C 风格 cast **0 处**」，我据此把 ③ 记为**已关闭**并写进本文档。**独立审查用不同口径扫出 6 处真实残留**（上条第二轮）。根因不是漏看，是**判据本身失明**：`CAST_OLD` 长了一张**类型白名单**（`(const )?char * | void * | unsigned int | int | bool | <X>Type`），而漏掉的那一族类型名是**占位符** `!!$packageName!!`，**结构上不可能被白名单匹配**——白名单对「生成式类型名」天然失明，而本仓 96,612 行产物**全是生成式类型名**。改法：`%TEMP%\tpl-ccast-full.py` 改为**故意过收**——不预判哪些像类型，凡「括号紧跟操作数起始」一律列出、由人判读（93 个 `.tpl`、候选 158 处、`cast_like` 24 处，逐条判读后真 cast 恰为 6，其余是形参声明 / 函数指针 typedef / `offsetof` 限定名 / C# 越界）。**纪律**：本仓的扫描器一律「过收 + 人工判读」，不得再用类型白名单结案；「0 命中」在报告之前必须先证明**判据本身能命中正样本**（我没做这一步，是这次翻车的直接原因）。
  - **⑨ 顺带扫出、但不属本批的 4 处 `(T*)ptr`**：`QuantTrading/src/BackTest/SimExchange.cpp:287,290,293,296`，**手写**文件（非 pump 产物）、**HEAD 即已存在**（逐行比对确认非本批引入）→ 归**批 6**，已登记 ❓ 区未动。
  - **登记（本批未动，见 ❓ 区）**：生成物 `*TableList.h` 的 5 头 / 10 个公开常量须授权；工具链三处既有问题（`os.system` 拼串 / 头注释嵌模板路径 / 产物无体检）；**新登记 6 条**（`.cu` 白名单缺口、Mdb README 的 `tradingDay` 未定义示例、pump.py 回退路径四处、README 注释列位）见 ❓ 区同名条目。

- **六仓 C++ 规范对齐 · 批 2a 三项收口：模板有界化 / `using namespace std;` 清零 / 生成物「勿手改」头（2026-09-15）**：
  用户定的三项一次做完再动 Spark（「合并成一轮做，churn 只翻一次」）。**批 2a 至此收口**，批 2 只剩 `namespace Spark`/`mdb` → PascalCase 那一段（公开 API，须与批 7 原子改）。
  - **① `sprintf` 有界化（安全类，本批价值最高的一项）**：最后一处无界写是 `Templates/Cpp/Protocol/Packages/Packages.cpp.tpl` 的 `GetDebugString`——写目标是 `thread_local char DataStringBuffer[10240]`，`offset` 跨全包字段累加、**没有任何上界**。新增文件内 helper `AppendDebugString(int offset, const char* format, ...)`：用 `vsnprintf` 按剩余容量写入并返回新 offset，越界只截断不越写；**264 处**调用点由 `offset += sprintf(...)` 改为 `offset = AppendDebugString(offset, ...)`。同时补 `#include <cstdarg>` / `<cstdio>`。**§5 取舍**：一个 helper 对 264 处调用点，而不是逐点加护栏（`offset` 是跨字段累加的，逐点判断等于把同一段判断抄 264 遍）。至此 `Templates/` 内 `sprintf` **0 处**。
  - **② `using namespace std;` 清零**：5 处全清（`BackTestApiImpl.cpp.tpl`、`Config.cpp.tpl`、`ServerConfig.cpp.tpl`、`InitMdbFromCsv.cpp.tpl`、`InitMdbFromDB.cpp.tpl`）。随之把因去 `using` 而失去限定的 `printf`/`fstream`/`vector` 改为显式限定，或补 `#include <cstdio>`（`Config.cpp.tpl`、`ServerConfig.cpp.tpl`）。
  - **③ 生成物「勿手改」头：落在 `pump.py`（三仓各一份），不是改 66 个模板**。`sys.argv[2]` 就是真实模板路径，按目标扩展名选注释符（`.sql` → `-- `，其余 `// `），产物首行写 `// 本文件由 <模板路径> 生成；请勿手改，改动请改模板后重跑 pumpall.py`。落盘编码沿用 `UTF-8-SIG`。**这条是堵历史隐患的**——本仓 96,612 行生成物此前没有任何「勿手改」标记。
  - **③ 的证明（`pump-header-proof2.py`，两个 168/168）**：①把每个产物的首行去掉后，与「用删掉该块的 `pump.py` 重新生成的无头版本」比对——**168/168 逐字节相同**（即多出来的确实只有那一行）；②恢复后再次强制重 pump——**168/168 逐字节相同**（幂等）。
  - **churn 逐条核对（不接受「反正是 pump 出来的」）**：首次全量重 pump 后 168 个产物全变（多一行头）。以「头之外是否还有差异」为 oracle：恰好 **20 个**文件有实质变化，且逐一对应到本批改过的模板（`MdbStructs.h/.cpp` ×4、`InitMdbFromCsv/DB` ×4、`Config.cpp` ×9、`BackTestApiImpl.cpp`、`Packages.cpp` ×2）。收尾再 pump 一轮只变 **2 个**（QT 与 Spark 的 `Packages.cpp`）——正是 `Packages.cpp.tpl` 单独的爆炸半径，**反证** `RiskIndex`/`ServerConfig` 的改动只落在 SAMS（不在 pump 范围内）。
  - **三仓重建重测全绿**：Spark Debug rc=0 / `UnitTests` **392/392** / Release rc=0 / WSL GCC **391/391**；Mdb Debug+Release rc=0、`TestDB.exe` 的 Sqlite + Duckdb 两段通过；QuantTrading Debug+Release rc=0、`UnitTests` **101 用例 / 690 断言全过**、5 个集成 exe **RC=0** 且线程干净退出。**QT 这次能构建，顺带关闭了一条旧登记**：`Libs/DBAdapters/x64-windows` 已由 2026-08-20 的旧副本刷新为 **2026-09-14 09:41**（6321 字节，含 `422e92a` 的 FieldType 扩展），`MdbStatic` 不再失败。
  - **字符串字面量多重集比对**（承批 1 教训的必跑闸门，本轮覆盖 212 个改动文本文件）：**只有 3 个文件有差异，且全部是「只删不加」**——`MdbStructs.cpp` 三份里 `GetSqlString` 的 SQL 格式串（用户授权的删除，跨仓后果见 ❓ 区）。
  - **独立审查（`code-reviewer`）严重 4 / 高 4 的处置**：严重 3（新增的 `AppendDebugString` 可能未使用）→ 加 `[[maybe_unused]]`（C++20 已确认，`Spark/CMakeLists.txt:15-16`），WSL GCC 构建复验通过；严重 1（`GetSqlString` 删除）→ 用户已明确授权「删掉该函数」，保留；严重 2（本文档把批 2a 记作「非公开 API 部分」）→ **本条目即更正**，公开成员这轮一并改了；严重 4（批 2a 不是纯改名、无法原子拆分）→ 属实，用户已定「合并成一轮」。高 2 / 高 4 → `ServerConfig` 的 `m_Instance` → `instance`、`in_file` → `inputFile`；另 `RiskIndex.h.tpl` 的 9 处公开成员 `m_*` → camelCase。
  - **两处我自行判断的改动（附回退面）**：①`RiskIndex.h.tpl` 公开成员改名，援引的是**同仓既有裁决**「公开成员一律 camelCase 去前缀」（该裁决由用户答；批 2a 的授权原文亦为「连公开成员一起改名」）。回退面：模板 9 处 + SAMS 侧消费点，无逻辑变更。②`Mdb/test/TestMdb/TestDB.cpp:274` 保留 `//TestMysql();` —— HEAD 的第 275 行 `//TestMariadb();` 本来就是注释，属该文件既有约定；且 Mysql 段需要 33060 的活服务，本机没有。回退面：1 行。
  - **登记（本批未动）**：`m_Protocol` **8 处**保留——它只声明在手写的 `QuantTrading/src/Apis/ApiBase.h`，不在任何模板里，属批 6；`TestDB.cpp` 的 `t_tradingDay`/`t_exchange`/`t_account` 等局部蛇形名属批 5；`LibTest` 与 `SAMS` 消费了本批改过的模板但**未重 pump**（一个已废弃、一个不在范围），存在语义漂移、**无编译错误**；`__pycache__/` 在 Spark/Mdb/QT 三仓未被忽略（`pumpall.py` 的运行副产物），宜补进 `.gitignore`。
  - **⚠️ 证明阶段 `Mdb/src/Mdb/MdbTableRegistry.h` 被删过一次（741 字节，git 状态 ` D`）**：根因是 `pump.py` 生成失败时**先 `os.remove(out_file_name)` 删目标**，而 `pumpall.py` 的 `DoPump` 用裸 `exit()`（**返回 0**）把失败静默吞掉。已用 `%TEMP%\pump-guard.py` 恢复；随后连续 4 轮全量 pump **未复现**。**这是 `pumpall.py` 的既有缺陷，三仓都有，已登记待决（见 ❓ 区）。**
  - **本文档的滚动**：本条目入区后 ✅ 达 6 批，按 §8.1 移最旧三条入归档（`D.19`/`D.18`/`D.17`）。搬运对条目边界、脚本从 `git HEAD` 取原文、只移动不删改；半关闭条目（`D.18`/`D.19`）的未决 bullet 抽出后留在 ❓ 区，不随历史一起埋掉。

- **六仓 C++ 规范对齐 · 批 2 前置：Mdb 停滞生成物补齐（2026-09-15，提交 `0bb5fa6`）**：
  批 2 开工前先做**生成物可复现性验证**——把 pumplist 引用的 tpl/model 全部 `touch`（只改 mtime、不动内容）后强制全量重 pump，再比对 git 变更。结果：**Spark 9 个产物 0 变更、QT 130 个产物 0 变更、Mdb 29 个产物有 7 个变更**，问题只在 Mdb。
  - **根因是"改了源头没重 pump"，两层**：①上游共享模型仓 `D:\Gitee\Model`（**六仓之外的独立 git 仓**，各仓以 `../Model/Types.xml` 引用）的提交 `43fc0cc 使用明确位数的整型`（2026-09-13）删掉了 `Int` 类型，`IntType` typedef 不复存在；Mdb 的 `Model/Tables/Tables.xml` 早改成 `type="Int32"`，但 `MdbStructs.h`(2 处) / `MdbPrimaryKeys.h`(1 处) / `MdbPrimaryKeys.cpp`(1 处) 仍写 `IntType` → **实测编译失败**（C3646「未知重写说明符」/ C4430「缺少类型说明符」），即**本仓在这次提交之前是编不过的**。②另 4 个文件落后于模板 `6d12e3d`（2026-08-27）：`MdbTables.cpp`(253 行)、`InitMdbFromCsv.cpp`(22)、`MdbPrimaryKeyComp.cpp`(12)、`FullTableList.h`(8)。
  - **这 4 个文件不是纯改名，含真实修复与一处内存管理改动**：`InitDB` 的订阅者空指针守卫（HEAD 在该分支是 nullptr 解引用崩溃）、`shared_lock` 作用域收窄到内层花括号、`records->push_back(new TradingDay(**it))` → `TradingDay::Allocate() + memcpy` 的对象池分配、`InitMdbFromCsv` 的裸 `new` 改 `Allocate()`（HEAD 的 `Insert` 失败路径会把堆指针 `Deallocate()` 进 `ObjectPool`，之后同一地址可能被两次发放）、`std::hash<char>()((char)record->PosiDirection)` → `std::hash<PosiDirectionType>()` 去掉一处 C 风格 cast。**内存管理改动已按 Harness §3.2 单独取得用户授权**。
  - **第三个坑：重 pump 本身会产出编不过的代码。** 模板 `ModuleTableList.h.tpl` 已改用 `@project`/`@module` 生成 `namespace <project>::<module>`，而 Mdb 的 `FullTableNames.xml` 还是旧 schema `<dbtables prefix="Full">`，两个变量解析成空串 → 输出 `namespace ::`、`kTableIDs`、`""`。已迁到 `project="Mdb" module="Full"`（对齐 `QuantTrading/Model/TableNames/MdOfferTableNames.xml` 的 `project="QuantTrading" module="MdOffer"`），消费方 `TestDB.cpp` 随之加 `using namespace mdb::full;` 并把 `FullTableList` 改 `fullTableList`。
  - **验证**：MSVC x64-Debug/Release 均 rc=0、0 错误（**回退那三处 `IntType` 后同一构建失败**，作对照——故"HEAD 编不过"是实测而非推断）；`pumpall.py` 幂等（两次独立 touch+pump 产出同一 SHA-256 `368b93be…`），**提交后重跑可复现性验证得 0 变更**；`TestDB.exe` 实跑 Sqlite 与 Duckdb 两段全过、写入读回数据正确（这条路径正是 `Allocate()+memcpy` 的位拷贝，POD 结构数据无损）；Mysql 段因本机 33060 无服务被拒（rc=3，与本次无关——HEAD 的 `main()` 同样调 `TestMysql()`）。独立审查 `code-reviewer`：**0 个阻断项**。
  - **审查的建议 A 已一并处理（中英文 README 的示例同步）**：不再教读者手写 TableList，改为 `#include "FullTableList.h"` + `using namespace mdb::full` + `fullTableList`；另把 README 里的 `model/` 更正为 `Model/`——**非改名所致**，本仓目录自始至终是 `Model/`、`pumplist.xml` 也一直写 `./Model/`，是 README 文档本身写错了。（我起初把这个归因写成"`0219614` 已改目录名"——那是 **Spark 仓**的提交，且 Mdb 根本没有过这个改名；提交说明已 `--amend` 改正，tree hash 未变。）
  - **三条教训（直接影响后续批次）**：①**"重 pump 就好了"不成立**——生成物与模板一致不等于代码正确，模型 schema 没跟上照样产出坏代码，**可复现 ≠ 正确**；②`git checkout` 回退文件后 `pumpall.py` **不会**重新生成（`NeedPump` 按 mtime 比较，checkout 把 mtime 刷成"现在"、比模板新），**必须 touch 模板/模型才能强制**；③Mdb 提交 `1b823c7`（"模型修正：Tables.xml 的 type=Int 改 Int32；MdbStructs.cpp 重 pump"）正是根因的活标本——当时**只重 pump 了 1 个文件**，这种"局部重 pump"直接制造了自相矛盾的生成态。
  - **对批 2 的约束（已写入计划 B0-c）**：`Cpp/Mdb/*.tpl` 一改，这 7 个文件必然再变一次。故本批必须先落地、单独 commit，否则批 2 的 Mdb 变更会同时压着三层改动（类型名 / 池化逻辑 / 规范改名），事后无法分别审阅与回退。全族同类症状（`namespace ::` 之类）已扫过，**无第二处**。

- **六仓 C++ 规范对齐 · 批 1 Beacon 完成（2026-09-14/15），批 2–7 待逐仓授权**：
  把 `rules/cpp-style.md` 落到 6 个仓（Spark / Templates / DBAdapters / Beacon / Mdb / QuantTrading）。**主轴是"改模板 → 重 pump → 再改手写"**：这 6 仓约 96,612 行是机器生成的，源头只有 64 个 `.tpl`，直接编辑生成物会被下次 `pumpall.py` 静默回滚。计划文件：`~/.claude/plans/snazzy-chasing-alpaca.md`。
  - **已锁定的 6 项决策（不再重议）**：①全量迁移（命名类 + 安全类）；②**格式类不纳入**（§2 include 顺序、§3 换行/150 阈值）——**注**：该条原先的立论"`.editorconfig` 也没有 `max_line_length`，无法被工具拦下"**是错的**（两仓均声明 150，且真实阈值下本仓本就达标）；2026-09-15 已核实、更正并六仓统一定稿，见下方第三轮；③`m_` 前缀先做 Templates+Spark，QT 存量单独排批；④先全量重测再动手；⑤**公开 API 改名按仓逐一取得授权**；⑥**Python 绑定字符串保留**（Beacon 的 `"kCosine"`/`"kInnerProduct"` 不动，Python API 零破坏）。
  - **批 1 Beacon 已完成并提交**（Beacon 仓 `bd95bd4`；本仓 `2d83422` 是 PROGRESS 文档提交，非代码）：15 个源文件 + `README.md` + 设计文档，**854 增 / 854 删**（完全对称，符合纯改名特征；第一轮 790/790，第二轮补 struct 公有成员与 `Data()` 后增至 854/854）。本仓零生成物，每处都是直接编辑。改动：`namespace beacon`→`Beacon`、私有成员去尾下划线、公开访问器改 PascalCase（`count`→`Count`、`slot_count`→`SlotCount`、`set_data`→`SetData`…）、枚举值 `kCosine`→`Cosine`、常量与局部去 `k` 前缀与蛇形。
  - **第二轮（2026-09-15，代码审查驱动）：补上「半迁移」缺口** —— 我先前只给 `FileHeader` 改了 struct 公有成员，漏了 `Hit`，而 `Hit` 才是真正对外可见的那个，方向恰好相反。**判定口径定为"按 C++ 成员访问级别"而非"跨 TU 可见性"**（规范 §1 表原文即"公有成员变量(struct) → PascalCase"，`struct` 数据成员默认 public），故一律 PascalCase：`Hit{id,score}`→`{Id,Score}`、`HnswIndex::Candidate{id,score}`→`{Id,Score}`、bench 的 `Dataset`/`BenchParams`/`BenchResult` 全部成员（`m` 顺带语义化为 `MaxNeighbors`，与主代码 `maxNeighbors` 一致，同时消掉审查指出的"`m` 一个字母两种含义"）。共 **127 处使用点 + 5 处声明**。**这是公开 API 改动**（`Hit::Id`/`Hit::Score`），已按批 1 授权范围处理并在提交说明中标注；**Python 侧零破坏已用运行验证**：`repr` 仍为 `<Hit id=1 score=0.993884>`、`h.id`/`h.score`/`Metric.kCosine` 全部原样（绑定层用显式字符串映射，按决策⑥保留）。
  - **第二轮顺带修掉的一处既有缺陷（由审查提出、用户修复）**：`VectorDb::Save` 的 `hasIndex` 标志按 `(index && !indexDirty)` 算，但写出条件却是 `if (index && !index->Write(out))`——**不含 `!indexDirty`**，于是索引脏时 flag 写 0、整段索引字节照写（多则数 MB）；读侧因 flag=0 跳过并忽略尾部字节，**功能正确**，因此 `VectorDbTest` 的 `DirtySaveOmitsIndexSegmentThenLazyRebuild` **修复前也通过**——它当时只证明了"读侧会跳过"，没证明"段真的没写"，用例名是名副其实的。**承自 HEAD，非本批引入**，且属序列化写出条件、落在"命名 + 安全"范围外，我登记后未动；**用户随后亲自改成 `if (hasIndex && !index->Write(out))`**（与标志一致，读写两侧对称）。**我用脚本实测确认**：同一份数据，"无索引基线"与"有索引 + Load 无索引文件 → indexDirty"两条路径保存出的文件**同为 118 字节、逐字节等长**，且该文件能读回、向量与元数据一致 → 索引段确实不再写出。**向后兼容**：旧代码写出的"flag=0 + 尾部带杂散索引字节"的历史文件，新读侧仍只依赖 flag、照常忽略尾部。设计文档「索引脏时不写索引段」一句至此才成为事实。
  - **第二轮同时处置的审查建议**：①**`VectorTable::Data()`→`FlatVectors()`** —— `data` 是 Harness §4 明列的无信息名称，而它是公开访问器；全仓仅 1 个调用点（`VectorDb.cpp` 的 `WriteVectorData`），故改名而非私有化（私有化需 `friend`，属更大改动）。私有成员 `data` **不改**——§4 的约束对象是"公开接口的标识符"。②**删掉我新加的注释**`// 度量按 u8 落盘；不用 Metric 以免遮蔽…`：Harness §4 只允许 workaround/外部库坑/非常规性能技巧三类注释，且须单独确认；字段名 `MetricCode` 本身已自解释，若有人改成 `Metric` 编译器立刻报错，注释无必要。③`MetricToU8(Metric m)`/`U8ToMetric(std::uint8_t v)` 的形参形与 HNSW 的 `M` 同名不同义 → 改 `metric`/`metricCode`。④`README.md:12` 的格式字段名与设计文档对齐（`version+dim+metric+count` → `Version(3)+Dim(u64)+MetricCode(u8)+SlotCount(u64)`）。**⑤审查建议的 `Deleted()`→`IsDeleted()` 我未采纳**——规范里找不到"谓词须加 `Is`"的条文，不按口味改动。**⑥`GetMetric()` 与同级 `Count()/Dim()` 命名不一致是刻意为之**——`Metric Metric() const` 会让成员函数名遮蔽同名 `enum class`，审查亦确认"保持现状即可"，记录在案以防后人当遗漏"统一掉"。
  - **四处编译器抓不到的撞名（本批最关键的收益，全部已处理）**：①`VectorTable::SetData` 的形参与成员同名，机械去前缀会让 `dim_ = dim` 等 **5 处退化成自赋值空操作** → 形参改 `newDim`/`newMetric`/`newData`/`newMetadata`/`newDeletedFlags`；②`HnswIndex::Read` 的局部与成员同名 → 局部改 `storedM`/`storedEnterPoint`/`storedTopLevel`…；③`HnswIndex::Remove` 的 `const int node_level = node_level_[id]` 会变成**用未初始化的自身** → 局部改 `nodeLevel`；④`metric()` 改 `Metric()` 会**遮蔽 `enum class Metric`**，同文件的 `Metric metric = …` 随之编译不过 → 改 `GetMetric()`（合 Spark 既有 `GetInstance()` 风格）。**根因**：本工程 `CMAKE_CXX_FLAGS` 被覆写、**无 `/W` 等级**，MSVC 的 C4458（形参隐藏成员）默认静默。
  - **5 处超出"机械去前缀"的语义化改名（须知悉）**：`m_`→`maxNeighbors`（裸 `m` 违 §4 表意，且与 `Read()` 局部撞名）、`deleted_`→`deletedFlags`（避开形参名，且它本就是 tombstone 标志位）、`node_level_`→`nodeLevels`（复数更准）、`FileHeader::metric`→`MetricCode`（避开 `enum class Metric` 类型名）、`elapsed_ms()`→**`ElapsedMs()`**（它是**函数**，须 PascalCase 而非 camelCase）。
  - **验证（全绿，第二轮后已全套重跑）**：MSVC x64-Release `UnitTests` **27/27**、x64-Debug **27/27**；Release `TestBeacon` 退出码 0、`count=11000` 与期望一致；`test/python` pytest **20/20**（**junitxml 核验**，0 失败 0 跳过）；`examples/rag` **14/14**（同样 junitxml 核验）；`/W4` 遮蔽扫描**生产源 + 4 个测试 + bench + Python 绑定全部 0 处 C4456/C4457/C4458**——唯二两条 C4457 在 `vcpkg` 的 `hnswlib/hnswalg.h` 里，非我方代码；我方文件的 C4267 行号（`TestBeacon.cpp:348,373`、`VectorDbTest.cpp:343,364`）与审查独立比对的 HEAD 版本**逐条一致**，属既有；自赋值/自初始化扫描 **0 命中**；字符串字面量比对**代码文件 0 改动**（仅 doc 里 `"MDBV"`→`"BEAC"` 一处故意纠正）；注释陈旧名扫描 **0 残留**；规范审计**真实偏差归零**（剩余 25 处 `top/push/pop` 属 `std::priority_queue`、2 处是保留的 Python 字符串、1 处是合法的 `namespace py` 别名）；设计文档框线表格**逐行显示宽度与每条竖线列坐标全部一致**（自写 `beacon-docwidth.py` 校验，0 处错位）。
  - **第二轮新增的一道工具纪律**：批量改名脚本改为**状态机屏蔽**——先按区间切出字符串/字符/注释并等长屏蔽，只在代码区间求替换位置，再按原偏移回写。这正是上一轮 20 处字面量误改的根治办法（此前是"事后多重集比对兜底"，能发现但发现时已改坏一版）。本轮的 `"id"`/`"score"`/`"<Hit id="` 等绑定字符串全部安然无恙即其效果。
  - **本批我自己犯的两个错，已更正并留证**：①**第三遍的蛇形正则穿透进了字符串字面量**，误改 20 处**数据**——测试产物文件名（`"ut_db.beacon"`→`"utDb.beacon"` 等 7 个）、`TestBeacon` 的 `"demo_index.beacon"`、基准程序**用户可见的表头** `"build_ms"`→`"buildMs"`、测试元数据 `"near_x"`→`"nearX"`。**我先前"字符串字面量未被误改"的结论是错的**（grep 模式写窄了），已全部还原。②`elapsed_ms` 被脚本改成 `elapsedMs`，漏了"函数须 PascalCase"。**由此新增两道闸门**：改动后必须跑「字符串字面量多重集比对」（`git show HEAD:<file>` 逐字面量对照）与「注释陈旧名扫描」。
  - **顺带修掉的两处既有缺陷**：①`README.md:129` 的 C++ 示例写成 `beacon::VectorDb db(384, beacon::Metric::kCosine);`，**改名后编译不过**，已修（`README.md:83` 的 `beacon.Metric.kCosine` 是 Python，按决策⑥保留）；②`doc/MdbVector 整体设计.txt` 里 `magic("MDBV")` 与代码实为 `constexpr char Magic[4] = {'B','E','A','C'}` 不符（**非本批引入**），已一并纠正并同步全部已改名字段名。该文档的框线表格用脚本按**显示宽度**（CJK 计 2）重排，重排算法先对 HEAD 版本自校验过"逐字节相同"才用于改内容。
  - **工具缺陷（影响其余五仓的数字，动手前须先修）**：①`style-audit.py` 的蛇形正则 `(?:_[a-z0-9]+)+` 要求下划线后仍有字符，**尾下划线标识符被整批漏计**（`db_`38、`table_`14、`dim_`、`begin_`…），须改用 `(?<![\w])([a-zA-Z][A-Za-z0-9]*(?:_[a-z0-9]+)*)_(?![\w])` —— 本批就因此漏了一整遍改动；②**撞名检测必须按作用域判定**，按"文件级标识符集合"比对会误报（`TEST(...)` 体是自由函数，fixture 成员不在其作用域内，`db_` 与 `db` 并存不构成遮蔽）；③`git diff --name-only` 对中文路径加引号，脚本须加 `-c core.quotepath=false`。
  - **第三轮（2026-09-15 收尾，提交前）：给 `Save` 修复补上回归守卫（审查「中」）** —— 原用例只证明了"读侧会跳过"，索引段照写也照样通过；将来有人把条件误改回 `if (index && ...)`，27 个测试全绿、缺陷静默回归。已在 `VectorDbTest.cpp` 的 `DirtySaveOmitsIndexSegmentThenLazyRebuild` 里加**字节级断言**：脏存文件须与"未启用索引"基线**逐字节等长**。**并实测证明守卫真的有效**（`%TEMP%\beacon-guardcheck.py`）：临时把条件改回缺陷版、重建运行，断言如实 FAILED——`FileSize(pathDirty)=151` vs `FileSize(pathNo)=119`，恰好多出 32 字节的空图索引段头（`maxNeighbors(u64)+efConstruction(u64)+enterPoint(i32)+topLevel(i32)+nodeLevels.size()(u64)`）；还原后重建复绿。
    - 过程中暴露一个我自己的低级错误：`guardcheck.py` 里对**原生** `cmd.exe` 写了 `//c`（那是 Git Bash/MSYS 的转义写法），cmd 视作无效开关而**静默不执行**，于是第一次自检报"通过"是假的——根本没重建，测的是上一个二进制。"改成 `/c`" 后立即如实失败。**教训：`//c` 只适用于 Git Bash，Python 里必须用 `/c`。**
  - **`.editorconfig` 六仓定稿（用户拍板，2026-09-15）** —— 此前我对它的认知是错的：我引"本仓行宽 100"去报了一条违规，而**那个 100 是我自己动手之前 HEAD 的旧值**；工作区其实早已被我改成 150（Spark 的 `max_line_length = 150` 同样是我加的，HEAD 里根本没有这一行）。用户指出正确值 150 后核实：`cpp-style.md:107` 的 150 管的是**函数声明/定义的参数列表**能否压成一行，而 `cpp-style.md:88` 把**行宽**整个外包给 `.editorconfig`；用户同时把 `markdown-style.md §12` 由「纯文本 ≤120、代码 ≤80」改为「纯文本 ≤150」，两处口径归一。**用户选定：全局 `max_line_length = 150` + 六仓铺同一份**。定稿与铺开脚本：`%TEMP%\editorconfig-canonical.ini`、`editorconfig-rollout.py`（按各仓既有惯例写 EOL——Beacon LF、其余五仓 CRLF；写完逐仓核验"归一化后与 canonical 逐字节一致"）。
  - **据此撤掉一处多余改动** —— 我曾据错误阈值把 `VectorDb.cpp` 那条 116 字符的 `if` 条件拆成两行；**116 ≤ 150，本不违规**，已按最小改动原则还原为单行。并实地复核：**改前改后均为 0 行超 150**（`%TEMP%\beacon-linewidth-cmp.py` 对 HEAD 与工作区逐文件比对），即本仓在真实阈值下本就达标。**格式类不纳入**的结论不变，但理由换了——不是"无法被工具拦下"，而是"本仓已达标，无需整改"；其余五仓是否同样达标尚未实测。
  - **设计文档 `has_index(u8)`→`hasIndex(u8)`**（`doc/MdbVector 整体设计.txt:45`，与代码局部名一致）。改名少 1 个显示列，用脚本按块内箭头原列坐标（47）重排空格，框线校验复跑仍 **0 处错位**。
  - **`out/` 不是 `out.txt`** —— Beacon `.gitignore` 只忽略了 `out.txt`，而 CMake 预设的构建根 `out/` 一直未忽略（`out/` 下 0 个文件被跟踪，却常挂 `?? out/`）。我这次 `/W4` 扫描产的 11 个 `.obj` 就落在 `out/w4/`。已补 `out/`。
  - **批 2–7 状态与阻塞**：**批 2（Templates，64 个模板 / 4,953 行）**——非 API 部分**可以开工**（实测 22 个模板共 190 处 `m_` **全在 `private:` 段**，另有新发现的 `t_` 23 处、`in_file` 4 处、`g_Errors` 2 处、无界 `sprintf` 5 处——后者是 Spark 生成物里 264 处 `sprintf` 与 QT/Mdb 生成物里 189 处的**唯一源头**），但 **`namespace Spark`/`mdb` 是公开 API**（`Cpp/Spark/*` 与 `Cpp/Protocol/Packages/*` 生成的正是被 Mdb/DBAdapters/QT 通过 `find_package` 消费的头），**须授权且与批 7 原子改**；**批 3（Spark，167 文件 / 15,024 行）阻塞**于公开 API 授权 + §3 确认（裸 `new` 46/`delete` 20 → 智能指针、`volatile` 6 处 → `std::atomic`）；**批 4（DBAdapters）阻塞于 B0-a**（5 个生成脚本处于未暂存删除态，且该仓**无 pumplist.xml**，其 `MdbStructs.h/.cpp` 1,105 行无可复现生成路径）；**批 5（Mdb）**手写仅 1 文件，主要靠批 2；**批 6（QT，109 文件）**阻塞于授权；**批 7（跨仓命名空间统一）**须授权，回退面最大，单独一批单独 commit。
  - **提交纪律**：**批 2 不能与批 1 同一个 commit** —— `Cpp/Mdb/*.tpl` 被 Mdb 与 QT 共用，改动落地即产生三仓生成物 churn，而**这两仓在 WSL 上构建不了**（缺 `Libs/Spark/x64-linux`，见 B0-b）。**批 1 至今未提交**（用户未要求提交）。
  - **可清理的临时件**：`%TEMP%\beacon-rename{,2,3,4}.py`、`beacon-strlit{,-revert}.py`、`beacon-stalecomment.py`、`beacon-structmember.py`、`beacon-comment-fix.py`、`beacon-doc-{repad,autosize,align,width}.py`、`beacon-w4{,b,c}.bat`、`beacon-w4sum.py`、`beacon-build.bat`、`beacon-*.xml`（pytest/RAG 的 junitxml）、以及 **`%TEMP%\beacon-replay\`、`%TEMP%\beacon-struct-bak\`**（后者是第二轮改动前 10 个文件的快照，§1 禁递归删除故未清理；**在本批提交或放弃前先别删**，它是未提交改动的唯一回退点）。
  - **本文件的体积**：批 1 条目加完后实测约 73 KB，超出 §8.1 的 50 KB 目标；**2026-09-15 已按 §8.1 滚动**——最旧三条移入 `PROGRESS-archive.md`（`D.19`/`D.18`/`D.17`），主文件剩 3 批，见下一条 ✅ 与 `## 归档索引`。


## 🔄 进行中

- **六仓 C++ 规范对齐**：批 1（Beacon）**已完成并提交**（`bd95bd4`）；批 2 前置（Mdb 生成物补齐）**已提交**（`0bb5fa6`）；**批 2a + 批 2b（Templates：有界化 / 公开成员改名 / C 风格 cast 清零 / 生成失败路径加固）已完成**，见上方 ✅ 条目。**批 2 剩余部分是 `namespace Spark`/`mdb` → PascalCase**——它生成的是被 Mdb / DBAdapters / QT 通过 `find_package` 消费的公开头，**须授权并与批 7 原子改**（见 ❓ 区①）。批 3–7 **未开工**，阻塞见 ❓ 区。**注意**：`Cpp/Mdb/*.tpl` 被 Mdb 与 QT 共用，落地即产生三仓生成物 churn，**须分仓、分批 commit**。

## ❓ 待讨论 / 待决策

- **登记待批（承自归档 `D.19`「STEP 协议数字化收口」，原文照录；均超出该批范围）**：
  - **登记待批（均超出本批范围）**：①C# 对端 `SharpLibrary/Network/StepProtocol/`（8 文件）同步；②`DAG/DAGDemo/PersonalLib/include/Protocol/StepUtility.h` 声明副本同步；③**`Package::Prepare(SessionIDType, int messageChain, int msgSeqNum)` 的 `msgSeqNum` 形参仍是有符号 `int`**——改它要动公开签名，且 QT 约 50 个调用点传的是 CTp API 的 `int requestID`；负值会静默回绕成 2^32 附近的大值（今天则存成负数），两种都原样保留调用方的位模式；④`docs/wire-protocol-revision-plan.md` 补 v3 变更记录；⑤`TailToStream` 现在与 `HeadToStream` 形状相同（同样有定长宽度校验），可以同样迁移到 `StepWriteCursor`——本批按批准的计划保留了 `snprintf`。
      - **登记待办（本批不动）**：审查提议若日后希望生产与测试**共享**版本文本，可在 `ProtocolVersion.h` 比照 `ProtocolMagicText` 增设 `constexpr std::string_view ProtocolVersionText`；当前测试本地用 `const std::string` 派生是合理替代（`std::format` 非 `constexpr`），不做也不算问题。

- **未收口 / 登记待批（承自归档 `D.18`「协议写路径收尾」，原文照录）**：**其中 ①（`GetDebugString` 无界 `sprintf`）已于 2026-09-15 批 2a 关闭、⑤（`Libs/DBAdapters` 旧副本）已随安装副本刷新而关闭、③（`Templates` 残留 C 风格 cast）已于 2026-09-15 批 2b 关闭**，下列原文保留以存历史，动手前请以本行为准：
  - **未收口 / 登记待批（均超出本批范围）**：①`GetDebugString` 的无界 `sprintf`（模板 `:311`，生成物 264 / 71 处）——写入目标是 `thread_local char t_DataStringBuffer[10240]`，`offset` 跨全包字段累加、**没有任何上界**，与上一批刚修掉的 `ToStepStream` 越界写同等级；**本批不修**是因为它的输出形态是 `Name:field:[value], …` 而非 STEP 的 `key=value\x01`，`StepWriteCursor` 装不进去，需要一个独立的「返回 `const char*` 的有界格式化」方案。②`FromXtpStream` 的读边界（模板 `:293` 的 `memcpy` 在 `while (offset < endIndex)` 只保证 1 字节时可过读，目前靠读完后 `return offset == endIndex;` 兜）——加边界判断会改变行为（原来过读、现在返回 false），属另一批。③**（已于 2026-09-15 批 2b 关闭）`Templates` 残留 C 风格 cast**：原文记作「**4 文件 5 处**」并断言 `ApiTest/ApiMiddle.cpp.tpl:155`、`ApiTest/SpiMiddle.cpp.tpl:132,152`、`Mdb/MdbStructs.cpp.tpl:168,176,183`、`KernelGen.cpp.tpl:48,58`「**现在都是 0 处**」——**这段记载本身是错的**。真实残留是 **11 处 / 8 文件**（原记载漏了 `Error/Error.cpp.tpl:12` 的 `(const char*)u8"…"`；`ApiMiddle`/`SpiMiddle`/`MdbStructs.cpp.tpl:168,176` 并非 0 处；只有 `MdbStructs.cpp.tpl:183` 与 `KernelGen.cpp.tpl:48,58` 是真的已消失）。现已全仓重扫 **0 处 / 0 文件**（`%TEMP%\tpl-ccast-rescan.py`，排除 `sizeof(double)`/`sizeof(bool)` 两处假阳性），消费方 Mdb / QuantTrading 已随重 pump 跟进。**错因**：当时是拿「计划里写过的那几处」逐条核对，**没做全仓重扫**——与批 1 那次「grep 模式写窄了」同一类错误。详见 ✅ 区批 2b 条目。④仓内手写代码 **105 处 / 24 文件**（`StepUtilityTest` 28、`PackageReaderTest` 17、`PackageSerializationTest` 11、`MD5Test` 8、`UtilityTest` 7、`TimeUtility` 5、`SingleShm` 4，其余各 1~2），另开一批。⑤`Libs/DBAdapters/x64-windows` 是 2026-08-20 旧副本、缺 `422e92a` 的 FieldType 扩展，导致 QuantTrading 全量构建在 `MdbStatic` 失败（见上）。
  - **§4 注释例外（已在代码后向用户说明的两处）**：`Package.cpp` 那 2 行（解释闸门为何必须排在指针与容量运算之前——`ToXtpStream` 是公开纯虚函数、仓外实现不保证先比容量再写，属外部契约的坑）；`StepUtilityTest.cpp` 两个 helper 上方 2 行（说明缓冲容量取 64 的理由，以及「容量边界与截断语义另有专门用例、不走这里」——防后来者误用 helper 去测边界）。

- **`GetSqlString` 删除的跨仓后果（2026-09-15，需决策）**：
  用户在批 2a 明确授权「删掉该函数」（`Templates/Cpp/Mdb/MdbStructs.cpp.tpl`）。对 `D:/Gitee/Mdb` 与 QuantTrading 是**闭包**的——两仓生成物已随模板重 pump 并各自编译通过。但仓外还有三处**不受本模板管辖**的副本（已实测）：
  - `DBAdapters/test/TestDB/MdbStructs.{h,cpp}` 与 `SAMS/Source/{HistoryDB,SyncDB}/Mdb/MdbStructs.{h,cpp}` 是**各自独立、已分叉的自带实现副本**（各自的 `.cpp` 里就有 `GetSqlString` 的定义），故**没有编译影响**；
  - `SAMS/Source/{HistoryDB,SyncDB}/{MysqlDB,SqliteDB}.cpp` 有约 **296 处**调用点（`n += (*it)->GetSqlString(m_SqlBuff + n);`）；
  - **`SAMS/Source/pumplist.xml:55` 指向一个不存在的 `../Templates/Cpp/DB/Mdb/MdbStructs.h.tpl`**（真实路径是 `Templates/Cpp/Mdb/...`，没有 `DB/` 这一层），所以 SAMS **从来不会被重 pump**——这**掩盖了分叉而不是消除它**。

  | 方向 | 代价 |
  | :--- | :--- |
  | (i) 给两处副本各补一份自有实现 | 改动落在 SAMS/DBAdapters 的冻结副本上，与模板继续脱钩 |
  | (ii) 恢复模板里的 `GetSqlString`、改由调用方有界化 | 推翻本次删除；`GetSqlString` 自身是无界的 `sprintf` 拼接，等于把安全项又还回去 |
  | (iii) 明确这两份副本为**手写冻结**、与模板脱钩并登记 | 最省；但须承认 SAMS 已不在模板治理范围内 |

- **`pumpall.py` 静默吞掉生成失败（2026-09-15 暴露，**已于批 2b 按方向 (i) 落地**）**：
  用户选定「`exit()` 改 `sys.exit(1)` 并让 `pump.py` 失败时不删目标」这一最彻底方案，三仓 `pump.py`/`pumpall.py` 均已改。**原记载里有一处失实**：旧版**并非**「失败后继续往下泵」——实测 `site.Quitter` 抛 `SystemExit(None)`、循环确实停了，旧版唯一的真实缺陷是**退出码为 0**，于是「一个模板炸了」在上层表现为「全部成功」。另外 `Mdb/src/Mdb/MdbTableRegistry.h` 那次消失的根因是 `pump.py` 失败分支**自己**在 `os.remove(out_file_name)`（不是 pumpall 继续跑），该行已删。
  - **未决部分（本批未动）**：同一缺陷在**其余 9 个仓**的副本里原样存活——`Docs`/`LibTest`/`Libs`/`Lottery`/`Python`/`SAMS`/`SAMSSharp`/`SharpLibrary`/`alg_common_gen`（其中 `alg_common_gen` **不是 git 仓**，改丢了无法回退）。是否把这套 diff 一并推送过去，或干脆把 **12 份副本收敛成单点**（`Templates/` 已是共享目录，工具链也可以），**待定**。已知现成靶子：`Docs/pumplist.xml` 的产物是**无扩展名的 JSON**、`SAMSSharp`/`Lottery` 有 `.razor`——本批新加的注释符白名单会让这两类**响亮失败**而不是静默写出坏文件。
  - 现状缓解仍在：`%TEMP%\pump-guard.py`（强制 `touch` 模板/模型后再 pump，并对产物做清点比对）。

- **六仓 C++ 规范对齐：批 2–7 的授权（2026-09-15 提出，全部未决）**：
  按 Harness §3，**公开 API 改名与高风险改动须逐仓单独取得授权**，不得援引批 1 Beacon 的授权（计划决策⑤）。批 1 的授权只覆盖 Beacon。以下五项各自独立，可以只批其中几项：
  - **①批 2 Templates 的 `namespace Spark`/`mdb` → PascalCase**：这两个命名空间生成的正是被 Mdb / DBAdapters / QT 通过 `find_package` 消费的**公开头**，改名会让三个消费方立刻编译失败。**必须与批 7 原子改、单独 commit**。`Cpp/Libs/PBApi` 另喂含已废弃 LibTest 的 Libs 仓。
  - **②批 3 Spark 的公开 API 改名**：`namespace Spark` 112 处 / 112 文件、小写访问器与方法约 1,295 处（最大头是 `length` 1,192 处，需先甄别哪些是我方方法、哪些是标准库——`std::` 一族已滤掉但跨库同名须人工确认）、C 风格 cast 134 处、`k` 前缀 221 处、`g_` 66 处、`enum CSV_PARSER_ERROR` + `CPE_*` → `enum class`。
  - **③批 3 Spark 的 §3 高风险项（须单独确认，与②可分开批）**：裸 `new` 46 处 / 裸 `delete` 20 处 → 智能指针（`LockFreeQueue.h`/`ObjectPool.h` 是分配原语内部，**按设计需保留并登记豁免**，其余可改）；`volatile` **6 处 / 2 文件** → `std::atomic`（§6 禁 `volatile` 作同步；**须先确认这 6 处是否真用于同步**——这是本批唯一的高风险项）。
  - **④批 6 QuantTrading 的公开 API 改名**：`namespace quanttrading` 87 处 / 83 文件 → `QuantTrading`（`mdb` 4 处须与 Mdb 仓同步）；`m_` 1,215 处 / 64 文件（`m_Mdb` 一个名字就 110 处）；`strcpy` 173 处 → 有界替代；裸 `new`/`delete` 81/8 → 智能指针。`namespace std` 的 `hash` 特化**不要动**，`py` 是别名**不动**。
  - **⑤B0-a（DBAdapters 的前置决策，非授权类）**：该仓 5 个生成脚本（`pump.py`/`pumpall.py`/`parseall.py`/`ParseTableModel.py`/`ParsePackageModel.py`）处于**未暂存的删除态**，且该仓**没有 `pumplist.xml`**——其生成的 `MdbStructs.h/.cpp`（1,105 行，占全仓 20%）**没有任何可复现的生成路径**。必须先定：**保留生成**（补 pumplist 并把脚本提交回来）、**转为手写**（删脚本、把 MdbStructs 标记为手写）、还是**从 Spark 复制工具链**。此决定不做，批 4 无法开工。

- **批 2b 登记待决①：生成物 `*TableList.h` 里的两个公开常量（2026-09-15，须授权）**：
  `Templates/Cpp/Mdb/ModuleTableList.h.tpl` 生成 `<module>TableIDs`（带 `k` 前缀）与 `<module>TableList` 两个**命名空间作用域的 `inline const`——即公开符号**，都违 §1「常量 PascalCase、去 `k` 前缀」。实际命中 **5 个生成头 / 10 个符号**：Mdb 的 `test/TestMdb/FullTableList.h`（`kfullTableIDs`、`fullTableList`），QT 的 `src/BackTest/BackTestTableList.h`、`src/MdOffer/MdOfferTableList.h`、`src/SimExchange/SimExchangeTableList.h`、`src/SimExchangeInit/SimExchangeTableList.h`（各一组；模块名在模板里被整体小写，故写作 `kbacktestTableIDs`/`backtestTableList` 等）。使用点合计约 37 处。**须授权**——改法只在模板一处，但会同时改 Mdb 与 QT 两仓的生成物，属公开 API 变更。**注意**：批 2b 只把同一行里的 `(int)` 换成 `static_cast<int>`，**符号名一个没碰**。

- **批 2b 登记待决②：工具链的三处既有问题（未动）**：①`pump.py`/`pumpall.py` 用 `os.system` + `%` 拼串 + 硬编码 `python`（违 `python-style.md` §7「禁 `os.system`，应用 `subprocess.run(args, shell=False)`」，也踩 Harness §6 注入防护口径）——改它会动到三仓的调用方式（`model` 是空格分隔的多文件，现在靠 cmd.exe 切分），**风险大于收益，留到工具链收敛时一起做**；②「勿手改」头注释里嵌的是**调用时原样传入的模板路径**——`pumpall.py` 传的是 pumplist 里的固定串故稳定，但手工直调会产出不同首行，制造假 `git diff`；③`pump.py` 对产物**不做任何内容体检**，`os.replace` 成功即算成功——本批已用**扩展名白名单**堵住最坏的一类（写错注释符会产出语法非法的文件而退出码仍是 0）。

- **`InitDB` 全量快照的池化对象无人归还（2026-09-15 修 Mdb 时发现，**既有缺陷**，需跨仓决策）**：
  模板生成的 `TradingDayTable::InitDB`（11 张表同型）从 `ObjectPool` 取一批对象塞进 `std::vector` 交给
  `OnRecordBatchInsert`，但**没有任何路径把它们还回池**。根因在上游 `DBAdapters`：
  `AsyncDBWriter::OnRecordBatchInsert` 把它们换进 `DBOperateImpl`，而 `DBOperateImpl::DeallocateRecord()`
  对 `Insert`/`BatchInsert`/`Truncate` 三类**提前 return**（只对 `Update`/`Delete` 归还）。
  放大效应：`Mdb::OnDBConnected` **每次重连都会重跑一遍 `InitDB`**，故池是被逐步抽干的。
  这不在"命名 + 安全"范围内，且修它要动**跨仓的所有权契约**，**未动**。
  三个可选方向：(i) 由 `DBAdapters` 侧在 `BatchInsert` 后归还（改上游契约，波及全部消费方）；
  (ii) 由 `InitDB` 侧记录并负责归还（改模板，影响 Mdb 与 QT 的生成物）；
  (iii) 接受现状并加注释说明这是有意的所有权转移（若上游确实打算长期持有）。

- **`InitDB` 在订阅者为空时静默返回、无任何日志（2026-09-15 修 Mdb 时发现，**既有**，模板级）**：
  重 pump 带出的 `if (m_MdbSubscriber == nullptr) { m_DBInited = true; return; }` 修掉了 HEAD 上的
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
  - **①（高）SAMS 处于「假安全」态**：批 2a 的 `sprintf` → `AppendDebugString` 加固与 `RiskIndex.h.tpl` 的公开成员改名**都到不了 SAMS**——`SAMS/Source/pumplist.xml:76` 指向**不存在的** `../Templates/Cpp/Risk/RiskIndex.h.tpl`（真实路径是 `Cpp/Ams/Risk/`），另有条目指向不存在的 `Cpp/Packages/`、`Cpp/DB/`。手写侧 `AccountRiskCheck.cpp:252,261,274,289,298,311,326,335,348` 有 **9 处**调用点仍写旧名。**不是「SAMS 已废弃所以没事」**——它照旧编得过，因为模板根本不在任何 pumplist 里。
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

## 备注

- 宿主必须显式调用 `Logger::Stop()` + `Join()` 收尾，否则最后一次缓冲必丢；这是进程退出时序的**定论**，不是可以靠改析构语义绕过的缺陷——见归档 `Q.17`
- P5 握手（协议版本协商）已决定**不做**，日后若要做的入口是 `Protocol::OnConnect`——见归档 `Q.18`

## 归档索引

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
