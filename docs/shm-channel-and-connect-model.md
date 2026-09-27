# Shm 后端的通道布局、连接对象类型与并发契约

本文记录共享内存后端（`src/Network/Shm/`、`include/Spark/TemplateLib/Buffer/ShmBuffer.h`）里四条无法从类型或命名读出的约定：
**通道在映射里的摆放、`IoBase::connects_` 里存的是什么类型、`Send` 在两个后端族下的不同完成语义、以及两端并发读写共享头的边界**。
本仓不写注释（Harness §4：命名即文档），故成文于此。与 `docs/shm-shared-object-lifecycle.md` 互补：
后者讲**对象**（命名、映射长度、残留复用、创建与销毁），本文讲**对象内部的布局与两侧的约定**。

## 一、通道布局：连接号从 1 起，0 号槽位留给头数组

- 映射长度是 `ShmBufferSize * maxConnectSize_ * 2`（推导见 `docs/shm-shared-object-lifecycle.md` 第二节）。
- **头数组紧挨映射起点**：`SingleShmHeader` 共 `maxConnectSize_` 个。0 号是**控制头**（`ShmBase::commonShmHeader_`），
  承载连接协商状态；1..`maxConnectSize_ - 1` 号是各连接的**通道头**（`ShmBuffer::shmHeader_` 指向其中自己那一个）。
- **数据通道**：第 i 号连接占 `2 * ShmBufferSize`，偏移为 `i * 2 * ShmBufferSize`；上行在前（Client 写、Server 读）、下行紧随其后
  （`ShmBuffer::upBuffer_` / `downBuffer_`）。通道偏移只按连接号算，**不含头数组的大小**。
- 头数组与通道相加恰好占满映射，于是 **0 号连接的槽位（前 2 MiB）不用作通道**——它容纳整个头数组
  （`maxConnectSize` 个头远小于 2 MiB）。这就是 `IsValidConnectionIndex(connectionIndex >= 1)` 的由来：
  传 0 会让通道压在头数组上，传负数会被 `static_cast<size_t>` 变成一个巨大的偏移。
- **两侧的连接号必须一致**：服务端在 `ShmServer::Accept` 挑第一个空闲槽位 i 并把它写进控制头的 `DownWriteCount`，
  客户端在 `ShmClient::CheckConnectResult` 从同一字段取回 i，各自用它构造 `ShmBuffer`。
  故控制头的 `DownWriteCount` 在协商期被复用为**槽位号**（不是计数器），`ShmServer::Accept` 的 5 秒超时复位也读它。
- **构造期防护**：连接号非法或基址为空时**保持「未 Attach」**（`shmHeader_` 为空），
  此后 `Write` / `Read` / `GetWriteBufferSize` / `GetReadBufferSize` 惰性返回 0，`SetConnectStatus` / `ResetSharedHeader` 不生效，
  共享内存一个字节都不动。Debug 档另有 `assert(IsValidConnectionIndex(...))` 当场拦下。

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
  该仲裁不靠锁，靠 `Status` 的这一次读—改语义。
- 槽位回收路径：`RemoveConnect` → `Connect::Deallocate` → `ShmBuffer::Deallocate`（上面的仲裁）→ 该槽位头被置为 `DisConnected`。
- **`volatile` 的用途与边界**（用户 2026-09-26 的裁定，原文与后续风险登记见归档 `Q.23`）：跨进程共享内存上的 `volatile`
  只用来阻止编译器把值缓存在寄存器里；**同步机制不靠它**，可见性依赖「单写单读 + 硬件缓存一致性」这一实践约定。
  一旦出现多写、或状态与数据被拆到不同字段，须按 Harness §6 重新评估（届时换 `std::atomic` 与内存序）。

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
- `MarkDisconnectedAndReportWhetherLastHolder_ReportsLastOwner`
  - 输入：同一份 header 上先后两次调用。
  - 输出：第一次 `false`（由本次置位）、第二次 `true`（已是最后持有者）。
- `Deallocate_ResetsSharedHeaderWhenAlreadyDisconnected`
  - 输入：缓冲区先写入 5 字节、再手动置位断连，随后 `Deallocate()`。
  - 输出：该槽位头状态为 `UnConnected`、`UpWriteCount` 回到 0。

## 六、已知未覆盖

- **「连接号两侧不一致」无用例也无防护**：编号由控制头单点传递，未做二次校验；若服务端复用了槽位而客户端仍持旧号，
  两端会操作不同的通道（第一节的「两侧必须一致」是约定而非校验）。
- **Release 档没有类型兜底**：`ShmBase::AddConnect` 的类型断言在 `NDEBUG` 下不存在，跨模块的错误注册只在 Debug 会被抓到。
- **零号槽位的 2 MiB 是浪费而非错误**：连接数 1 时映射 2 MiB、头数组只占其前几十字节，其余空闲；
  这是「通道偏移不接头数组」这一取法的固有代价，未做紧凑化。
