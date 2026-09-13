# 项目进度

> 本文件用于跨会话状态记录，新会话开始请先阅读。

## ✅ 已完成

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
- 单元测试用例数（47 / 9 + 7 / 12 / 15）为当前快照，用例增减后需同步更新 README，后续可考虑改为不标注具体数量以避免频繁维护
- **Step 头上的 `Reserved` 字段暂不上线（本轮决策，待复核）**：`Head.xml` 里 `HeadField` 有 7 个字段（含 `Reserved`），但 Step 的文本包头只序列化 6 个（`HeadItemCount = 6`），`Reserved` 仅 Xtp 分支有。理由：`Reserved` 的定义是"保留字段，必须为 false"，缺省即 false，上线只会让包头多 12 字节；如果希望两端能校验"对端没乱用保留位"，需要把它加进 `HeadToStream`/`HeadFromStream` 并把 `HeadItemCount` 改成 7。
- **P5 握手（协议版本协商）未实施**：计划里本就建议**不做**——版本号已经能在第一帧的固定偏移上校验出来，握手只会把"不一致"的发现推迟到连接建立之后，且要新增一对报文。当前实现按此执行，若日后要做，入口是 `Protocol::OnConnect`。
- **生成器不认 `size`，包体越界写没有写前防护（2026-09-13 记，本批只做了事后判定）**：`ToXtpStream`/`ToStepStream` 的模板实现直接 memcpy/写文本，然后 `return int(ppos - buff)`，既不比对 `size` 也不返回负数。本批在 `MakePackage` 补的判定是事后检查：能拦住非法帧发出、能拦住 `BodyLen` 被截断，但包体真超限时序列化器已经把字节写到了缓冲之外（`Buffer<SIZE>` 的 `char m_Buffer[SIZE]` 后面就是它自己的 `m_Length`/`m_ReadPos`，溢出的破坏面是对象自身成员）。要做到写前防护，只有两条路：模板侧每写一个字段前比对剩余容量（改 `../Templates`，影响全部生成物与生成时间），或给 `ToXtpStream` 传一个带容量语义的可写游标对象。**需要用户决定是否做**；不做的前提是"没有任何模型会产出接近 64 KB 的包"，这条假设当前成立但无自动化守卫。
- **文本协议反序列化对 `uint16`/`int32` 仍走 `else: atoi`（2026-09-13 记，待决定）**：
  `Templates/Cpp/Protocol/Packages/Packages.cpp.tpl` 的整数分支是
  `elif $type in ('uint8','int8','int16','uint32','uint64')` + `else: atoi(value.c_str())`，即 `uint16s`
  与 `int32s` 这两个段**掉进 `else`**，拿不到上一批新加的 `StepUtility::ParseInteger` 范围检查。补两个标签
  进去改的是文本协议反序列化路径，且是**行为变更**（从"小值正确、超范围静默截断 / `atoi` 溢出 UB"变成
  "拒绝并报错"），需用户点头。并存项：`atoi` 的溢出 UB 对**本就走在 `else` 里**的既有族是既有问题，
  非本批引入。同一处的 `formats` 对 `uint16s` 三花脸（`%hu`/`%d`/`%u`）属纯风格，可一并收口。

## 归档索引

- `Q.15` 8 位整型是否补进类型模板（2026-09-13 关闭）
- `Q.14` Step 协议两端实现已分叉（2026-09-13 关闭）——见 `PROGRESS-archive.md`
