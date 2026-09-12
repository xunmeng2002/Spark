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

## 🔄 进行中

- 无

## ❓ 待讨论 / 待决策

- **设计约束：宿主必须显式调用 `Stop()`+`Join()`，否则最后一次缓冲必丢（2026-09-12 定论，非待修缺陷）**：进程退出时日志线程先被终止，任何晚于此的析构（`~Logger()` / `~ThreadBase()`）都无法补救——这是上一条实证得出的时序结论，不是可以靠改析构语义绕过的。宿主若确实无法在 `return` 前收尾，可考虑自行注册更早的收尾点（`std::atexit` 回调在 `ExitProcess` 之前执行，理论上仍能完成一次真正的 `Stop()`+`Join()`；**未实测**）。
- 单元测试用例数（36 / 8 / 14 / 6）为当前快照，用例增减后需同步更新 README，后续可考虑改为不标注具体数量以避免频繁维护
