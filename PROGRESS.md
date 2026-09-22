# 项目进度

> 本文件用于跨会话状态记录，新会话开始请先阅读。

## ✅ 已完成

- **本区已按用户 2026-09-17 裁定清空**：原有 5 条（含 clang-format 三批与 CSV 缩写规范化）已**原文移入** `PROGRESS-archive.md`，ID 为 `D.25`–`D.29`，见下方归档索引；**2026-09-22 补一次滚动**：`Buffer` 两批新条目把本区推到 49,688 字节、距 50 KB 上限仅剩 312 字节，故把最旧的三条（2026-09-18）原文移入归档为 `D.30`–`D.32`，本区回到 5 条。**2026-09-22 再补一次批次数滚动**：批 3a / 批 3b 记入后本区回到 6 条（超出 §8.1 的 5 批），故把最旧的「2026-09-18 第二轮复查」原文移入归档为 `D.33`（用户 2026-09-22 授权：超过阈值即自行执行、不必逐次请示）。这是用户**明确选定的最小体积方案**，代价是本区不再保留「最近 3–5 批 ✅」（§8.1 的该项要求在本文件上被有意让开；**2026-09-22 起恢复按 §8.1 的 5 批上限滚动**，见本行前述三次补滚，该让开不再适用）——**需要本仓过往批次的上下文时，请先 grep 归档**。第三次即本次：批 4 记入后本区复至 6 条，故把最旧的「`RingBuffer` 重写」移入归档为 `D.34`。本区条目按时间顺序排列，**新条目追加在本区最末**。

- **2026-09-22 · `Buffer<SIZE>` 批 1（纯加固，对外签名零变化）**：①内部由「读指针 `readPos_` + 冗余 `length_`」改为**索引对** `readIndex_`/`writeIndex_`，`GetLength()` 直接由 `writeIndex_ - readIndex_` 得出，`length_` 整个删除（单一事实来源）。②**修掉一处真实越界**：旧 `GetWriteBufferSize()` 写作 `unsigned((buffer_ + SIZE) - (readPos_ + length_))`，一旦 `readPos_ + length_` 越过 `buffer_ + SIZE`，指针相减经 `unsigned` 转换**下溢成 4294967291 而不是负数**，`Append` 的裁剪随之失效、`memcpy` 堆越界写——`probe_setlength` 在 ASan 下实测复现（`Buffer.h:23` heap-buffer-overflow）。③`SetLength` 补齐**读位置到缓冲末尾**的容量守卫（`assert` 拦 Debug / `(std::min)` 钳 Release）；修前它只断言 `len <= SIZE`，`Shift` 之后传满量即越界。④`GetData`/`GetWritePos`/`GetLength`/`GetWriteBufferSize` 该 `const` 的补上；拷贝/移动四件套显式 `= delete`（对象池持有裸指针，复制即双重归还）；补 `static_assert(SIZE > 0)`。⑤**语义不变性已钉死**：`SetLength` 维持既有**绝对语义（「自读位置起 len 字节」，不是写入增量）**——另一候选解释会当场弄坏现有 `BufferTest.SetLength`，A/B 之辨留待后续批裁定。⑥单测 17 → **22 条**（每档各 22：Debug 含 `SetLength_BeyondCapacityTripsAssert` 死亡测试，Release 含 `SetLength_BeyondCapacityClampsWithoutAssert` 钳位测试，两者互斥），另补 `GetWritePos`、零长 `Append`、删除拷贝移动三类。⑦四档验证：WSL GCC Debug **425/425**、MSVC x64-Debug **426/426**、MSVC x64-Release **426/426**、WSL GCC Release 编译零告警且 BufferTest 22/22；**四档编译均 0 警告**（我一度在测试里写 `Append(nullptr, 0)`，被内联后触发 `-Wnonnull`，已改为 `""`）。⑧**更正一处旧记数**：上一批记的「MSVC Release 425」是错的——Release 不比 Debug 少用例（互斥的那两条每档各活跃一条），两档均应为 **426**。⑨`MemMove()` 经全仓 grep 确认**生产路径零调用者**（5 处调用全在 `BufferTest.cpp`），按最小改动原则本批**保留**，去留留待后续批裁定。**本批 2 个文件未提交**。

- **2026-09-22 · `Buffer<SIZE>` 批 2（`unsigned`→`size_t` 收口 + 审查修复）**：①`Append`/`SetLength`/`Shift`/`GetLength`/`GetWriteBufferSize` 与 `readIndex_`/`writeIndex_` 全量改 `size_t`，`SIZE` 模板形参同步（向 `Buffer<BufferSize>` 的 64 KiB 是**加宽**，零告警）。②日志格式串连锁修正：`TcpBase.cpp:82` 与 `TcpIocpBase.cpp` 共 **8 处** `%d`→`%zu`，其中 `:74` 是审查抓到的**真实漏改**——`WriteLogFunc`（`Logger.h:23`）是裸函数指针、无 `__attribute__((format))` 亦无 `_Printf_format_string_`，**MSVC 的 C4477 与 GCC 的 `-Wformat=2` 都不检查 `WriteLog` 调用**，故四档 0 告警与"漏改存在"并不矛盾；`%zu` 完备性只能人工枚举（已确认 10 处齐、无 `%d` 绑 `GetLength()`）。③补显式转换 9 处（`TcpIocpBase.cpp:270,304`、`TcpIocpConnect.cpp:76,82`、`Protocol.cpp:159`、`ShmBase.cpp:117,155`、`SingleShm.cpp:142`），均 `size_t`→`int/unsigned/ULONG` 且值域受 64 KiB 上限约束。④审查修复：`Shift` 丢弃分支与构造三处重复的"写同一对 0"抽成私有 `ClearIndices()`（§5）；`GetWritePos()` 补 `const` 重载；测试 4 个用例的逐字重复前置抽成 `PartiallyConsumedBufferTest` fixture + `ExpectFilledToEndOfCapacity()`（副作用：套件数 27→**28**，用例数仍 426）。⑤**两条我自己的验证论证被当场推翻，留痕备查**：(a) 我曾报"MSVC `/W3` clean rebuild 0 告警"——重跑同脚本得 **407 条**，基线实验（原始 flags→80 TU/0 告警，`/W3`→80 TU/407 告警）证明上一轮那次重编**实际跑在原始 flags 下**、`/W3` 未生效；`/W3` 真实结果是 C4251×398、C4244×6（`TimeUtility.cpp:182/188/192`、`Logger.cpp:211/219/233`）、C4018×2（`ShmBase.cpp:30/79`）、C4996×1（`TcpIocpServer.cpp:73`），**全部既有类别、0 error、无一条落在 Buffer 边界**。(b) 我曾以"`-Wformat` 零告警"证明 `%zu` 改全——因 ② 所述原因该检查**根本不生效**，论证不成立。⑥审查对 `ClientIoSubscriberImpl.cpp:87` 的归因经 `git diff` **证伪**：该文件本批只改 2 行（`:41` `%zu`、`:78` 去 `(unsigned)`），`:87` 的 `SetLength(n)` 在 HEAD 即如此，隐患**既有、非本批引入**。⑦**`SetLength` 的 A/B 之辨由此确证为 B（绝对语义）**：调用点证据是 `TcpBase::DoRecv` 在 `GetData()`（=`buffer_ + readIndex_`）处 recv 后调 `SetLength(len)`，`Protocol.cpp:115`、`ShmBase.cpp:175`、`SingleShm.cpp:148` 同型（新缓冲、写在前端、登记 len）。⑧四档：WSL GCC Debug **425/425**、WSL GCC Release 0 告警 + **425/425**、MSVC x64-Debug **426/426**、MSVC x64-Release **426/426**，四档 0 warning。

- **2026-09-22 · `Buffer<SIZE>` 批 3a（三常量搬迁 + 全仓改名）**：①`BuffSize`→`BufferSize` **移入 `include/Spark/Network/Io/Connect.h:10`**（用户选定；放 `Spark::Network` 因 `std::list<Buffer<BufferSize>*> Buffers` 本就定义于此）；`ShmBuffSize`→`ShmBufferSize` 入 `src/Network/Shm/ShmConnect.h:10`；`LogBuffSize`→`LogBufferSize` 入 `src/Core/Logger/LogData.h:10`。三者**刻意保持 `constexpr unsigned int`**：三个真实出口都吃无符号 32 位（`CreateFileMappingA` 形参 `DWORD`、`WsaBuffer.len` 为 `ULONG`、`recv` 形参 `int`），且 `ShmConnect<SIZE>`/`ShmBuffer<SIZE>` 的形参亦为 `unsigned`，**加宽会反向注入 3 处收窄转换**；三个数值逐字未变，共享内存布局与线格式兼容性零风险。②字节级改名 **84 处 / 28 文件**（口径 = 不含 `Buffer.h` 被删的 3 行常量定义；用 `(?<![A-Za-z_])` 保护 `kBufferSize`/`GetWriteBufferSize`，UTF-8 中文注释未破）。③`TcpIocpConnect.h:6` 补 `#include <Spark/Network/Io/Connect.h>`（IWYU 正确：该头 `:42,48,68` 直接拼写 `Buffer<BufferSize>`）。④**两处命名空间限定错位已修**：两个测试订阅者头（`ClientIoSubscriberImpl.h:18`、`ServerIoSubscriberImpl.h:16`）原为**全限定** `Spark::BuffSize`，改名后成 `Spark::BufferSize` 而新家在 `Spark::Network`，报 `'BufferSize' is not a member of 'Spark'`；改为 `Spark::Buffer<Spark::Network::BufferSize>`。审查确认此即该问题**全集**（`git grep "Spark::Buffer<"` 只 2 处）。⑤审查结论 **0 严重 / 2 中 / 5 低**。**M2 跨仓核查**：仓外唯一 Spark 公开头消费方 `D:/Gitee/DbAdapters` **不涉及** Buffer/`IoSubscriber`；但 `D:/Gitee/Libs/Spark/x64-windows/include/Spark/` 是**陈旧导出头快照**、仍带旧名，需人工确认其刷新是否由流水线自动完成（**未确证**）。**L1 风险登记**：`test/unittest/TemplateLib/ShmBufferTest.cpp:13` 的局部 `ShmBufferSize`（=256）与新全局量**同名不同值**，今日**无二义**（该文件只 `using namespace Spark;`，而它不引入 `Spark::Network`，也未 include `ShmConnect.h`），用户已裁定不改——日后该文件若加 `using namespace Spark::Network;` 即成二义编译错。⑥四档：**425/425、425/425（Release 0 告警）、426/426、426/426**，0 error / 0 warning。⑦**批 1、批 2、批 3a 的改动原同处一个工作区，已作为单个提交 `ae46ab2` 落地**（33 文件 / +276 −146）；审查 M1 曾建议拆分提交或改写提交信息以保住可追溯性，最终按用户指示一次提交。

- **2026-09-22 · `Buffer` 批 3b：类改名 `Buffer`→`LinearBuffer` + 文件名对齐（`Reset` 改名按用户裁定押后）**：①连带 `include/Spark/TemplateLib/Buffer/Buffer.h`→`LinearBuffer.h`、`test/unittest/TemplateLib/BufferTest.cpp`→`LinearBufferTest.cpp`（cpp-style §2「文件名与类名一致」；先例是 `939016d`）。②脚本落地（非手抄）时有两条自定规则：**(a) 保护字符串字面量**——`Package.cpp:44/49/75`、`TcpBase.cpp:82`、`TcpIocpBase.cpp:74` 这 5 处日志文本，与 `tools/step_e2e.py:84-85` 那两条必须和 `WriteLog` 原文逐字匹配的断言串，一律不动（裸词模式本会把它们改成 `LinearBuffer Too Small`，等于改日志、并可能改坏 e2e 工具）；**(b) 保护目录段**——`TemplateLib/Buffer/` 这个目录名不随类名改（`Buffer.h`/`ShmBuffer.h`/`SpscRingBuffer.h` 三个头同处该目录），做法是连目录段一起误改后统一回填，以免漏掉 `ShmBuffer.h`/`SpscRingBuffer.h` 两条。③**未撞构建系统**：头文件仅 12 处显式 include，测试改名由 `add_application` 的 `file(GLOB_RECURSE ... CONFIGURE_DEPENDS)` 自动收录——以「构建目录出现 `LinearBufferTest.cpp.o`，且 `--gtest_list_tests` 只剩 `LinearBufferTest.`、`BufferTest.` 消失」证实，并排除陈旧 `.o` 被误链（`out/` 里确实还有更早改名遗留的 `BufferTest.cpp.o`/`RingBufferTest.cpp.o`，ninja 不跟踪）。④**一处改错已回退**：`README.en.md:354` 的 `PackageReaderTest | Buffer management` 被误改——该行说的是 `PackageReader` 自带 `char buff_[MaxPackageSize * 2]` 的缓冲操作、**不涉及新类**（中文 README 同位置写「缓冲管理」未被波及，两份一度不对称），已还原；`docs/wire-protocol-revision-plan.md` 两处「接收 Buffer 泄漏」**保留**改名（指 `Protocol::OnRecv` 收到的池对象）。⑤`Reset`→`ResetWhenIdle` 押后：生产调用点只有 `TcpIocpConnect.cpp:51`/`:93`，正落在所有权有争议的 IOCP 拆链路径上，改名会给一处未必满足「仅静止态」前提的调用贴上安全标签。⑥四档 **425/425、425/425、426/426、426/426**，0 error / 0 warning。⑦**已提交 `fcf8fd7`**（34 文件 / +121 −118），两个改名（`Buffer.h`→`LinearBuffer.h`、`BufferTest.cpp`→`LinearBufferTest.cpp`）均被 git 记作真改名而非删加。⑧**同一行上的另一处既有错误措辞已订正（独立提交 `b408c50`）**：`README.md:359` / `README.en.md:360` 原写「缓冲区读写、扩容」/ `read/write and expansion`，与定长缓冲名实不符（存储是 `char buffer_[SIZE]`，`Append` 写不下即截断、`SetLength` 钳到段容量、`MemMove` 只在同数组内挪动，**无任何扩容路径**），改为「定长缓冲区读写、紧凑与空间重用」/ `Fixed-size buffer read/write, compaction and space reuse`；该措辞源自 `138d87a`（2026-07-26），与本批改名无关，故不并入本批而单独提交。⑨**后续更正（2026-09-22）**：上文⑤ 所记的 `Reset`→`ResetWhenIdle` 改名**已由用户裁定不做**，结论与理由见 ❓ 区首条——`LinearBuffer` 不声称线程安全、所有权归调用方，该后缀会暗示不存在的并发契约。本条正文保留当时的「押后」原文，此为后续裁定。

- **2026-09-22 · `Buffer` 批 4：模板形参 `SIZE`→`Size`、`Append` 形参 `data`→`source`、移除 5 条中文注释**：①非类型模板形参改名 **48 处 / 5 文件**（`LinearBuffer`、`SpscRingBuffer`、`ShmBuffer`、`ShmConnect` 四个类 + `ShmBufferTest.cpp`），依据 `rules/cpp-style.md` §1（`UPPER_CASE` 挂宏定义、常量行为 PascalCase），非类型形参按「常量/值」归属；`static_assert` 的消息串同步改（`"SIZE must be greater than 0"` → `"Size must be greater than 0"`，幂次方那条同理），免留旧形参名在诊断信息里。②**用户复核后对改名前提提出质疑、我查证后最终确认保留 `Size`**：规范 §1 确未直接规定模板形参，审查 L3 那条是从「§1 没把模板形参列进 UPPER_CASE」反推而来、**属类推而非既有明文**；业界亦无统一答案（cppbestpractices 按类型命名 PascalCase、Apache Gluten 与 Zubax 按常量命名 camelCase/snake_case、libc++ 则一律全大写 `_Np` 系，标准库自身亦不一致，见 cplusplus/draft#3642）；保留理由是 `UPPER_CASE` 在 C++ 中的语义是「预处理宏」（无作用域、无类型检查），而形参是受作用域与类型检查的真实实体、其名**不进名称修饰**，且本仓常量行本就是 PascalCase。③`LinearBuffer::Append` 形参 `data`→`source`（含函数体 `memcpy` 一处），消掉 §4 的禁用名。④删除中文注释 **5 条**：`LinearBuffer.h` 3 条（类级所有权转移约定、`SetLength` 的登记语义与负值警告、`Reset` 前置条件）、`SpscRingBuffer.h` 2 条（类级 SPSC 线程约定、`ResetWhenIdle` 前置条件）——用户裁定去注释，`rules/cpp-style.md` §4 的「注释例外」不再覆盖它们。⑤验证：MSVC x64-Debug **426/426**、x64-Release **426/426**、WSL GCC Debug **425/425**、WSL GCC Release **425/425**；全仓 `git grep` 复扫单词 `SIZE` 在 `PROGRESS*` 之外**零残留**。⑥**已提交 `07ec001`**（5 文件 / +43 −50）。**同款问题的未动项**：`ShmBuffer.h:12-16` 的 `volatile` 计数器（既有、按 §3.2 待单独授权）、`ShmBuffer.h:65/71/131/177` 与 `PackageReader.h:17` 的 `data` 禁用名（宜同类一次改齐）仍登记在 ❓ 区。

## 🔄 进行中

- **Spark §1/§6/§7 规范对齐：批 3 的 Spark 侧剩余项**：IO 族 `IOxxx` → `Ioxxx`（`0c95abb` + `9e12009`）与 CSV 族 `CSVxxx` → `Csvxxx` + 同族匈牙利前缀清零（`4b9ef4d` + `f13cb2f`，续做 `e26dd92` + `4584d6b`）两个子集**已落地并提交**，见归档 `D.24`（IO 族）与 `D.25`（CSV 族）；**剩余项待授权，见 ❓ 区**。本条目原为「六仓 C++ 规范对齐」的跨仓总账，2026-09-17 按用户指示**只保留 Spark 侧**，其余部分已从本文件删除。

## ❓ 待讨论 / 待决策

- **`ObjectPool` 并发访问偶发崩溃（2026-09-18 判定为**既有**缺陷；27.5% 实测触发率；按 Harness §3.2 已停手待授权）**：单跑 `UnitTests.exe --gtest_filter=ObjectPoolTest.MultiThreadAllocate` **40 次里 11 次以 `0xC0000005`（访问违例）退出**；全量套件 40 次里 3 次异常（2 次同一违例、1 次 `abort`；gtest 的 stdout 是块缓冲，崩溃点被缓冲吃掉，只能看到最后一个 `[ RUN ]` 行）。**该用例与 `ObjectPool` 源码与 HEAD 逐字节相同**（`test/unittest/TemplateLib/`、`include/Spark/TemplateLib/ObjectPool/` 均无 diff），**故不是本批引入**；且**单独跑比全量跑更易崩**（27.5% vs 7.5%），怀疑与冷池要先并发 `Expand` 有关，但这是推测、**未证实**。**影响面待评估**：`Allocate`/`Deallocate` 走无锁链表、只有 `Expand` 上锁，三者的交互疑有问题；而 `ObjectPool<T>::GetInstance()` 生产路径在用（`StepClient::SendReqInsertOrder` 每发一单 `Allocate` 一次）。**待决**：①是否立项修（属多线程/内存管理，须单独授权、单独评审）；②修前门禁怎么办——套件现有约 7.5% 的偶发假红，是接受、还是把该用例暂时摘出。**在授权之前，「单测全过」这句话不能当作完全可信的绿灯。**（原文记的是当日套件规模 405；2026-09-22 已增至 **426**，见 ✅ 区 2026-09-22 两条）

- **登记待批（承自归档 `D.19`「STEP 协议数字化收口」，2026-09-17 按用户指示只保留本仓项）**：`docs/wire-protocol-revision-plan.md` 补 v3 变更记录。该条其余各项均涉仓外，已删除；原文见归档 `D.19` 与本仓 git 历史 `b913108`。

- **登记待批（承自归档 `D.18`「协议写路径收尾」，2026-09-17 按用户指示只保留本仓项）**：④仓内手写代码 **105 处 / 24 文件**的 C 风格 cast（`StepUtilityTest` 28、`PackageReaderTest` 17、`PackageSerializationTest` 11、`MD5Test` 8、`UtilityTest` 7、`TimeUtility` 5、`SingleShm` 4，其余各 1~2），另开一批。该条其余各项均涉仓外模板或消费方，已删除；原文见归档 `D.18` 与本仓 git 历史 `b913108`。
  - **§4 注释例外（已在代码后向用户说明的两处）**：`Package.cpp` 那 2 行（解释闸门为何必须排在指针与容量运算之前——`ToXtpStream` 是公开纯虚函数、仓外实现不保证先比容量再写，属外部契约的坑）；`StepUtilityTest.cpp` 两个 helper 上方 2 行（说明缓冲容量取 64 的理由，以及「容量边界与截断语义另有专门用例、不走这里」——防后来者误用 helper 去测边界）。

- **`pump.py` / `pumpall.py` 的三处既有问题（2026-09-15 登记；2026-09-17 复核后补回本区）**：两个脚本就在本仓根目录（362 + 65 行）。①`os.system` + `%` 拼串 + 硬编码 `python`（违 `python-style.md` §7「禁 `os.system`」，也踩 Harness §6 注入防护口径）——改它会动到三仓的调用方式（`model` 是空格分隔的多文件，现在靠 cmd.exe 切分）；②「勿手改」头注释里嵌的是**调用时原样传入的模板路径**，手工直调会产出不同首行、制造假 `git diff`；③对产物不做内容体检，`os.replace` 成功即算成功（已用扩展名白名单堵住最坏的一类）。**注**：本条曾在 2026-09-17「删其他仓库相关内容」时被误判为跨仓工具链而删除，复核确认脚本落点在本仓后补回。

- **批 3 Spark 侧公开 API 改名的收尾（2026-09-15 提出；2026-09-17 授权已获，待排批执行）**：按 Harness §3 本须单独授权，**2026-09-17 用户裁定：API 改动没问题，项目尚未发布、仍在初期**，此后本仓公开 API 的改名不再逐项报批。**待执行项**：`namespace Spark` 112 处 / 112 文件、小写访问器与方法约 1,295 处（最大头是 `length` 1,192 处，需先甄别哪些是我方方法、哪些是标准库——`std::` 一族已滤掉，但跨库同名须人工确认）、C 风格 cast 134 处、`k` 前缀 221 处、`g_` 66 处。~~`enum CSV_PARSER_ERROR` + `CPE_*` → `enum class`~~（已于 2026-09-16 随 CSV 批落地，见归档 `D.25`）。**原条目③（裸 `new` 46 处 / 裸 `delete` 20 处 → 智能指针、`volatile` 6 处 / 2 文件 → `std::atomic`）已整项撤销**，不再是待办，裁定见归档 `Q.21`。**注**：本条原为「六仓 C++ 规范对齐：批 2–7 的授权」，2026-09-17 按用户指示**只保留 Spark 侧**，涉仓外的部分（批 2 Templates、批 6 QuantTrading、批 4 DbAdapters 前置等）已删除。**注意去向**：本条是**未决条目、从未归档**，被删的原文只存于本仓 git 历史 `b913108`；归档里的 `D.20`–`D.23` 是六仓批 1 / 批 2 前置 / 2a / 2b 的**已完成**记录，与这条不是一回事。

- **`ServerIoSubscriberImpl.cpp:37-38` 的无界 `sprintf` + 格式化串注入（2026-09-16 扫描时发现，**既有缺陷**，本批未动）**：属 §6 明令禁止项（禁止 `sprintf`；外部输入不得直接拼接构造）。虽在 `test/` 下，但它是**端到端冒烟测试唯一走的收发回调**，改它与改生产代码同样要走闸门与授权。**登记，择批修。**

- ~~**CSV 族收尾三项（2026-09-16 登记）**：①`delete` 用在 `new char[]` 上；②`CsvRecord.cpp` 的 `#if 0` 死代码；③匈牙利前缀标识符。~~ **三项已全部关闭**（`ee74e59`、`e26dd92` + `4584d6b`），落地记录与教训见归档 `D.25`；本行仅存删除线以防重复立项。

- **CSV 4 文件里本批未处理的 §6/§7/§4 偏差（2026-09-16 复核新增；本批刻意未动，以保住闸门 1d 的「纯改名」证据）**：①**`CsvParser::Parse()` 重复分配不释放**——`CsvParser.cpp:13,24,34` 三处 `new char[TokenMaxLen + 1]`，而 `Parse()` 可被反复调用、每次都覆盖 `currentWord_` 却从不 `delete[]` 旧缓冲（**内存泄漏，HEAD 即存在**）；②`(char *)csvText_` 的 C 风格 cast 2 处（`CsvParser.cpp:21,30`）——`cursor_` 全程只读与自增，改成 `const char*` 后两处 cast 可直接删掉；③`(int)strlen(...)` 2 处（`CsvRecord.cpp:31,40`）与 `(int)csvFields_.size()` 1 处（`CsvRecord.h:63`）；④单参数构造 `CsvParser(const char *)` 缺 `explicit`（`CsvParser.h:20`）；⑤`GetErrorCode()` 缺 `const`（`CsvParser.h:25,39`）；⑥`*` 未贴类型（`char *GetFieldName` 与 `char* nameBuffer_` 在 `CsvRecord.h` 内并存）；⑦`CsvFieldMap` 这个类内 `typedef` 排在 `struct CsvFieldLess` 之后，违 §4「类型别名在最前」。**独立审查逐条复核后确认上述 7 项的行号与处数全部属实**，并**补出下列 11 项我漏掉的**（同一批登记，择批修；★ = 审查新增）：
  - **安全（优先）**：★`AppendNameToken`/`AppendContentToken` 的 `memcpy` **无容量校验**——向固定 1 KB 的 `CsvRecordMaxHeadSize` 累加写，头行超限即越界，且 `AnalysisFieldName` 恒 `return true`，越界后**没有任何失败路径**（`CsvRecord.cpp:33,42`）；★`atoi`/`atoll`/`atof` 无溢出与格式校验，越界即 UB、格式错静默返 0（`CsvRecord.cpp:118,128,144`，违 Harness §6「跨类型转换须 `TryParse` 风格」）。
  - **规范**：★`CsvRecord` 的 **8 个纯读取访问器全部缺 `const`**（§7，比我只记的 `GetErrorCode()` 面更大，加 `const` 对调用方**源兼容**，`CsvRecord.h:19-21,26-30,61,66,71`）；★形参 `s1`/`s2`（`CsvRecord.h:46`，`CsvFieldLess::operator()`）；★重复的 `private:` 访问标签（`CsvParser.h:26,29`；`CsvRecord.h:32,35`）；★`<stdlib.h>`/`<stdint.h>` 应为 `<cstdlib>`/`<cstdint>`（§7）；★`#include` 三组连排、组间缺空行（§2）；`tokenLen` 名不精确（是含 NUL 终止符的拷贝字节数；原名同样不准，**本批未使情况变差**）；循环下标 `i`——**Harness §4 的禁令原文限定为「公开接口的标识符」，故属可选优化而非违规，勿升级处理**。
  - **信息（不建议作为规范批处理）**：⑰`virtual ~CsvParser()`/`virtual ~CsvRecord()` 在无虚函数、无派生的类上只徒增 vtable；⑱全文件依赖 `strchr`/`strlen`/`strcmp`/`char*` 而非 `std::string`/`string_view`（§7），属**架构级重写**，须单独立项。

- **同类宽松解析还有三类未收紧（2026-09-18 实测影响面，待决）**：`int32s`（244 处，前批）与 `int64s`（**50 处**，2026-09-18 **用户本人**改为 `ParseInteger`）均已收紧；同一个 `Packages.cpp.tpl` 里仍有 **`enums` 走 `static_cast<...Type>(atoi(...))`（123 处 / 22 个具名枚举类型）、`bools` 走 `atoi`（12 处）、`doubles` 走 `atof`（187 处 / 51 个字段名）** 在格式错或越界时静默取值。实测（本机 MSVC 18 Insiders 探针，已删）：`atoi("true") == 0`、`atoi("false") == 0`（**bool 被反向解析也不报错**）、`atoi("1.9") == 1`（静默截断）、`atoi("99999999999") == 2147483647`（静默饱和）、`atoi("13")` 对三值枚举照收不误；`atof` 对 `"abc"`/`"1.5abc"` 与**全部非有限值**均不报错。**另有一处比解析更硬的独立缺陷**：写侧 double 是 `{:.6f}`（`src/Network/Protocol/StepUtility.cpp:222-225`），**精度上限由写出侧决定、与 `atof` 无关**——实测 `1e-7` 写出即 `"0.000000"`、`10.1234567` 写出即 `"10.123457"`，且 `nan`/`inf` 会被原样写出。**待决**：是否照 L25 的同一手法一并收紧这三类；`doubles` 还得先定「小数点个数 / 非有限值 / 有效位数」的口径。

- **严格化的对端口径须与对端确认（2026-09-18 登记并**当场更正**，本批唯一的行为变更）**：写侧是 `{:04X}={:d}`（纯十进制、无正号），**自家写→自家读不受影响**。**更正**：本条初版称「对端发补位（`00123`）会被拒绝」——**该说法是错的**，`std::from_chars` 与 `strtol` 同族，**前导零照收**。实测口径已由 `StepUtilityTest` 两条新用例钉死：**接受** `0`/`123`/`00123`/`-1`/`-0` 与各宽度边界值；**拒绝** `+123`、` 123`、`123 `、`123abc`、`1.5`、`0x10`、空串、`-`、`--1`、越界（且**被拒绝时不改动出参**）。故真正的收窄只有「前导 `+` / 空白 / 尾随垃圾 / 越界」四类，且这四类在本批之前是**静默取值**、之后是**整包拒绝** + `… Out Of Range` 警告。**待决**：确认对端报文口径，再决定是否需要放宽（如先 trim 空白）。

- **❓ 区的一处非未决残留与归档的一处就地改写（2026-09-18 复核，待决）**：①❓ 区里那条「CSV 族收尾三项」已**关闭**，仅靠删除线留在原处（其自述理由是「以防重复立项」）——按 §8.1「❓ 仅未决」应予移出，但移出会失去这道防重复的提示，**故未擅动**；②`PROGRESS-archive.md` 里 `D.24` 段「刻意保留（附理由）①」一行被**用户本人就地改写**（原文 `IOType`/`IOModel` → 现文 `IoType`/`IoModel`），与 §8.1「只移动不删改」相抵；`Q.21` 的「173/173」是先例——正文保留原文、由归档索引行注明更正。**待决**：②是否照此先例在 `D.24` 索引行加注，①是否移入归档。

- **`TcpIocpBase::OnRecvComplete` 在订阅者可能已归还缓冲区之后仍操作该缓冲区（2026-09-22 代码审查发现，**既有**缺陷；按 Harness §3.2 已停手待授权）**：`OnRecvComplete`（`src/Network/Tcp/TcpIocp/TcpIocpBase.cpp:308`）把 `overlapped->MyBuffer` 交给 `ioSubscriber_->OnRecv(...)`，而生产路径的订阅者 `Protocol::OnRecv`（`src/Network/Protocol/Protocol.cpp:161`）就在其中调 `buffer->Deallocate()`，把该对象**还回了 `ObjectPool`**；`OnRecv` 返回后第 310 行紧接着 `PostRecv(overlapped)` → `MyOverlapped::Reset()`（`TcpIocpConnect.cpp:93`）→ **对同一个已归还对象**执行 `MyBuffer->Reset()`，再把 `WsaBuffer.buf` 重新指向它的 `GetData()` 去 `WSARecv`。若该槽在此期间被别的线程 `Allocate` 走，就是**跨线程覆写别人的缓冲区**——ASan 抓不到（对象仍在进程内、地址有效）。**与 HEAD 等价**：旧 `Reset()` 是 `readPos_ = buffer_`，危害一模一样，故**非本批引入**。**根因不是「订阅者乱归还」，而是 `IoSubscriber::OnRecv` 从来没有成文契约、四个后端自己分裂成两派**（2026-09-22 用户质疑后重新核对；我先前写成「订阅者契约把它当可自由归还的池对象」，那是把两派说成了一派，措辞已改准）：`TcpBase::DoRecv:177`、`ShmBase::DoRecv:173`、`SingleShm::DoRecv:146` **每次 recv 都现 `Allocate()` 一个池对象**再交出去（收方理应归还），只有 `TcpIocpBase:308` 交的是**每连接常驻**的 `MyBuffer`（收方**不得**归还）。`Protocol::OnRecv` 三条出口（147/154/161）**全部** `Deallocate()`，跟的是前三个后端的多数派，于是与 IOCP 这一派撞车；`Protocol.cpp:160` 那句注释（「Append 已经拷走字节，之后再无引用，所以在这里归还」）正是它选择那一派的书面理由。**接线取证**：`Protocol.cpp:40` 的 `ioBase_->Subscribe(this)` 加 `Protocol.h:12` 的 `class Protocol : public IoSubscriber`，证明 IOCP 路径上的 `ioSubscriber_` 就是 `Protocol` 自己——**不可与 `ClientIoSubscriberImpl` 混淆**，后者是 `TestTcpClient`/`TestShmClient` 绕过 `Protocol` 直接挂在 `IoBase` 上的订阅者（`ClientIoSubscriberImpl.cpp:15` `io_->Subscribe(this)`），它「只在主动断连时归还」的写法恰好**符合** IOCP 那一派（反过来说，它在 Select 派下每次 recv 都漏归还一个 64 KiB 池对象）。**另：IOCP 目前无任何自动化覆盖**——`TestUtility.cpp:14` 的 `IoModel = IoModelType::Select` 是全仓唯一赋值处，所有测试都跑 Select 分支。**待决**：①是否立项修（涉内存管理与跨线程，须单独授权、单独评审）；②修法取「IO 层自持收包缓冲、不借给订阅者」还是「投递 `OnRecv` 前先换上新缓冲」。

- **`Buffer` 批 4 之后的未决项（2026-09-22 第三次改写；批 1–批 4 均已落地，见 ✅ 区）**：①`Reset` 改名**已裁定不做，不再是待决项**（2026-09-22 用户两次确认：先是「Reset 没必要改名，有点啰嗦」，随后追加理由「这个 LinearBuffer 没说是线程安全的，应该由使用者自己负责」）——`WhenIdle` 后缀只在 `SpscRingBuffer` 上有真实前提（那里确有「两个索引须成对静止」的并发契约），而 `LinearBuffer` 从不声称线程安全、所有权由调用方持有，加后缀反而凭空暗示一个不存在的并发契约；**两侧命名不对称是有意的，勿为「对齐」再改**。我先前记的「须与 IOCP 那条一并定夺」随改名取消而消解——IOCP 那条本身仍是独立未决项，见本区首条。②`SetLength` 收到超大 `len` 时取「钳到末尾」（现状）还是「判为非法置 0」——两者都会让对端收到错帧，但钳位会**发出 64 KiB 的零字节**、置 0 则什么都不发；**证据链已完备**：全部 8 个调用点均无法合法超过当前段容量（`TcpBase.cpp:179` 有 `len <= 0` 守卫；IOCP 侧 `PostRecv` 前 `Reset()` 清零索引并把 `WsaBuffer.len` 置 `BufferSize`），故超大值只可能来自 bug（`ClientIoSubscriberImpl.cpp:87` 的 `sprintf` 无 `n < 0` 检查、`TcpIocpBase.cpp:304` 的 `DWORD`→`int` 收窄只滤了 `len == 0`）；若改判非法，须用**严格 `>`**（`bytesTransferred == capacity == BufferSize` 是合法可达边界），并连带重写 `LinearBufferTest.cpp` 的死亡测试与 NDEBUG 用例。**原载另两项（审查 L2 的形参 `data`、L3 的非类型模板形参 `SIZE` 大小写，原文误记为「三个类」、实为四个类）与 3+2 条中文注释的去留，已由批 4 全部处置，见 ✅ 区 2026-09-22 批 4 与提交 `07ec001`。**

## 备注

- **本文件现已达标：30.4 KB（31,155 字节），低于 §8.1 的 50 KB 目标 19.6 KB。** 2026-09-17 这一天本文件连做两次瘦身，**两次都是用户主动下的新裁定，不是滚动作业**：
  1. **删除与其他仓库相关的内容**（❓ 区 12 条整条删除、4 条裁剪为 Spark 侧）：全文 78.7 KB → 59.3 KB（60,718 字节）。删掉的是**其他仓库**的事，不为压尺寸。
  2. **把 ✅ 区 5 条全部移入归档**（`D.25`–`D.29`，从 `git HEAD` 取原文、只移动不删改）：主文件净减 37,705 字节。用户选的是**最小体积方案**，并已知其代价：✅ 区不再保留「最近 3–5 批 ✅」，**§8.1 的该项要求在本文件上被有意让开**——需要本仓过往批次的上下文时，请先 grep 归档。

  两次之间用户还曾裁定「维持现状」（当时 ❓ 区 33.5 KB，与 §8.1 的「❓ 仅未决」「禁止整条搬走——那等于把待办一起埋掉」互相掣肘，故接受超标），并嘱**后续会话勿为此再动归档**；上述两次瘦身是**用户事后的新裁定**，不违背该嘱托。**此后仍请勿自行以尺寸为由动归档**；批次数触发的滚动作业照 §8.1 正常执行（✅ 区现为空，下一批 ✅ 条目直接写进空区即可，短期内不会触发滚动）。
- **尺寸口径（2026-09-18 第六次复核；本节数字以本次为准）**：测量方法固定为「以 `## ` 标题切段、段内以 LF 拼接（段末空行计入）、每段再加一个 LF、按 UTF-8 字节计」，可复算（此口径下旧记的三个数字 89 / 545 / 4,572 可原样复现，故与历次数字逐项可比；测量脚本 `out/measure_progress.py` 未入库，口径即上文那一句）。
  - **现在**：全文 **30.4 KB**（31,155 字节）/ 17,120 字符；其中 ❓ 区 **11.0 KB**（11,283 字节，11 条）、✅ 区 8.3 KB（8,525 字节，说明行 + 3 条）、归档索引 5.3 KB（5,384 字节）、本备注 5.2 KB（5,329 字节）、🔄 区 545 字节、文件头项目定位 89 字节。**❓ 区仍是最大的一块**——它 11 条里 10 条未决，另 1 条是已关闭的删除线残留（该残留的去留本身也列为本区待决），按 §8.1 只能原地保留。
  - **同日两次瘦身的账**（三笔相加与实测逐字节吻合）：删其他仓库内容 **−20,675 字节**（全部落在 🔄 区与 ❓ 区）；✅ 区整段移出 **−38,898 字节**（含条间空行），主文件另加一条指向归档的说明 **+499 字节**，再修掉 4 处原指向已清空 ✅ 区的悬空引用 **+694 字节**，合净 **−37,705** 字节（60,718 → 23,013）。**归档文件相应由 171.3 KB 增至 209.4 KB**（175,441 → 214,376 字节）。
  - **同日第三次操作（关闭条目入归档，非瘦身）**：`§4 新规对齐的收尾登记` 的 ①②③ 与 `x64-windows` 条共 5 行原文移入归档（`Q.23`/`Q.22`），④ 升为顶层 ❓ 条，API 授权条改写为「授权已获、待排批」，并补回两条被误删的本仓待办。主文件 22,927 → 21,981 字节，归档 214,376 → 218,193 字节（+3,817）；本次属**关闭条目**，非尺寸裁定。
  - **2026-09-18 操作（关闭条目 + 新增未决，非尺寸裁定）**：七个已关闭的 ❓ 条（CI 门禁、`uint16s`/`int32s` 的 `atoi`、`ProtocolVersionValue`、Step `Reserved`、`IoFactory` 日志标签、`TcpIocpCompletePort` 类名、`.gitignore` 两处缺口）原文移入归档（`Q.24`–`Q.30`），本批 ✅ 记入 ✅ 区（由空区起头），另新增 5 条 ❓。主文件 21,981 → 22,135 字节，归档 218,193 → 228,453 字节（+10,260）。
  - **2026-09-18 操作（第二批：读路径端到端 + 事实更正，非尺寸裁定）**：✅ 区新增本批条目（+1,644 字节）；❓ 区三处改写——宽松解析条「四类」改「三类」并补入实测数字（`int64s` 已由用户收紧）、对端口径条**更正**我先前的错误断言（前导零 `00123` 是被接受的，不是被拒绝的）、模板条改记为已关闭。两笔相加 +2,701 与实测逐字节吻合，另本备注自身 +604 字节（本节数字为定稿值）。主文件 22,135 → 25,440 字节，**归档未动**（228,453 字节）。
  - **2026-09-18 操作（第三批：代码审查处置 + 关闭 2 条 ❓ 入归档，非尺寸裁定）**：✅ 区新增「审查一轮已全部处置」一条；❓ 区两条已关闭条（读路径端到端、模板仓）原文按边界脚本移入归档（`Q.31`/`Q.32`，脚本从工作区原文切块搬移、逐字节校验各出现 1 次），12 条 → 10 条；归档索引补 2 行。主文件 25,440 → 28,150 字节，归档 228,453 → 230,758 字节（归档侧另含新条目与既有各条格式对齐的增补）。
  - **2026-09-22 操作（Buffer 批 1 记入 ✅ 区 + 新增 2 条 ❓，非尺寸裁定）**：用同一口径重算并复现了两个历史锚点——`ec7c53c`（记 31,155）算得 **31,160**、`9d081d1`（记 21,981）算得 **21,987**，**均偏高 5~6 字节**（0.02% 量级，疑在该次记录之后有过零头改动或文件头计数差 1 字节：本节记「文件头 89 字节」，本次实测 90），故**新旧数字可按同一量级比对、但不逐字节可比**。**现在**：全文 **40.6 KB（41,589 字节）/ 23,111 字符**（口径 = 文件头 + 各段，段末空行计入、每段再加一个 LF）；其中 ✅ 区 **14.5 KB**（14,805 字节，说明行 + 4 条）、❓ 区 **14.1 KB**（14,444 字节，含本次新增的 IOCP 所有权条与 Buffer 后续批条）、备注 6.2 KB（6,320 字节）、归档索引 5.3 KB（5,384 字节）、🔄 区 546 字节。**归档未动**，且**仍低于 §8.1 的 50 KB 目标 9.4 KB**，未触发滚动。
  - **两个口径相差约 1.8 倍**（UTF-8 字节 vs 字符），引用时请写明用的是哪一个，混用会得出互相矛盾的结论。
  - **2026-09-22 操作（批次数滚动：✅ 区 6 → 5 条，`D.33`；非尺寸裁定）**：按 §8.1「已完成区超过 5 批即滚动」把最旧的「2026-09-18 第二轮复查」原文移入归档；本次操作合计主文件 **-488 字节**（移动 + 落账 + ✅ 区说明订正 + 归档索引，**不含本行自身**）、归档 **+1685 字节**。
  - **2026-09-22 操作（批 4 记入 ✅ 区 + 批次数滚动：✅ 区 6 → 5 条，`D.34`；非尺寸裁定）**：按 §8.1「已完成区超过 5 批即滚动」把最旧的「`RingBuffer` 重写」原文移入归档，另把 ❓ 区那条「批 3b 之后的三项待决」改写为批 4 之后的短版（②③ 与注释项已关闭，仅留 `Reset` 改名与 `SetLength` 两项）。本次操作合计主文件 **-1306 字节**（移动 + 新增批 4 条目 + 说明行订正 + ❓ 区改写 + 归档索引，**不含本行自身**）、归档 **+3998 字节**。
  - **2026-09-22 操作（更正 `Reset` 改名的错记录；非尺寸裁定）**：❓ 区原写「按用户裁定押后」，实为**已裁定不做**，已改写为已关闭项并补上用户给出的理由（`LinearBuffer` 不声称线程安全）；✅ 区批 3b 条目末尾加 ⑨ 后续更正引向。主文件 **+682 字节**（不含本行自身），归档未动。
- 宿主必须显式调用 `Logger::Stop()` + `Join()` 收尾，否则最后一次缓冲必丢；这是进程退出时序的**定论**，不是可以靠改析构语义绕过的缺陷——见归档 `Q.17`
- P5 握手（协议版本协商）已决定**不做**，日后若要做的入口是 `Protocol::OnConnect`——见归档 `Q.18`

## 归档索引

- `D.34` `RingBuffer` 重写为单调索引 SPSC（用户重写；我修缺陷 + 补用例 + 变异验证 + 改名 `SpscRingBuffer`，2026-09-22，`939016d`）
- `D.33` 同一批的第二轮复查亦已全部处置（0 严重 / 0 高 / 2 中 / 3 低）：`--seconds` 默认 40、抽 `ShutdownTestLogger()`、测试计数口径更正（2026-09-18）
- `D.32` 上一条的独立代码审查已全部处置（两轮合计 0 严重 / 2 高 / 8 中 / 10 低，2026-09-18）
- `D.31` 读路径的端到端覆盖从零补上（STEP 帧真往返，2026-09-18）
- `D.30` 七项待决一次性落地（协议整数解析收紧 + `tools/` 接进 CI 门禁 + 五处登记项关闭，2026-09-18）
- `D.29` 默认值审计与钉死：`template` 并行是配置漏写、不是工具坏；重排 16 文件并修掉它掀开的 `s4scan` 误报（2026-09-17）
- `D.28` clang-format 收敛把检查器的一个口径改坏，已修并补判别力语料（2026-09-17）
- `D.27` 全仓 clang-format 收敛（选项 c 两步）完成：手写代码 167/167 合规（2026-09-17）
- `D.26` Spark §4「类内顺序」新规对齐：36 文件重排 + `.clang-format` + 检查工具入仓（2026-09-17）
- `D.25` Spark 批 3 子集 · CSV 缩写规范化：`CSVxxx` → `Csvxxx`（目录/类名/枚举/内部偏差，2026-09-16，`4b9ef4d`，合并 `f13cb2f`）
- `D.24` Spark 批 3 子集 · IO 缩写规范化：`IOxxx` → `Ioxxx`（类名/文件名/目录名同批原子改，2026-09-16，`0c95abb`，合并 `9e12009`）
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
- `Q.32` 模板改动在 `D:\Gitee\Templates` 仓（2026-09-18 关闭：用户已在模板仓自行提交 `2ceedcd`，两仓已同步）
- `Q.31` 读路径的端到端覆盖仍是零（2026-09-18 关闭：命令行覆盖 `TestProtocol` + `tools/step_e2e.py` 真往返，两端都经解析）
- `Q.30` `.gitignore` 两处缺口（2026-09-18 关闭：补 `log/` 与 `TestShm`）
- `Q.29` `TcpIocpCompletePort` 文件名与类名不一致（2026-09-18 关闭：类更名对齐文件名，成员名刻意保留）
- `Q.28` `IoFactory.cpp:45` 日志标签 `IOType`/`IOModel`（2026-09-18 关闭：用户本人改为 `IoType`/`IoModel`）
- `Q.27` Step 头 `Reserved` 是否上线（2026-09-18 关闭：裁定不上线，`HeadItemCount` 维持 6）
- `Q.26` `ProtocolVersionValue` 是否按协议类型拆分（2026-09-18 关闭：维持共用，(i)）
- `Q.24` `tools/` 检查是否接成 CI 门禁（2026-09-18 关闭：`struct` 例外建成可配置项 + 接进 `pipeline.yml` + 排除 `tools/selfcheck/`；**内含「冒烟不走读路径」的盲区结论**）
- `Q.25` 文本协议 `uint16s`/`int32s` 走 `else: atoi`（2026-09-18 关闭：模板补 `uint16`/`int32`，重 pump 收紧 244 处；**内含「冒烟不走读路径」的盲区结论**）
- `Q.23` §4 新规对齐的收尾登记 ①②③（2026-09-17 关闭：`SingleShm.h` 公有成员 / `new`-`delete` 禁令解除 / `volatile` 用作进程间共享内存保留；**内含 `volatile` 的遗留风险说明**）
- `Q.22` `D:/Gitee/Libs/Spark/x64-windows` 被刷新（2026-09-17 关闭：用户本人多次重发，SDK 内容 = `16edbe6`，与 HEAD 语义差为零；**内含重发时必须处理的 `install(DIRECTORY)` NTFS 大小写陷阱**）
- `Q.21` clang-format 第二步（选项 b）的 churn 评估与作废的两个错数（2026-09-17 关闭，含 LCS 对位量法教训；**其正文里的「173/173」已被 `b913108` 更正为 167/167，按归档「只移动不删改」保留原文**）
- `Q.20` `out/build/WSL-GCC-*` 构建目录陈旧（2026-09-17 关闭：两个目录均已重新 configure）
- `Q.19` 生成器不认 `size`，包体越界写没有写前防护（2026-09-14 关闭，本次拆分时移入）
- `Q.18` P5 握手（2026-09-14 关闭，本次拆分时移入）
- `Q.17` 设计约束：宿主必须显式调用 `Stop()`+`Join()`，否则最后一次缓冲必丢（2026-09-14 关闭，本次拆分时移入）
- `Q.16` README 上的单元测试用例数是否继续标注（2026-09-14 关闭）
- `Q.15` 8 位整型是否补进类型模板（2026-09-13 关闭）
- `Q.14` Step 协议两端实现已分叉（2026-09-13 关闭）——见 `PROGRESS-archive.md`
