# 项目进度

> 本文件用于跨会话状态记录，新会话开始请先阅读。

## ✅ 已完成

- **clang-format 收敛把检查器的一个口径改坏，已修并补判别力语料（2026-09-17，紧接上一条）**：
  上一条 A 步收尾后复核工具，发现 `tools/initcheck.py` 通道 B 由 **217 掉到 211**——clang-format
  把较短的类外初始化列表并到签名行、`{` 留在次行，而旧判据要求 `:` 与 `{` 同行，于是 6 条
  初始化列表**静默脱离检查**。**这是「检查器的口径与被检对象的写法同步失效」，与上一条那三处
  同属一个失败型**：绿灯是真的，但绿灯的覆盖范围悄悄变小了——而这次连自检也是绿的（见下）。
  - **修法与两次翻车**：①初版改成「扫到第一个括号外的 `{` 就收尾」——撞上成员的 **braced-init**
    （`buff_{0}`、`sockets_{INVALID_SOCKET, INVALID_SOCKET}`、`lastConnectAttemptTime_{}`），
    实测 **9 增 3 丢**（`PackageReader` / `SocketNotify` 归零、`TcpBase` 少 3 个成员）；
    ②再改成「配对 `}` 与 `{` 同行才算 braced-init」——又被**空函数体** `{}` 骗过（clang-format
    会把短构造函数连空体收成一行，如 `Logger::Logger() : ..., logData_(nullptr) {}`），扫描
    越过函数体、把下一个函数的限定名（`Logger::GetInstance`、`TcpEpollServer::Init`）也收进
    成员表。该噪声恰好被 `judge()` 的 `nm in index_of` 挡掉而**未变成误报**——但那是下游
    擦屁股，判据本身是错的，不能因为结果碰巧对就留着。
  - **最终判据（贴合语法，不认形状）**：括号外的 `{`，左侧跳空白后紧贴标识符或 `>`/`]` 的
    是 braced-init，其余（前接 `)` 或处在行首）是函数体起点。类外定义的四种形状通吃。
  - **语料这次没能兜住，这才是真问题**：`tools/selfcheck/` 里原只有 `initpos.cpp` 一条通道 B
    语料，用的恰是**新旧判据都能认**的「`:` 独占一行」形状——故判据失效期间自检**全绿**，
    「已知正例必报」的自证成了空转。补三份：`initwrapped.cpp`（签名同行、`{` 次行）、
    `initbraced.cpp`（含 braced-init 成员）、`initsentinel.cpp`（整条压一行含空体 `{}`，
    专测**多报**）。**实测三个历史错判据各被至少一份钉住**（依次漏报 0 / 漏报 0 / 多报 2），
    缺一份就漏一类；判别力矩阵与形状对照表写进 `tools/selfcheck/README.md`。
  - **两个细节是踩出来的**：①`initsentinel.cpp` 末尾的 `Delta` 必须跨行**且非空**——跨行才能
    让越界扫描停下来（否则一路扫到文件尾判空，症状变成漏报，与真实仓的现象不是一回事），
    非空才不会被 clang-format 塌成 `{}`；②三份新语料的函数体因此都写成非空，**对 clang-format
    稳定**，判别力不再依赖目录豁免。
  - **验证**：全仓 176 文件 / 通道 A 24 / 通道 B **217** / 乱序 0（已回到格式化前的数字，
    且新判据在 `cc21352` 与当前树上的检出集合**逐条相同**，行号差异系空行增删所致）；
    自检 8 项全对（s4scan 10/0/2，initcheck 2/0/1/1/1）。
- **全仓 clang-format 收敛（选项 c 两步）完成：手写代码 173/173 合规（2026-09-17）**：
  用户裁定「执行 c」，即先把 Tab→空格 单独做一次机械归一化，再启用 clang-format。**六个提交**：
  `030f614`（109 个文件、6225 个 Tab 按制表位展开）→ `919485a`（新建 `.git-blame-ignore-revs` 登记前者）
  → `5dff1a0`（文档与 PROGRESS 登记）→ `9539b25`（第二步 A 步：142 个手写文件，空白与换行布局归一）
  → `0bd533a`（登记 A 步）→ `0ff2111`（第二步 B 步：4 个文件收尾）。
  **结果**：手写 173 个文件 `clang-format --dry-run --Werror` **全部通过**；9 个生成物与 14 个
  第三方快照按设计不纳入。
  - **拆分方式与理由**：A 步的判据是「去掉全部空白（含换行）后内容逐字相同」——142/142 通过，
    即**一个字符都没改**，故可安全登记进 `.git-blame-ignore-revs`；B 步含真实字符增删
    （`Logger.h` 的 `WriteErrorLog` 宏续行被重折、**多出一个续行反斜杠**），**不登记**，
    必须留在 blame 里可见。这正是用户要求拆两个提交的目的：不让真实改动混进纯空白提交而
    随之一并被豁免。
  - **豁免实测生效，且能跨「拆行」**：`ThreadSafeListTest.cpp` 里被 clang-format 拆开的
    `std::thread producer(` 一行，忽略前归 `9539b25`，忽略后回落到 `51d7b8c9`（紫云 2026-07-26）
    ——git 的忽略机制能跨换行位置移动重新归属，不只是越过多余空白。
  - **⚠️ 本批我连续犯了三个同类错误，都是「判据或量法偏松，而结果看起来正常」**：
    ① 用 `zip(a, b)` 逐行对位统计 churn——两侧行数不等时第一个插入点之后**每一行都被判成
    「改了」**，把 863 行夸成 **5241 行**，还凭空造出「大括号挂行」（本配置
    `BreakBeforeBraces: Allman` 下**不可能发生**，而我当时先去解释这个假象而不是怀疑量法）；
    ② 据此报出「收益仅 −2%」，真实是「被动文件 8629→863 行、−90%」，**方向完全相反**；
    ③ 拿 `git diff -w` 为空当纯空白证明，而 clang-format 会把一行拆成两行（内容同、行结构变），
    `-w` 照样报，该证明不成立；另把初始化列表 `:member` → `: member` 误记为「含非空白」，
    它其实是空白改动——**全仓真实字符变化只有 `Logger.h` 那一处**。三条教训已写入
    `docs/cpp-style-clang-format.md` §4.3。
  - **区域级归并的一处自身 bug（已修）**：初版给 `delete`/`insert` 区块特判「非纯空白、保留
    原文」，于是「删掉一个空行」这种空白改动被挡在 A 之外——实测 **55 个手写文件**的
    clang-format 改动**全部**是空行增删，全被误判。统一成一条 strip_ws 相等判据后各就各位。
  - **验证**：每个提交后均跑 WSL-GCC `ninja UnitTests` rc=0 + **391/391 全过**；B 步后逐文件
    `--dry-run --Werror` 复核 173/173 绿；A 步另有独立复查（逐文件 strip_ws 比对 HEAD、
    并确认**无任何非空行被删除**）。
  - 原 ❓⑤ 的完整原文与结论见归档 `Q.21`；量法与判据的坑见 `docs/cpp-style-clang-format.md` §4。
- **Spark §4「类内顺序」新规对齐：36 文件重排 + `.clang-format` + 检查工具入仓（2026-09-17）**：
  用户更新了 `rules/cpp-style.md` §4（相对旧版有七处 delta），要求把 Spark 仓对齐；同批把「§4 里 clang-format 能表达的部分」固化为仓内配置，并把排查用的结构扫描器纳入仓库。**已提交**：`16edbe6`（36 文件重排 + `.clang-format` + `CMakeLists.txt` 的 `-Wreorder`）、`b234cec`（用户自行收掉 `Packages.h`）、`e5a27cb`（检查脚本修复 + 自检语料入库 + `SingleShm.h` 格式化 + `.clang-format` 关 `FixNamespaceComments`）；`out/build/WSL-GCC-*` 的重新 configure 一并算在本批。
  - **规范 delta 七条**：①访问优先 `public`→`protected`→`private`，每段尽量只出现一次；②段内第 3 组扩为「特殊成员函数：构造、析构、拷贝/移动构造、拷贝/移动赋值（含 `operator=`）」，第 4 组收窄为「其他操作符重载」；③数据成员移至段末（`public` 数据成员放 `public` 末尾，**不再次打开 `public:`**）；④显式写访问标签，不依赖 `class` 默认私有；⑤标签顶格、成员缩进 4 空格、段间空行；⑥非静态数据成员声明顺序决定初始化顺序，初始化列表需一致；⑦例外：单例可将私有特殊成员置顶，但必须显式 `private:`、该区只放特殊成员、随后立即 `public:`。
  - **用户的三条裁决（本批据此执行）**：①**`struct` 不用显式 `public`**——故扫描器报出的 212 条 `struct` `NO_FIRST_LABEL` **不是违规**，一条未动；②访问段顺序的处理是**把 `protected` 那一段整体挪到 `private` 前面**，**不改任何访问级别**（我起初以为要改级别，被用户纠正）；③**`tpl` 生成的文件一律不动**（`test/Packages/Packages.h` 与仓外 `Templates/`），故该文件上报出的 188 条 `NO_BLANK`/`DUP_LABEL` 全部原样保留。
  - **改动 36 文件，四类**：①**合并重复访问段**（`DUP_LABEL`）；②**访问段顺序**——把落后于 `private` 的 `protected` 段整体前移（`ShmBase.h`/`ShmServer.h`/`SingleShm.h` 等）；③**段内组序**——数据成员移到段末（`AspectTest.cpp`、`ThreadBaseTest.cpp`、`TimerTest.cpp`，以及 `ObjectPoolTest.cpp` 里 5 个 helper 类型）；④**N6 初始化列表与声明同序** 4 处（`Protocol.cpp`/`Sem.cpp`/`TcpBase.cpp`/`ShmSubscriberImpl.cpp`）——**行为中性**，实际初始化顺序由声明顺序决定，改的只是列表书写顺序。
  - **本批的方法论：「搬行对象」而非「重打内容」。** 所有重排一律在行列表上做**切片搬运**（把现有行对象放进新位置），从不重新键入成员内容——「搬运过程吃掉某个字符」因此在结构上不可能发生。与之配套的是**断言策略必须用「整块结果等值」，不能用「出现次数」**：见下方第一处翻车。
  - **翻车一（最严重，已 `git checkout` 回退重做）：`ro5.py` 静默删代码。** `L[u + 2:d]` 在 `d < u` 时求值为**空切片**，于是 `TimerTest.cpp` 少了 `GetTimeInterval`/`GetEventCount`/`GetCurrentEventCount` 三个 getter；`ThreadBaseTest.cpp` 的 `L[:c-4] + L[c:j-1] + data + [''] + L[k:]` 跳过了 25..30 号下标，少了 `bool IsJoinable() const`。**两处的出现次数断言全都通过**——因为我断言的是「新内容出现了 N 次」，而**被删掉的旧内容一次也没被试断言过**。是逐行看 `git diff` 才发现的。改写成 `ro6.py`：断言**整段结果的逐行等值**（`assert new[d-1:d-1+len(expect)] == expect`），并给两处 `long long value;` 的搜索补起始偏移以消除歧义。
  - **翻车二：`fixn6.py` 把 `TcpBase.cpp` 的函数体 `{` 吃掉了**（`lines[i+3:]` 跳过了大括号行），同样靠 `git checkout` 回退重做。**翻车三：`TcpBase.cpp` 初始化列表尾多一个逗号**（`remoteAddressLen_(...)` 后直接跟 `{`），由 WSL GCC 构建报 `expected identifier before '{' token` 抓出。
  - **扫描器自身也翻车两次（都已修并留证）**：`n5check.py` v1 报出 **640 条假阳性**（把 `{` 紧邻首个标签这种**合法**写法当成「段间无空行」，并对缩进类体误报「未顶格」）；v2 修好假阳性后**静默返回 0 条**，根因是 `mlines[:o].count('\n')`——`o` 是 masked **字符串**的字符偏移，拿它去切**行列表**会按元素个数切、得到错误行号。改为 `masked[:o].count('\n')` 并加注释（该注释已随脚本入仓，见 `tools/s4scan.py`）。
  - **新增 `.clang-format`（仓根）**：只固化 §4 中**机械可表达**的三项——标签顶格（`AccessModifierOffset: -4`）、成员缩进 4 空格（`IndentWidth: 4` + `IndentAccessModifiers: false`）、段间空行（`EmptyLineBeforeAccessModifier: LogicalBlock`）。缩进基准取 **`UseTab: Never`**，跟随仓根 `.editorconfig` 的 `indent_style = space`。**边界是实测出来的、不是凭记忆断言的**：正例（§4 规范里的 `Example` 块）经 `clang-format` 往返后与原文**逐字一致**（据此确认 `LogicalBlock` 才是对的值，`Always` 会在 `{` 之后多补一个空行）；反例（同时含「数据成员在构造函数前 / 数据成员未置段末 / `public:` 重复 / `private:` 在 `protected:` 前 / 初始化列表逆序」五类违规）经 `clang-format` 后**零差异**——这五类它一律不管。取舍与 churn 分析见 `docs/cpp-style-clang-format.md`。
  - **新增 `tools/s4scan.py` + `tools/initcheck.py` + `tools/README.md`（本批入仓）**：把排查用的扫描器从 `%TEMP%` 提升为仓内工具。`initcheck.py` 原先硬编码 `sys.path.insert(0, r'C:/Users/15031/AppData/Local/Temp')`，改为**相对本文件的路径**；N5 的两条检查（标签顶格 / 段间空行）并入 `s4scan.py`，使一个脚本覆盖 `docs/cpp-style-clang-format.md` §3.1 表格声明的全部条款。**入仓时逐条比对过等价性**：`initcheck.py` 输出与 `%TEMP%` 原版**逐字节相同**；`s4scan.py` 输出与「原 `s4scan2.py` ∪ 原 `n5check.py`」归一化后是**同一批 400 条**（仅统一了打印格式）。
  - **判别力自检（「0 命中」不算证据，必须先证明判据能命中）**：两个脚本都做了「注入的已知正例必须报警、已知反例必须不报警」。6 类规则各造正例语料并确认全部命中（`NO_FIRST_LABEL` 对 `class` 与 `struct` 都触发，其余各 1 条以上）；反例（`class Neg` 六组齐备 + 单例 `Singleton` 走 N7 例外）**0 命中**——**N7 例外不误报**是本轮特意验的一条。`initcheck.py` 两条通道各造一处逆序，`INIT_ORDER/A` 与 `INIT_ORDER/B` 均命中；同序反例 0 命中。**语料已入库 `tools/selfcheck/`**（此前只在 `/tmp`，见下方「工具自身的假绿灯」一条），复现命令写在 `tools/README.md` §4 与该目录 README。
  - **`CMakeLists.txt` 加 `-Wreorder`（`if(NOT MSVC)`）**：§4 第 ⑥ 条另有一道编译器侧防线。**先证明这条警告在本仓能响**：造一个 `S() : b_(2), a_(1) {}` 而声明顺序 `a_` 在前的样本，带 `-Wreorder` 报 3 行、不带则**完全静默**（证明它不在默认告警集内，必须显式打开）。**MSVC 侧未启用**：对应项 `C5038` 属 `/W4` 级而本仓未开 `/W4`，单独用 `/w15038` 需在真实 Windows 工具链上实测，故留待有可用 MSVC 环境的时机再评估——已在 `CMakeLists.txt` 注释与文档里写明。
  - **验证（全绿）**：WSL-GCC-Debug 重新 configure 后 `build.ninja` 里 93 条编译规则均带 `-Wreorder`；`ninja UnitTests` rc=0、**0 warning / 0 error**；`UnitTests` **391/391 全过**；`tools/s4scan.py` 剩 **400 条候选，全部落在用户明确划出的范围外**（212 条 `struct` 的 `NO_FIRST_LABEL` + 188 条 `test/Packages/Packages.h` 生成物）；`tools/initcheck.py` **0 处乱序**（类内 24 个 + 类外 217 个初始化列表）。
  - **收尾复核（同日，用户提交后）**：后一半由用户自行收掉——`b234cec` 把 `test/Packages/Packages.h` 的 188 个包类由 `struct` 改为 `class` 并显式写 `public:`（改的是仓外模板 `Templates/Cpp/Protocol/Packages/Packages.h.tpl`，本仓 `tpl` 生成物此前按用户指示未动）。**当前全仓复核：456 个类/结构体，仅剩 24 条 `NO_FIRST_LABEL`，即全部 24 个真实 `struct` 的既定例外（用户裁决「`struct` 不用显式 `public`」）；432 个 `class` 全部通过，`Packages.h` 命中归零。**
  - **一处是我自己差点制造的假发现**：此前把 `test/TestCommon/ShmSubscriber/ShmSubscriberImpl.h:11` 的 `ShmSubscriberImpl(IoBase* io, ServerTypeType serverType)` 记为「单参数构造缺 `explicit`」——**核实后是两个无默认值的参数，§6 该条不适用**，故未登记、未改动。**教训：登记前回原文数参数，别凭「看起来像单参数」下判。**
  - **工具自身的假绿灯（用户报出，已修）：从 `tools/` 子目录运行两个脚本都报 0。** 根因是默认文件列表用 `git ls-files` 但**未固定 `cwd`**——`git` 按**当前目录**解释匹配、返回相对当前目录的路径，而 `tools/` 下没有 `.h/.cpp`，于是列表为空 → 打印「0 命中」，与「真的干净」无从区分；`--help` 被当文件名（`FileNotFoundError`）、`..` 被当文件（`PermissionError`），两者都只打印一行报错后**继续报 0**。**这正是本批反复吃亏的同一失败型**（口径与被判对象共用盲区 / 空输入冒充绿灯）。修法：`s4scan.py` 新增 `repo_root()`（`git rev-parse --show-toplevel`）与 `parse_targets()`——无参数时文件列表相对**仓库根**取（故与 cwd 无关），显式给路径时相对**当前目录**解析并递归展开目录，`-h/--help` 打印用法，**任一分支得到空列表一律 `exit 2` 响亮失败**；`initcheck.py` 复用同一解析，并新增「只给子集时通道 B 结论不作数」的告警。
  - **判别力语料入库（`tools/selfcheck/`，6 文件）**：语料此前只存在于 `/tmp`，等于这份自检**只能被当时那个人跑一次**、命令抄不动——文档里的复现步骤指向不存在的路径，是「写了但从没被跑」的典型。入库并把该目录加进 `EXCLUDE`（**无参数全仓扫描不受污染**，实测仍 176 文件 / 24 条）。期望值写在每个文件头部注释与本目录 README 里，**是跑出来核对的、不是凭记忆写的**：`pos.h` 实际 **10 条**（含我原先漏记的 `NO_BLANK` ×2），`nofirst.h` 实际 **2 条**——**`struct` 的那条也会被报出**，用户裁定的「`struct` 不必显式 `public:`」是**报告侧的既定例外**，检查器并未在判据侧排除 `struct`；我第一版注释写成「struct 不应报警」，已按实测改正。
  - **文档同步**：`tools/README.md` 的「退出码恒为 `0`」改为 0/2 两档并写明「**有发现也是 0**——条目需人工判读」；补 `-h`、子目录无关性、通道 B 子集告警、以及「先看规模行，规模为 0 或偏小时结论不可信」；§5 门禁一节补上 `.workflow/pipeline.yml` 实况（`trigger: manual`，挂了也拦不住任何东西）与取舍建议（**`initcheck.py` 适合先接**：今天 0 命中、接进去即绿；`s4scan.py` 的 `struct` 例外目前**只在报告侧建模**，未在判据侧排除）。`docs/cpp-style-clang-format.md` §3.1 指向新语料。
  - **紧随其后查出的第二个口径错误（同一失败型的反面：不是漏报，是**淹没**）**：修完 cwd 后 `python tools/s4scan.py ..` 从 `tools/` 能跑了，但报 **416 个文件 / 4573 条**——因为 `os.walk` 会走进 `out/build/*/vcpkg_installed`，那里每个 preset 都有一份第三方头文件副本。**真实结果只有 176 文件 / 24 条，其余 4573 条全是第三方头文件**。误报和漏报一样致命：4573 条噪声里没人找得出那 24 条真发现。**修法不硬编码目录名，而是取自 `.gitignore`**——一次 `git ls-files --others --ignored --exclude-standard --directory` 拿到忽略目录表（本仓 15 条），递归展开时剪掉子目录；取不到 git 时退化为「不跳过」而非静默跳过，跳过数在末行报出（`跳过 EXCLUDE 内 N 个文件`），**绝不静默**。
  - **修这个 bug 时我自己又制造了一个（已由 D 组用例抓住）**：加 EXCLUDE 过滤时把「显式点名的文件不受 EXCLUDE 约束」写进了注释和 `-h` 文本，**代码却对所有展开结果一律过滤**——于是 `tools/selfcheck/pos.h` 被自己排除、报「全部落在 EXCLUDE 内」退出 2，**语料自检整个失效**。修法是给显式点名的文件加 `pinned` 集合，`p in pinned or not any(...)`。**教训：注释里写的行为必须有一条用例去跑它**，否则注释就是谎言。
  - **语料必须挡在 clang-format 之外（否则语料会被「好心地修好」而静默失效）**：试过 `clang-format --style=file --dry-run --Werror`——6 个语料里 `pos.h` 与 `initpos.cpp` **会被改写**，且改动方向恰好是**摧毁语料本身**：`pos.h` 的两处 `NO_BLANK` 被补上空行、`namespace N` 内的 `class Indented` **被去掉缩进**（`NOT_FLUSH` 随之消失）——**10 条期望发现会静默少掉 3 条**，而自检看起来仍在正常工作，正是这套语料本该防住的那种假绿灯。已加 `tools/selfcheck/.clang-format`（`DisableFormat: true`）保护，实测 6 文件全绿、**不外溢**（`src/Network/Shm/SingleShm.h` 仍被正常标记），语料自检结果不变。**副产物**：`SingleShm.h` 当前**不是** clang-format 干净的（`#include <string>` 被标记），即全仓并不满足 `.clang-format`——这是 ❓⑤ 的既有事实，本批未动。
  - **顺带核实「语料会不会被 glob 进构建」**：**不会**。仓根 `CMakeLists.txt` 的模块只收显式目录（`include` + `src/<模块>`），测试走 `add_subdirectory(test)`，且**全仓没有任何 CMake 文件引用 `tools/`**——故把语料放在 `tools/selfcheck/` 而不是 `test/` 是刻意的：`test/` 会被构建收进去，故意写坏的代码会进编译。构建图里也确认无 `selfcheck` 字样。
  - **回归验证**：改完两个脚本后重跑全仓，与重构前**同批**（176 文件 / 456 类 / 24 条；类内 24 + 类外 217 个初始化列表 / 0 乱序）；`python tools/s4scan.py ..`（从 `tools/`）与 `python tools/s4scan.py .`（从仓根）均为 **176 文件 / 24 条**；5 条自检命令逐条与语料头部注释的期望值一致；混合给「目录 + 显式文件」得到 11 + 1 = 12 个文件、10 条（**全部落在 `pos.h` 上，`src/Network/Shm` 的 11 个文件贡献 0 条**），与两边分别跑的结果自洽。
  - **`SingleShm.h` 的收尾（`e5a27cb`）**：用户指出它不满足 `.clang-format`（`--dry-run --Werror` 会报 `<string>` 那行），已应用 `clang-format -i`——**纯空白改动**（Tab→4 空格、折叠 `#include` 后与文件尾的多余空行），证据是「格式化前 vs 格式化后」的 `diff -u` 只有空白行。**这里我一度说错**：拿 `git diff -w` 当「纯空白」的证明，而它是**对 HEAD** 比的，输出里那 1 增 5 删其实是**用户自己把 `shmName_` 从 `public` 挪到 `protected`**（真语义改动），不是格式化引入的。**改用构建兜底**：WSL-GCC `ninja UnitTests` rc=0（`SingleShm.cpp` 被重新编译，证明构建确实吃进了新头文件），**391/391 全过**。附带效果：该文件由 Tab 改为空格，与同目录仍用 Tab 的 `ShmBase.h` 暂不一致——这正是 ❓⑤ 选项 a「只对改动文件收敛」的第一步。

- **Spark 批 3 子集 · CSV 缩写规范化：`CSVxxx` → `Csvxxx`（目录/类名/枚举/内部偏差，2026-09-16，`4b9ef4d`，合并 `f13cb2f`）**：
  同 IO 批的判据（§1「缩写按普通单词处理」+ §2「文件名与类名一致」→ 三类只能同批原子改），但**证据策略必须换**：CSV 不满足等长不变式（`TOKEN_MAX_LEN`→`TokenMaxLen`、`TCSVField`→`CsvField`、`ltstr`→`CsvFieldLess` 都变长），IO 批「断言等长把偏移漂移变成不可能」那层保护在此失效。
  - **范围（9 文件，185 增 / 185 删，完全对称 = 纯改名特征）**：路径 `src/Serialization/CSV/`→`Csv/`、`CSVParserTest.cpp`→`CsvParserTest.cpp`、`CSVRecordTest.cpp`→`CsvRecordTest.cpp`（含 gtest 套件名）；公开类 `CSVParser`/`CSVRecord`→`CsvParser`/`CsvRecord`；枚举 `enum CSV_PARSER_ERROR`→`enum class CsvParserError`（**语义变化**：改为限定作用域，使用点须补 `CsvParserError::`），值 `CPE_*` 去前缀；内部偏差 `TOKEN_MAX_LEN`→`TokenMaxLen`、`CSV_RECORD_MAX_{HEAD,CONTENT}_SIZE`→`CsvRecordMax{Head,Content}Size`、`struct TCSVField`→`CsvField`、`struct ltstr`→`CsvFieldLess`、`CCSVFieldMap`→`CsvFieldMap`、成员 `m_mapCSVField`→`csvFieldMap_`；中英 README 测试表各 2 行。`CSV` 作**文件格式名**的散文与 include 路径原样保留。
  - **本批最重要的教训：自洽的 oracle 证明不了规则表完整。** 我漏了枚举**类型名** `CSV_PARSER_ERROR` 本身（只写了 4 个 `CPE_*` 枚举值），而**闸门 1d 照样报 OK**——它的判据是「拿 HEAD 原文按改名规则机械重推」，**规则表漏了，重推就一起漏**，两边一致地错。与 `PROGRESS.md:17 ⑧`「扫描口径写窄 = 假绿灯」、IO 批漏掉 `HandleIOEvent` 是同一失败型：**判据与被判对象共用同一个盲区**。根治靠加一道**独立口径**（`--scan`：直接搜残留 `CSV[A-Za-z0-9_]*`，不依赖规则表），并**先证明扫描器非空转**——拿 HEAD 原文跑，它确实报出 `CSV_PARSER_ERROR` ×4（1 定义 + 3 使用），与漏掉的那处吻合。
  - **第二处漏网，同样只有独立口径能抓：右边界断言把自己挡住了。** 残留扫描写成 `(?<![A-Za-z0-9_])(CSVParser|CSVRecord|…)(?![A-Za-z0-9_])`，末尾那个 `(?![A-Za-z0-9_])` 使 `CSVParserTest`/`CSVRecordTest` **结构上不可能匹配**，报 0 处。改成不做边界假设的 `CSV[A-Za-z0-9_]*` 全仓扫，才在两份 README 各抓到 2 处。**两次都是同一类错**：我拿「已想到的名字」造句，而漏掉的必然不在句子里。
  - **枚举定义体重写必须保 Allman**：第一版把 `enum CSV_PARSER_ERROR {` 合成一行，违 §3「括号独占一行」。改为**只替换 header 类型名与定义体里的枚举值、其余字节（含换行缩进）原样搬运**，并用 `cat -A` 核过 `{` 独占一行、枚举值 tab 缩进。
  - **注释改动是有意的，与 IO 批相反**：测试文件里 `// CSVParser 测试` 是**自指类名的标签**，不改就成陈旧注释。故本批屏蔽区间只含字符串/字符字面量，注释照改，并逐文件核验注释条目数（`CsvParserTest` 3 / `CsvRecordTest` 5，旧名 0 处）。
  - **闸门（全绿）**：脚本自测 15/15（含 `// 轻量 CSV 文件解析器` 假阳性语料、Allman 枚举定义体、`CSV_PARSER_ERROR` 类型名三条防回归语料）；闸门 1d 六文件与机械重推**逐字节全等**；`--scan` 独立口径 `CSV*` 清零；字面量多重集**零差异**（0/0/15/19/33/39）；陈旧注释 0；中英 README 正样本保留；`pumpall.py` rc=0、231 个跟踪文件逐字节零 churn、无 `.pumptmp`；`x64-Debug`/`x64-Release` 各 0 error，`UnitTests` **392/392 ×2**。
  - **重命名的目录/文件由 `GLOB_RECURSE CONFIGURE_DEPENDS` 自动收进构建**（`submodules/CMakeCommon/CMakeCommon.cmake:50,80,98,117`）→ **零 CMake 改动**；构建日志里可见 `Serialization\CsvParserTest.cpp.obj` / `CsvRecordTest.cpp.obj`。`git diff -M` 把 `CsvParserTest.cpp` 报成 delete+create 是相似度跌破 50% 阈值所致，内容本身已由闸门 1d 证明是纯改名。
  - **工具链踩坑（`%TEMP%\spark-build.py`）**：从 Git Bash 驱动 MSVC 构建有四个坑——①`cmd.exe /c` 会被 MSYS 转成 `C:/`（须 `//c`）；②`cmd.exe` **不认 `\"` 转义**（那是 C runtime 规则），故 subprocess 用 list 传参会因内层引号转义而失败；③bash heredoc/printf 中转 .bat 会被**八进制转义吃掉反斜杠**（`\2022`→0x82 `2`、`\Build`→退格 `uild`、`\v`→VT）；④最终解法是**由 python 亲自写纯 ASCII + CRLF 的 .bat 再以 list 调用**。另：不跑 `vcvars64.bat` 时 `INCLUDE` 未设，报的是 `fatal error C1083: 无法打开包括文件: "string"/"stdint.h"`——**与本批改动无关**，勿误归因。
  - **同批续做（`e26dd92`，合并 `4584d6b`）：CSV 族匈牙利前缀清零，122 处 / 4 文件，102 增 / 102 删（完全对称）**。范围是批首**未选**的那一项——形参 `psz*`、`ch[A-Z]`/`n[A-Z]` 前缀、私有成员 `chC_`/`chNC_`/`currWord_`/`curr_`、无信息名 `data_`、缩写 `itor`，一律改为承载语义的 camelCase（`pszEnd`→`stopChars`，因其语义是「终止字符集」而非某数据源；`data_`→`csvText_` 依 Harness §4「名称即意图」）。**刻意不混入 §6/§7 的既有偏差**（同文件里的 `(char *)` / `(int)` C 风格 cast、缺 `explicit`、`GetErrorCode()` 缺 `const`），故**闸门 1d 在本批可用**：从 HEAD 按规则表机械重推、与工作区逐字节全等，直接证明「除改名外无任何其他改动混入」。
  - **本批把上一批的教训变成了两条可执行的闸门**：①**两条互相独立的口径必须给出相同的逐文件命中数**——规则表机械推导（9/16/43/54）与**不依赖规则表**的前缀正则扫描（9/16/43/54），两边一致才结案；②**扫描器自己也要有自测语料**（7 条旧名正样本 + 10 条新名反样本），因为「0 命中」在报告之前**必须先证明判据本身能命中正样本**——上一批的 `CSV_PARSER_ERROR` 正是死在「口径与被判对象共用同一个盲区」。反样本里 `currentWord_`/`currentChar_` 是以 `curr` 开头的**新名**，裸 `curr[A-Za-z0-9_]*` 会把它们报成残留（假阳性），故扫描正则写作 `curr(?!ent)[A-Za-z0-9_]*`——**这是我差点自己制造的一个「口径与目标不一致」，由反样本语料挡住**。`x64-Debug`/`x64-Release` 各 0 error，`UnitTests` 392/392 ×2。
  - **本批能自洽的前提是先把口径量全**：动手前全仓扫过 `psz*` 61 处 / 4 文件、`ch[A-Z]` 24 / 4、`n[A-Z]` 12 / 2，**全部落在 CSV 这 4 个文件里**；仓内别处只剩 `b[A-Z]`（`TcpIocpBase.cpp` 3 处）、`str[A-Z]`（`MD5.cpp` 10 处）、`p[A-Z]`（`TcpIocpCompletePort.cpp` 2 处）三处疑似假阳性——故本批未牵动别的模块。测试文件里匈牙利前缀命中 **0**，也印证了「形参名不影响调用方」。
  - **同批另一笔（`ee74e59`，独立 commit）：`new char[]` 配 `delete` 的未定义行为 + 删 `#if 0` 死代码**。`CsvParser.cpp:39` 与 `CsvRecord.cpp:20-21` 共 3 处 `delete` 改 `delete[]`（与 5 处 `new char[]` 逐一核过配对），并删掉 `CsvRecord.cpp` 里调用早已不存在的 `record.Analysis(...)` 的整块 `#if 0`。**刻意不混进改名批**：它是内存管理改动（Harness §3），且改名批的证据策略要求 HEAD 基线干净——HEAD 落后两批会让差异**无法归因**。**一条教训**：这两项是我登记、**用户自己动手**改的，但用户改的是他读到的那几行，我复核时发现 `CsvParser.cpp:39` **仍漏着**（登记条目描的是 2 处，实际 3 处）——**「对方说改过了」不等于「都改过了」，复核必须逐个分配点比对，而不是只核对登记条目提到的那几行**。
  - **独立审查（`code-reviewer`，针对 `e26dd92`）判定 0 阻断项**，并用机械方式复核本批的三条声明：对 `ee74e59` 原文施加规则表后与工作区**逐字节比对 4/4 全等**；且**15 个新名在 HEAD 原文中的出现次数均为 0**——无同名标识符共存，故替换是 alpha-renaming，**语义必然保持**（这比逐行目视可靠）。它还确认了 14 条规则项在 HEAD 中**均有实际命中、无空转条目或编号遗漏**。
  - **审查指出两处「本批自己新引入」的低severity 隐患，已在 `6300a1b` 收掉**：①`SetSeparator` 的形参 `separator` 与成员 `separator_` 只差一个下划线，而 `CsvParser` 的成员是 `char separator_[2]`——若把它误写成 `separator[0] = separator`，**对算术类型取下标是合法左值表达式，能静默编译通过**，变成对形参自赋值、成员纹丝不动。原名 `chSeparator` 区分度更高，是**本批把区分度改没了**，故形参定为 `separatorChar`（两个类一致）。②`itor`→`foundField` 丢掉了「迭代器」这一类别信息——`foundField` 读起来像字段对象，实际是 `CsvFieldMap::iterator`；改为 `fieldIterator`，并把 `(*x).second` 写成 `x->second`。
  - **审查的仓外核查结论**：形参名对外**零影响**（全仓无 `STRINGIFY`/`##`/模板参数名挂钩，调用点全按位置传参，无宏包裹）；`e26dd92` 的 diff 里**没有任何 `class`/`enum` 行增删**，对外契约不变；`D:/Gitee/DAG/DAGDemo/PersonalLib/` 是**冻结的 vendored 快照**（自带一份 `CsvParser`/`CSVRecord` 与自己的 `pszData`），**不是消费方，勿误登记**。

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

## 🔄 进行中

- **六仓 C++ 规范对齐**：批 1（Beacon）**已完成并提交**（`bd95bd4`，条目已入归档 `D.20`）；批 2 前置（Mdb 生成物补齐）**已提交**（`0bb5fa6`，条目已入归档 `D.21`）；**批 2a + 批 2b（Templates：有界化 / 公开成员改名 / C 风格 cast 清零 / 生成失败路径加固）已完成**。**批 3 已落地两个子集**：IO 族 `IOxxx` → `Ioxxx`（`0c95abb` + `9e12009`）与 **CSV 族 `CSVxxx` → `Csvxxx`（目录/类名/枚举/内部偏差）+ 同族匈牙利前缀清零**（`4b9ef4d` + `f13cb2f`，续做 `e26dd92` + `4584d6b`），见上方 ✅ 条目——**此处原先写的「两者能先落地，都是因为它们不触碰任何跨仓公开契约」是错的，已由独立审查推翻**——IO 与 CSV 两族的**类名**都是 `SERIALIZATION_EXPORTS` 导出符号，`CsvRecord` 实际被 `D:/Gitee/Mdb/src/Mdb/InitMdbFromCsv.cpp`（11 处）、`D:/Gitee/QuantTrading/src/Mdb/InitMdbFromCsv.cpp`（11 处）与 `Templates/Cpp/Mdb/InitMdbFromCsv.cpp.tpl:132` 消费。它们能先落地的**正确理由**是：这些消费方此刻**本来就已经编译不过**（`InitMdbFromCsv.cpp:8-9` 写着 `using namespace spark::core;` / `using namespace spark::serialization;`，而已发货 SDK 的 `include/` 里 `namespace spark` 命中 **0 处**）——**本批不是它们的第一个断点**。跨仓跟随须按决策⑤逐仓单独授权、单独 commit，见 ❓ 区。**批 3 剩余部分仍阻塞于授权**：`namespace Spark` 112 处、小写访问器与方法约 1,295 处、C 风格 cast 134 处、`k` 前缀 221 处、`g_` 66 处，见 ❓ 区②③。**批 2 剩余部分**仍是 `namespace Spark`/`mdb` → PascalCase——它生成的是被 Mdb / DbAdapters / QT 通过 `find_package` 消费的公开头，**须授权并与批 7 原子改**（见 ❓ 区①）。批 4–7 **未开工**，阻塞见 ❓ 区。**注意**：`Cpp/Mdb/*.tpl` 被 Mdb 与 QT 共用，落地即产生三仓生成物 churn，**须分仓、分批 commit**。

## ❓ 待讨论 / 待决策

- **§4 新规对齐的收尾登记（2026-09-17）**：
  - ~~**① `src/Network/Shm/SingleShm.h` 的 `std::string shmName_;` 是 `public` 数据成员，违 §1「公有成员变量 PascalCase」**~~ **已关闭（用户自行处理）**：用户把它从 `public` 段挪进了 `protected` 段（现 `SingleShm.h:28`），不再踩「公有成员 PascalCase」，**符号名保持 `shmName_` 未改**。挪动本身正确（3 个使用点全在类内）；但那次编辑带进来两个格式偏差，已由我收掉——段间多出一个空行（21-22 行两个连续空行 → 一个）、成员缩进写成 4 空格而该文件其余全是 Tab（已改回 Tab）。顺带清掉该文件第 17 行既有的纯 Tab 尾随空白（`.editorconfig` 要求 `trim_trailing_whitespace`）。
  - ~~**② §6 表格里「禁止 `new`/`delete`」这一行的消失是遗漏**~~ **已裁定：禁令已解除**。用户确认 §6 该行不再存在是有意的，`new`/`delete` 转为 §7「智能指针」的**优先**写法（「**默认使用** `std::unique_ptr`」）而非**禁止**，并明确**批 3 不做 `new`/`delete` 这一项**。据此，本区「六仓 C++ 规范对齐：批 2–7 的授权」条目③里的「裸 `new` 46 处 / 裸 `delete` 20 处 → 智能指针」**整项撤销**，不再是待办。**代码未动。**
  - ~~**③ `volatile` 用于同步（`ShmBuffer.h:12-16`、`ThreadBase.h` 的 `shouldRun_`）**~~ **已裁定：保留，非违规**。用户给出的设计口径：这两处 `volatile` 用在**进程间共享内存**的读写上，作用是**避免编译器把值缓存在寄存器里导致不读**；同步机制本身**不靠 `volatile`**；共享内存链路每次**只有一个进程写、另一个读**（单写单读）。据此 §6「禁止 `volatile` 用于同步」在这一用法上**不适用**，上述条目③里的「`volatile` 6 处 / 2 文件 → `std::atomic`」**整项撤销**。**遗留风险（仅登记，不再作待办）**：C++ 标准对跨线程/跨进程的可见性只保证到 `std::atomic` 与内存序，`volatile` 只约束编译器不做寄存器缓存、不约束相邻访存的重排，当前实现依赖「单写单读 + 硬件缓存一致性」这一实践约定成立。若该约定日后变化（出现多写，或状态与数据分离在不同字段），需重新评估。
  - **④ 是否把 `tools/` 的检查接成 CI 门禁（未决）**：本仓 `.workflow/pipeline.yml` 只有 1 个 Build 阶段（`build@gcc`，gcc 11.1.0，`cmake -G 'Unix Makefiles' ../ && make -j2`），且 **`trigger: manual`——不自动跑**，故门禁挂了也拦不住任何东西。真要接，关键在于**扫哪些文件**：全仓扫会立刻报 24 条 `struct` `NO_FIRST_LABEL`（既定例外）→ 门禁从第一天起就是红的；只扫改动文件则可绿，但 **`initcheck.py` 的通道 B 会跨文件失效**（它要先全仓建「类名 → 成员声明顺序」表，只喂子集时表里只剩这几个文件的类，其余 `cls not in table` 直接静默 continue）。**建议分两步**：先只接 `initcheck.py`（今天 0 命中，接进去立刻有效且不误伤），`s4scan.py` 等把 `struct` 例外在检查器里显式建模之后再接。**两个前置障碍已于 2026-09-17 排除**：①脚本原本 `git ls-files` 未固定 `cwd`，在子目录下会静默报「0 命中」——CI 的工作目录不受本仓控制，这个 bug 会让门禁**永远绿**，现已改为与 cwd 无关（并新增 `-h` 与空列表 `exit 2`）；②判别力语料已入库 `tools/selfcheck/`，门禁的可信度可以随时自证，而不必「相信上次那个人跑过」。**注意 `tools/selfcheck/` 已在 `EXCLUDE` 中，CI 也不要显式扫它**——更要紧的是：**显式点名的文件按设计绕过 `EXCLUDE`**（语料自检靠的就是这个口子），所以「CI 把改动文件显式喂给脚本」这种接法，一旦某次提交动了 `tools/selfcheck/`，就会**红在故意写坏的语料上**。接门禁时须在文件筛选那步排除该路径。

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
  - **②批 3 Spark 的公开 API 改名**：`namespace Spark` 112 处 / 112 文件、小写访问器与方法约 1,295 处（最大头是 `length` 1,192 处，需先甄别哪些是我方方法、哪些是标准库——`std::` 一族已滤掉但跨库同名须人工确认）、C 风格 cast 134 处、`k` 前缀 221 处、`g_` 66 处、~~`enum CSV_PARSER_ERROR` + `CPE_*` → `enum class`~~（已于 2026-09-16 随 CSV 批落地，见 ✅ 区）。
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

- ~~**CSV 族收尾三项（2026-09-16 登记）**：①`delete` 用在 `new char[]` 上；②`CsvRecord.cpp` 的 `#if 0` 死代码；③匈牙利前缀标识符。~~ **三项已全部关闭**（`ee74e59`、`e26dd92` + `4584d6b`），落地记录与教训见 ✅ 区 CSV 条目；本行仅存删除线以防重复立项。
- **CSV 族改名的跨仓跟随（2026-09-16，需决策）**：`CSVRecord` 与 `CSV_PARSER_ERROR` 的改名**并非零外部影响**（此前记载有误，见 🔄 区更正）。实测消费方三处：`D:/Gitee/Mdb/src/Mdb/InitMdbFromCsv.cpp`（11 处 `CSVRecord csv_record;`）、`D:/Gitee/QuantTrading/src/Mdb/InitMdbFromCsv.cpp`（11 处）、`Templates/Cpp/Mdb/InitMdbFromCsv.cpp.tpl:132`（模板，喂 Mdb 与 QT 两仓）。按决策⑤**须逐仓单独授权、单独 commit**。**前置事实**：这三处**此刻都已经编译不过**（引 `namespace spark::`，而已发货 SDK 里该命名空间命中 **0 处**），故本批不是它们的第一个断点——必须先建基线，才能把「本批引入的失败」与「既有失败」分开。另：`enum CSV_PARSER_ERROR` → `enum class` 是**语义变化**（改用限定作用域），与「纯改名」不是一类，登记时须分开陈述。
- **CSV 4 文件里本批未处理的 §6/§7/§4 偏差（2026-09-16 复核新增；本批刻意未动，以保住闸门 1d 的「纯改名」证据）**：①**`CsvParser::Parse()` 重复分配不释放**——`CsvParser.cpp:13,24,34` 三处 `new char[TokenMaxLen + 1]`，而 `Parse()` 可被反复调用、每次都覆盖 `currentWord_` 却从不 `delete[]` 旧缓冲（**内存泄漏，HEAD 即存在**）；②`(char *)csvText_` 的 C 风格 cast 2 处（`CsvParser.cpp:21,30`）——`cursor_` 全程只读与自增，改成 `const char*` 后两处 cast 可直接删掉；③`(int)strlen(...)` 2 处（`CsvRecord.cpp:31,40`）与 `(int)csvFields_.size()` 1 处（`CsvRecord.h:63`）；④单参数构造 `CsvParser(const char *)` 缺 `explicit`（`CsvParser.h:20`）；⑤`GetErrorCode()` 缺 `const`（`CsvParser.h:25,39`）；⑥`*` 未贴类型（`char *GetFieldName` 与 `char* nameBuffer_` 在 `CsvRecord.h` 内并存）；⑦`CsvFieldMap` 这个类内 `typedef` 排在 `struct CsvFieldLess` 之后，违 §4「类型别名在最前」。**独立审查逐条复核后确认上述 7 项的行号与处数全部属实**，并**补出下列 11 项我漏掉的**（同一批登记，择批修；★ = 审查新增）：
  - **安全（优先）**：★`AppendNameToken`/`AppendContentToken` 的 `memcpy` **无容量校验**——向固定 1 KB 的 `CsvRecordMaxHeadSize` 累加写，头行超限即越界，且 `AnalysisFieldName` 恒 `return true`，越界后**没有任何失败路径**（`CsvRecord.cpp:33,42`）；★`atoi`/`atoll`/`atof` 无溢出与格式校验，越界即 UB、格式错静默返 0（`CsvRecord.cpp:118,128,144`，违 Harness §6「跨类型转换须 `TryParse` 风格」）。
  - **规范**：★`CsvRecord` 的 **8 个纯读取访问器全部缺 `const`**（§7，比我只记的 `GetErrorCode()` 面更大，加 `const` 对调用方**源兼容**，`CsvRecord.h:19-21,26-30,61,66,71`）；★形参 `s1`/`s2`（`CsvRecord.h:46`，`CsvFieldLess::operator()`）；★重复的 `private:` 访问标签（`CsvParser.h:26,29`；`CsvRecord.h:32,35`）；★`<stdlib.h>`/`<stdint.h>` 应为 `<cstdlib>`/`<cstdint>`（§7）；★`#include` 三组连排、组间缺空行（§2）；`tokenLen` 名不精确（是含 NUL 终止符的拷贝字节数；原名同样不准，**本批未使情况变差**）；循环下标 `i`——**Harness §4 的禁令原文限定为「公开接口的标识符」，故属可选优化而非违规，勿升级处理**。
  - **信息（不建议作为规范批处理）**：⑰`virtual ~CsvParser()`/`virtual ~CsvRecord()` 在无虚函数、无派生的类上只徒增 vtable；⑱全文件依赖 `strchr`/`strlen`/`strcmp`/`char*` 而非 `std::string`/`string_view`（§7），属**架构级重写**，须单独立项。
- **`D:/Gitee/Libs/Spark/x64-windows` 在未发指令的情况下被刷新了（2026-09-16，需用户确认）**：本会话窗口内该目录的 mtime 集中在 13:36–13:41，`include/Spark/Serialization/Csv/CsvRecord.h`（13:38）已是**新名** `class SERIALIZATION_EXPORTS CsvRecord`。**决定性判据**：记下该目录 mtime 后跑 `cmake --build x64-Debug`（结果 `ninja: no work to do`），mtime **纹丝不动**；`out/build/x64-Debug/build.ninja` 的 `default all` 里也只有 `build all: phony … Cored.dll test\all`、**不含 `install`** → **`cmake --build` 不安装，不是我这条命令链造成的**（`CMakeCommon.cmake:139/155/168` 的 POST_BUILD 钩子只往目标目录拷 DLL 与 config）。推测是并行的 IDE / 另一个会话所为。这属对外可见动作——它改变了 QT 编译时面向的头文件，且**绕过了本区 IO 族条目②已登记的「装前先把旧 `IO/` 单次 `mv` 让位」那道 NTFS 大小写陷阱处置**。**请确认是否你本人有意为之。**（大小写陷阱已自查掉一半：`python os.listdir` 读回 `include/Spark/Network/` = `['Io', 'Network.h', 'NetworkExport.h', 'Protocol']`、`include/Spark/Serialization/` = `[..., 'Csv', ...]`——**磁盘真名已是 `Io`/`Csv`，没有留下旧 `IO` 目录**，故本次刷新**未**踩到那道 NTFS 陷阱。遗留的只有「谁刷的、是否要与 QT 的消费时点对齐」这一个问题。）

- **承自归档 `D.22`（批 2a 的未决部分，2026-09-16 滚动时按 §8.1 抽出，短版）**：①`Mdb/test/TestMdb/TestDb.cpp` 的 `t_tradingDay`/`t_exchange`/`t_account` 等**局部蛇形名**属批 5；②`LibTest` 与 `SAMS` 消费了批 2a 改过的模板但**未重 pump**（一个已废弃、一个不在范围），存在语义漂移、**无编译错误**；③`m_Protocol` 8 处只声明在手写的 `QuantTrading/src/Apis/ApiBase.h`、不在任何模板里，属批 6（本区⑤另载，此处仅存互引）。**批 2a 已关闭的部分**（`sprintf` 有界化、`using namespace std;` 清零、生成物「勿手改」头、`__pycache__/` 补齐、`pumpall.py` 加固）见归档。

## 备注

- **本文件当前超出 §8.1 的 50 KB 目标约 26 KB——这是 2026-09-17 用户裁定「维持现状」的结果，不是漏做的滚动作业。** 滚动作业本身已按硬触发条件执行（最旧的 ✅ 批已入归档 `D.23`）；超标的成因是 **❓ 区自身 33.5 KB**，而 §8.1 同时要求「❓ 仅未决」与「禁止整条搬走——那等于把待办一起埋掉」。两条规则在此互相掣肘：要压到 50 KB，只能么丢掉未决待办，要么把 ✅ 区压到 3 批以下——两者都不可取，故选择接受超标。**后续会话请勿为此再动归档；若确需压缩，先向用户要新的裁定。**
- **尺寸口径（2026-09-17 复核，因原先记的「约 12 KB」已过期）**：全文 **79.8 KB**（79,806 字节）/ 45,242 字符；其中 ✅ 区 **39.1 KB**（40,005 字节）、❓ 区 **32.7 KB**（33,456 字节 / 19,461 字符）、其余为项目定位与归档索引。按 §8.1 的字面口径（KB）算超出约 **30 KB**。**引用本条时请写明用的是哪个口径**：同一份内容按 UTF-8 字节算是 79.8 KB、按字符数只有 45,242，两个口径相差约 1.8 倍，混用会得出互相矛盾的结论。上一版此处记的「按字符数只超 8.3 K 字符」是派生出来的数，本次复核复算不出它的算法（它约等于 45,242 减去 33,963，而 33,963 从何而来已无从追溯），故不再沿用，只保留两个可直接测量的原始数。本次新增两批（clang-format 收敛批 + 紧随其后的检查器口径修复批）与移除的 ❓⑤ 四行长 bullet 相抵后**净增约 3.8 KB**，增量全部落在 ✅ 区。
- 宿主必须显式调用 `Logger::Stop()` + `Join()` 收尾，否则最后一次缓冲必丢；这是进程退出时序的**定论**，不是可以靠改析构语义绕过的缺陷——见归档 `Q.17`
- P5 握手（协议版本协商）已决定**不做**，日后若要做的入口是 `Protocol::OnConnect`——见归档 `Q.18`

## 归档索引

- `D.23` 六仓 C++ 规范对齐 · 批 2b：Templates 残留 C 风格 cast 清零 + 生成失败路径加固（2026-09-15）
- `D.22` 六仓 C++ 规范对齐 · 批 2a 三项收口：模板有界化 / `using namespace std;` 清零 / 生成物「勿手改」头（2026-09-15）
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
- `Q.20` `out/build/WSL-GCC-*` 构建目录陈旧（2026-09-17 关闭：两个目录均已重新 configure）
- `Q.19` 生成器不认 `size`，包体越界写没有写前防护（2026-09-14 关闭，本次拆分时移入）
- `Q.18` P5 握手（2026-09-14 关闭，本次拆分时移入）
- `Q.17` 设计约束：宿主必须显式调用 `Stop()`+`Join()`，否则最后一次缓冲必丢（2026-09-14 关闭，本次拆分时移入）
- `Q.16` README 上的单元测试用例数是否继续标注（2026-09-14 关闭）
- `Q.15` 8 位整型是否补进类型模板（2026-09-13 关闭）
- `Q.14` Step 协议两端实现已分叉（2026-09-13 关闭）——见 `PROGRESS-archive.md`
