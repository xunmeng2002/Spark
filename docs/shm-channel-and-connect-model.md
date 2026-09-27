# Shm 后端的通道布局、连接对象类型与并发契约

本文记录共享内存后端（`src/Network/Shm/`、`include/Spark/TemplateLib/Buffer/ShmBuffer.h`）里四条无法从类型或命名读出的约定：
**通道在映射里的摆放、`IoBase::connects_` 里存的是什么类型、`Send` 在两个后端族下的不同完成语义、以及两端并发读写共享头的边界**。
本仓不写注释（Harness §4：命名即文档），故成文于此。与 `docs/shm-shared-object-lifecycle.md` 互补：
后者讲**对象**（命名、映射长度、残留复用、创建与销毁），本文讲**对象内部的布局与两侧的约定**。

## 一、通道布局：连接号从 1 起，0 号槽位留给头数组

- 映射长度是 `ShmBufferSize * maxConnectSize_ * 2`（推导见 `docs/shm-shared-object-lifecycle.md` 第二节）。
- **头数组紧挨映射起点**：`SingleShmHeader` 共 `maxConnectSize_` 个。0 号是**控制头**（`ShmBase::commonShmHeader_`），
  承载连接协商状态；1..`maxConnectSize_ - 1` 号是各连接的**通道头**（`ShmBuffer::shmHeader_` 指向其中自己那一个）。
- **头结构的字段顺序是跨进程 ABI，被 `ShmMappingLayoutVersion` 钉住**：7 个 4 字节字段、共 28 字节，顺序为
  `Status`、`MappingMagic`、`MappingLayoutVersion`、`UpWriteCount`、`UpReadCount`、`DownWriteCount`、`DownReadCount`。
  前三个由 0 号控制头承载映射自描述（见 `docs/shm-shared-object-lifecycle.md` 第三节），后四个是每槽位各自的读写计数器。
  `ShmBufferTest` 用 `static_assert(sizeof(SingleShmHeader) == 7 * sizeof(unsigned))` 钉住总长；
  **改字段顺序或插入字段必须同时递增 `ShmMappingLayoutVersion`**，否则新旧进程会把同一段内存解释成两种布局。
- **数据通道**：第 i 号连接占 `2 * ShmBufferSize`，偏移为 `i * 2 * ShmBufferSize`；上行在前（Client 写、Server 读）、下行紧随其后
  （`ShmBuffer::upBuffer_` / `downBuffer_`）。通道偏移只按连接号算，**不含头数组的大小**。
- 头数组与通道相加恰好占满映射，于是 **0 号连接的槽位（前 2 MiB）不用作通道**——它容纳整个头数组
  （`maxConnectSize` 个头远小于 2 MiB）。这就是 `IsValidConnectionIndex(connectionIndex >= 1)` 的由来：
  传 0 会让通道压在头数组上，传负数会被 `static_cast<size_t>` 变成一个巨大的偏移。
- **头数组装得下是编译期不变式**：头数组占映射前 `maxConnectSize_ * sizeof(SingleShmHeader)` 字节，
  必须落在 0 号槽位的 `2 * ShmBufferSize` 之内，否则会压在 1 号连接的通道上。`ShmBase.h` 的 `static_assert`
  按**最大**允许连接数 `MaxSharedMemoryConnectSize` 校验（对任何运行期连接数都更强），故调小 `ShmBufferSize`
  （例如 16 KiB）会让四档构建当场失败。现状是 `2047 * 28 = 57316` 字节 对 `2 MiB`（约 37 倍余量），
  运行期判据**永远不可达** —— 这正是它做成编译期断言而不是 `IsConnectSizeAllowed()` 里一条分支的原因。
- **两侧的连接号必须一致**：服务端在 `ShmServer::Accept` 挑第一个空闲槽位 i 并把它写进控制头的 `DownWriteCount`，
  客户端在 `ShmClient::CheckConnectResult` 从同一字段取回 i，各自用它构造 `ShmBuffer`。
  故控制头的 `DownWriteCount` 在协商期被复用为**槽位号**（不是计数器），`ShmServer::Accept` 的 5 秒超时复位也读它。
- **槽位号必须先于 `Accepted` 落盘**：服务端写槽位号与写 `Accepted` 是两次独立发布，顺序固定为
  「先 `StoreMappedField(DownWriteCount, i)`、后 `StoreStatus(Accepted)`」。客户端以 acquire 语义读到 `Accepted` 时，
  才因此保证槽位号已经随同一条 release 序列对它可见（内存序模型见第四节）。
  反过来写会让客户端读到「`Accepted` 已到、槽位号还是上一次协商的残留」，取到错误甚至越界的 i。
- **客户端取号处是信任边界，必须校验上界**：客户端手里的 i 由**对端**写入，而 `ShmBuffer` 不知道映射容量，
  上界只能在调用方校验。故 `ShmClient::CheckConnectResult` 取回 i 后先过
  `ShmBuffer<Size>::IsConnectionIndexWithinMapping(i, maxConnectSize_)`（要求 `1 <= i < maxConnectSize_`）；
  不满足即按协商失败处置 —— 记 Warning（`Reject Connect Index:%u Out Of Range`）、控制头复位成 `UnConnected`、
  走既有的 1 秒重试，**不构造 `ShmConnect`**。挡下的是三类越界后果：通道偏移落出映射
  （`upBuffer_` / `downBuffer_` 直指映射之外，构造期 `SetConnectStatus` 与首帧读写即踩）、
  `ShmClient::CheckData` 的 `sems_[i]` 越出信号量数组、以及构造期的 `SetConnectStatus` 把 `Status`
  写到头数组之外（别的槽位头或通道上）。判据本身是 `ShmBuffer` 上的 `constexpr` 静态谓词，由单元用例钉住（第五节）。
- **构造期防护**：连接号非法或基址为空时**保持「未 Attach」**（`shmHeader_` 为空），
  此后 `Write` / `Read` / `GetWriteBufferSize` / `GetReadBufferSize` 惰性返回 0，`SetConnectStatus` / `ResetSharedHeader` 不生效，
  共享内存一个字节都不动。Debug 档另有 `assert(IsValidConnectionIndex(...))` 当场拦下。
  注意这条只守**下界**（`>= 1`，且 `Release` 档没有断言兜底），上界由上一段的调用方校验负责。

## 二、连接对象的类型不变式

- `IoBase::connects_` 是 `std::map<SessionIdType, Connect*>`，**连接对象的类型在容器里被擦除**；
  shm 后端往里放的只有 `ShmConnect<ShmBufferSize>*`，注册点只有两处：`ShmServer::Accept` 与 `ShmClient::CheckConnectResult`
  （各自 `ShmConnect<ShmBufferSize>::Allocate(...)` 之后 `AddConnect`）。
- 因此 **5 处下行转换用 `static_cast`**：`ShmBase::Send` / `DoRecv`，以及 `ShmServer::CheckConnect` / `CheckData` / `HandleData`。
  这条前提由**注册点的唯一性**保证，不由类型系统保证。
- 类型断言只在**注册处做一次**（`ShmBase::AddConnect`，Debug 档 `dynamic_cast` 校验），**不在每轮 IO 周期做**：
  每周期一次虚调用，且 5 个调用点多数不判空，改 `dynamic_cast` 还得为它们补 `nullptr` 分支。
  `NDEBUG` 下该断言整个消失，Release 零开销。
- `Connect::GetNextBuffer` / `PushFront`（发送队列）只有 TCP 后端在用；shm 的 `Send` 直接写通道、从不入队，
  故 shm 后端没有「续发」这件事，`ShmBase::DoSend` 因此是一个空的 override（见第三节）。
- **`ShmBuffer` 的池往返由 `ShmConnect` 独占**（2026-09-28）：`ShmBuffer` 是装在**已安装头文件**里的模板，
  它的 `Allocate` / `Deallocate` 一旦公开，任何翻译单元都能借还 —— 而 `ObjectPool<T>::GetInstance()` 是
  头文件内联的函数局部静态量，**每个模块各有一份池实例**（与 `ShmBuffer` 批 A 立的 `D.50` 同一条陷阱）。
  故 `ShmBuffer` 现在既没有 `Allocate` 也没有 `Deallocate`：构造靠构造函数、释放走
  `ShmConnect<Size>::~ShmConnect` 里的 `ObjectPool<ShmBuffer<Size>>::GetInstance().Deallocate(shmBuffer_)`。
  借还两侧都在 `src/Network/Shm/ShmConnect.h`，与池实例同源。单元用例用一个 concept 断言
  「`ShmBuffer` 不满足池往返的形状」（第五节）。

## 三、`Send` 的完成语义：Shm 同步写穿，Tcp 异步排队

`IoBase::Send` 与 `IoBase::DoSend` 是两件事：`Send` 是应用侧的投递入口，每个后端族各自实现；
`DoSend` 是「把上一次没发完的缓冲续发出去」这个**可选钩子**——`IoBase` 把它声明为纯虚，由各后端自己给出实现：
会「发不完」的后端（Tcp 族）在这里排空发送队列，不入队的后端（Shm 族）给一个空实现。
两个后端族给同一个 `Send` 的是**同名不同完成语义**：

| 事项 | Tcp 族 | Shm 族 |
| ---- | ---- | ---- |
| `Send` 做什么 | `PushBack` + `socketNotify_->Notify()`，**立即返回** | 在调用方线程里写通道，写不完每 1 ms 重试 |
| 谁做搬运 | IO 线程，稍后由 `DoSend` 排空 | 调用方线程，此刻 |
| 覆盖 `DoSend` | 是（`TcpBase`，排空发送队列） | 是（`ShmBase`，**空实现**——Shm 从不入队） |
| 对端不消费时 | 发送队列无上界增长（吃 `LinearBuffer` 池） | 调用方线程一直等，**只有对端 `DisConnected` 才丢弃** |
| 缓冲上界 | 无（池多大就能堆多少） | 固定每方向 1 MiB（`ShmBufferSize`） |

三条后果，调用方须按后端区分假设：

- **`Send` 在 Shm 上会阻塞**：对端活着但不读时，`ShmBase::Send` 的 1 ms 重试循环没有截止时间，
  会一直转（对端断连时才丢弃并记 Warning）。照 Tcp 语义假定「`Send` 立即返回」的调用方会在 Shm 上挂住。
- **Shm 的背压是硬的**：1 MiB 通道写满即挡住调用方，不会像 Tcp 那样把内存堆上去——这正是共享内存窗口的意义。
- **「通道写满」这条分支至今没有用例**：1 MiB 通道在冒烟的 10000 次往返里都难填满，
  正确性依赖 `ShmBuffer::Write` 的返 0 语义（由 `ShmBufferTest` 钉住）。

**为什么不给 Shm 也做队列**（用户 2026-09-27 裁定：维持现状，把差异写进文档）：那要给 Shm 造一个它现在没有的原语
——「唤醒本地 IO 线程」（Tcp 侧是 `socketNotify_`）；不造它、只靠 IO 循环轮询排空，就是给**每条消息**加上一个轮询周期的延迟。
更关键的是背压会从「固定 1 MiB 窗口」变成「无上界排队」，等于拿掉共享内存窗口最核心的性质。故保留同步直写。

**`ShmBase::DoSend` 是空实现**（用户 2026-09-28 裁定）。它原先是一份与 `TcpBase::DoSend` 同构、
却在 Shm 侧永远不会被调用的续发实现：2026-09-27 曾按「留着容易误解」把它删除，而删除的前提是
把 `IoBase::DoSend` 由纯虚降为带空实现的虚函数。用户 2026-09-28 判该降级为**对 `IoBase` 接口的损伤**，
故反转——`IoBase::DoSend` 恢复纯虚，`ShmBase::DoSend` 保留为**空实现**：
「Shm 不做续发」这件事由这里的空实现表达，接口层不为任何一个后端放宽。

## 四、并发契约

- **每个计数器单写单读**：`UpWriteCount` 由 Client 写、`UpReadCount` 由 Server 写、`DownWriteCount` 由 Server 写、
  `DownReadCount` 由 Client 写（对应上文「上行 Client 写、下行 Server 写」的通道分工）。
- **`Status` 是唯一的双写字段**：两端各自 `SetConnectStatus`，也各自走断连仲裁。过渡顺序是
  「先释放者置位 `DisConnected`、后释放者清零」：`MarkDisconnectedAndReportWhetherLastHolder()` 返回 true 表示
  「调用前已是 `DisConnected`」= 对端先释放、本进程这份 `ShmBuffer` 是最后持有者，**只有此时才 `ResetSharedHeader()`**。
  该仲裁不靠锁，靠 `Status` 上的**一次 CAS 循环**：`compare_exchange_weak(expected, DisConnected, acq_rel, acquire)`
  在 `expected != DisConnected` 时反复重试，成功即「本次置位」（返 false），失败时 `expected` 已被刷新为
  `DisConnected`、循环退出即「对端先置位」（返 true）。`StatusReference` 交出 `std::atomic_ref` 供这一步使用。
- 槽位回收路径：`RemoveConnect` → `Connect::Deallocate` → `ShmConnect<Size>::~ShmConnect` →
  `ShmBuffer::MarkDisconnectedAndResetChannelWhenLastHolder()`（上面的仲裁，最后一任持有者顺手 `ResetSharedHeader()`
  把该槽位头清零成 `UnConnected`）→ `ObjectPool<ShmBuffer<Size>>::Deallocate(shmBuffer_)`。
  服务端协商超时那条路径不在其上，它直接对超时槽位调 `SingleShmHeader::ResetChannelHeader`（不经过 `ShmConnect`）。
- **内存序：`std::atomic_ref` 取代 `volatile` + 手写屏障**（2026-09-28 改动，**取代**归档 `Q.23` 的裁定；原裁定原文与
  登记背景仍在归档里，此处不重写它，只声明其结论已被本次改动覆盖）。`SingleShmHeader` 的全部字段访问统一走
  `LoadStatus` / `StoreStatus` / `LoadMappedField` / `StoreMappedField` 四个静态入口，读一律 `acquire`、写一律 `release`：
  - 通道搬运的发布顺序因此有了**语言级**保证：`WriteIntoChannel` / `ReadFromChannel` 先以 acquire 读入两个计数器、
    搬运完数据、再以 release 写回新计数。对端以 acquire 读计数时，必然看到与之配套的那段数据（`memcpy` 不会越过 release）。
  - `Status` 是唯一的双写字段，写点全部走 CAS 或 release store；读点全部 acquire。
  - 计数器的读入从「同一个操作里读两次」收敛成「读一次、复用到返回值与断言」，顺带消掉了两次读之间可能被对端插进来的窗口。
  - `std::atomic_ref<T>::is_always_lock_free` 对 `ConnectStatusType`（`int32_t`）与 `unsigned` 都是编译期断言：
    若某平台上共享内存里的 4 字节访问不是无锁的，四档构建当场失败。这是**故意的**——共享内存里塞一把锁没有意义。
  - `volatile` 已从 `ShmBuffer.h` 与相关调用点全部移除；Harness §6 / `rules/cpp-style.md` §6 都禁止用 `volatile` 做同步，
    此前是依赖归档 `Q.23` 的实践约定豁免，现在不再需要豁免。

## 五、相关测试与输入输出

`test/unittest/TemplateLib/ShmBufferTest.cpp`（四档构建均跑）。

- `ConnectionIndexZeroTripsAssert`（仅 Debug 且启用 death test）
  - 输入：连接号 0。
  - 输出：断言命中（`IsValidConnectionIndex`）。
- `InvalidConnectionIndexLeavesBufferDetached`（仅 Release，`NDEBUG`）
  - 输入：连接号 0 与 -1。
  - 输出：`GetShmHeader() == nullptr`、`Write` 返 0、`GetWriteBufferSize()` 为 0，且共享内存首字节的状态仍为 `UnConnected`。
- `NullSharedMemoryBaseLeavesBufferDetached`（仅 Release，`NDEBUG`）
  - 输入：基址 `nullptr`、连接号 1。
  - 输出：同上「未 Attach」。
- `ConnectionIndexWithinMappingRequiresBothBounds`（四档构建均跑）
  - 输入：`(1, 4)`、`(3, 4)`、`(0, 4)`、`(4, 4)`、`(0xFFFFFFFF, 4)`、`(1, 1)`。
  - 输出：前两例 `true`（`1 <= index < 4`）；后四例 `false`（0 号留给头数组、`index == 连接数` 即越界、
    对端写下的 `0xFFFFFFFF` 同样被拒、连接数 1 时不存在可用的数据连接号）。
- `MarkDisconnectedAndReportWhetherLastHolder_ReportsLastOwner`
  - 输入：同一份 header 上先后两次调用。
  - 输出：第一次 `false`（由本次置位）、第二次 `true`（已是最后持有者）。
- `MarkDisconnectedAndResetChannelWhenLastHolder_MarksDisconnectedWhenStillConnected`
  - 输入：栈上直接构造 `ShmBuffer`（连接号 1、状态 `Connected`），调用一次。
  - 输出：该槽位头状态为 `DisConnected`；因调用前是 `Connected`（非最后一任），计数器**不动**。
- `MarkDisconnectedAndResetChannelWhenLastHolder_ResetsHeaderWhenAlreadyDisconnected`
  - 输入：先写入 5 字节、再手动置位断连，随后调用一次。
  - 输出：该槽位头状态为 `UnConnected`、`UpWriteCount` 回到 0。

此外**没有 `TEST` 名字的两条编译期断言**（同文件、四档构建均跑，写在文件顶部）：

- 池边界：`HasObjectPoolRoundTrip` concept 分别代入 `ShmBuffer<TestShmBufferSize>` 与一个自带 `Allocate` / `Deallocate`
  的探针类型；前者代入须为 `false`（池往返不出现在已安装头文件里），后者须为 `true`（证明判据不是恒假）。
- 头结构长度：`sizeof(SingleShmHeader) == 7 * sizeof(unsigned)`（第一节的 28 字节 ABI 钉）。

`test/unittest/Network/ShmInitTest.cpp`（四档构建均跑）：映射自描述的三条用例见
`docs/shm-shared-object-lifecycle.md` 第八节（`Init_RejectsReusedShmObjectWithForeignMappingMagic`、
`Init_AcceptsStampedShmObjectForAClient`、`Init_RejectsUnstampedShmObjectForAClient`）。

## 六、已知未覆盖

- **「两端连接数不一致」只挡住了后果的一半**（2026-09-28）：
  - **客户端侧已挡**：越界连接号在 `CheckConnectResult` 被拒（第一节），不再有越界指针/越界 `sems_` 下标/越界槽位 `Status` 改写。
  - **服务端侧仍不校验**，且客户端拒绝时服务端**已**为那个槽位 `AddConnect` 过（`connectCount_` 已加一）。
    服务端的 5 秒超时只把控制头复位、把这个槽位的头清成 `UnConnected` —— 槽位对下一次协商**可复用**，
    但服务端会为同一槽位再 `Allocate` 一个 `ShmConnect`，于是**每经历一次不完整协商就多占一份 `connectCount_`**，
    累积到 `maxConnectSize_ - 1` 后服务端开始拒绝一切连接。该泄漏无用例、未修（要修需给服务端加槽位回收）。
  - **映射面那另一半已消除**：客户端 `Init()` 不再 `ftruncate`（改为 `fstat` 校验既有对象的大小），
    见 `docs/shm-shared-object-lifecycle.md` 第四节。
- **服务端协商超时的槽位复位是「只清头、不回收连接对象」**（2026-09-28）：超时分支现在改用
  `SingleShmHeader::ResetChannelHeader(commonShmHeader_ + timedOutIndex)`（原先是对该头整体 `memset`），
  语义等价且原子序正确，但 `connectCount_` 与那个槽位上的 `ShmConnect` 仍留在 `connects_` 里 —— 即上一条泄漏本身未修。
- **首轮创建—打戳之间存在窄窗**（2026-09-28）：服务端 `Init()` 先 `memset` 整个映射（此刻 `MappingMagic` 为 0）、
  再打戳。若客户端恰好在这一瞬间走到复用校验，会读到未打戳的头并把对象判为「外来布局」而拒绝；重试即可恢复。
  窗口长度是服务端 `memset` 整段映射的耗时（连接数越大越长），单进程冒烟与四档单测均未复现。
- **拒绝分支的调用点没有用例**：触发它需要「服务端连接数 > 客户端连接数」的两端搭配。
  **原先造不出这个局面**（客户端 `Init()` 的 `ftruncate` 会把服务端对象截短，见上一批的归档记录）；
  客户端改为 `fstat` 校验后这条障碍已消除，局面理论上可在单进程内构造（同一对象、两个 `IoBase` 实例、连接数不同），
  但**用例仍未写**：被钉住的只是判据本身（`ShmBufferTest`），`ShmClient::CheckConnectResult` 里那处调用没有对应用例。
- **`Sem UnLock Failed.` 是既有抖动**：`Sem` 的 Windows 计数上限是 1（`CreateSemaphoreA(..., 1, 1, ...)`），
  客户端的 `Send` 每写一段就 `UnLock` 一次、服务端每轮 IO 周期 `Lock` 一次，两者失衡时会多释放一次并记 Error。
  2026-09-28 的 11 轮 Shm 冒烟里出现 1 次（第 4 轮，回显照常跑完 10000 次），不在本次改动的路径上，未修。
- **Release 档没有类型兜底**：`ShmBase::AddConnect` 的类型断言在 `NDEBUG` 下不存在，跨模块的错误注册只在 Debug 会被抓到。
- **零号槽位的 2 MiB 是浪费而非错误**：连接数 1 时映射 2 MiB、头数组只占其前几十字节，其余空闲；
  这是「通道偏移不接头数组」这一取法的固有代价，未做紧凑化。
