# Shm 后端的通道布局、连接对象类型与并发契约

本文记录共享内存后端（`src/Network/Shm/`、`include/Spark/TemplateLib/Buffer/ShmBuffer.h`）里四条无法从类型或命名读出的约定：
**通道在映射里的摆放、`IoBase::connects_` 里存的是什么类型、`Send` 在两个后端族下的不同完成语义、以及两端并发读写共享头的边界**。
本仓不写注释（Harness §4：命名即文档），故成文于此。与 `docs/shm-shared-object-lifecycle.md` 互补：
后者讲**对象**（命名、映射长度、残留复用、创建与销毁），本文讲**对象内部的布局与两侧的约定**。

## 一、通道布局：连接号从 1 起，0 号槽位留给头数组

- 映射长度是 `ShmBufferSize * maxConnectSize_ * 2`（推导见 `docs/shm-shared-object-lifecycle.md` 第二节）。
- **头数组紧挨映射起点**：`SingleShmHeader` 共 `maxConnectSize_` 个。0 号是**控制头**（`ShmBase::commonShmHeader_`），
  承载连接协商状态；1..`maxConnectSize_ - 1` 号是各连接的**通道头**（`ShmBuffer::shmHeader_` 指向其中自己那一个）。
- **头结构的字段顺序是跨进程 ABI，被 `ShmMappingLayoutVersion` 钉住**：`Status`（4 字节）之后是 4 字节对齐填充，
  再跟 6 个 `size_t` 字段（64 位平台各 8 字节），合计 **56 字节**，顺序为
  `Status`、`MappingMagic`、`MappingLayoutVersion`、`UpWriteCount`、`UpReadCount`、`DownWriteCount`、`DownReadCount`。
  前三个由 0 号控制头承载映射自描述（见 `docs/shm-shared-object-lifecycle.md` 第三节），后四个是每槽位各自的读写计数器。
  2026-09-28 之前这 6 个字段是 `unsigned`（4 字节），故当时的头是 7 × 4 = 28 字节；加宽是本次改动的一部分，
  `ShmMappingLayoutVersion` 随之由 1 升到 2。
  `ShmBuffer.h` 与 `ShmBufferTest` 都用 `static_assert(sizeof(SingleShmHeader) == 8 + 6 * sizeof(size_t))` 钉住总长，
  另有一条 `alignof(SingleShmHeader) == alignof(size_t)`：`size_t` 计数器必须自然对齐，`std::atomic_ref` 才成立。
  **改字段顺序或插入字段必须同时递增 `ShmMappingLayoutVersion`**，否则新旧进程会把同一段内存解释成两种布局。
- **头布局现在随平台的 `size_t` 宽度变化**（2026-09-28 起）：LP64 下 56 字节、ILP32 下 32 字节。
  复用校验只比对 `MappingMagic` 与 `MappingLayoutVersion`，**认不出两端宽度不同**——同版本号、不同宽度的两个进程会互认。
  实际部署恒定 LP64：映射长度本身就有 `1 MiB × 2047 × 2 ≈ 4 GiB`，32 位进程的地址空间装不下，`mmap` / `MapViewOfFile`
  这一关先失败。故这是**记录在案的理论隐患**而非当下缺陷；若将来真要支持 32 位混跑，得把宽度（或每字段宽度表）
  编进 stamp，光靠版本号挡不住。
- **数据通道**：第 i 号连接占 `2 * ShmBufferSize`，偏移为 `i * 2 * ShmBufferSize`；上行在前（Client 写、Server 读）、下行紧随其后
  （`ShmBuffer::upBuffer_` / `downBuffer_`）。通道偏移只按连接号算，**不含头数组的大小**。
- 头数组与通道相加恰好占满映射，于是 **0 号连接的槽位（前 2 MiB）不用作通道**——它容纳整个头数组
  （`maxConnectSize` 个头远小于 2 MiB）。这就是 `IsValidConnectionIndex` 下界的由来：
  传 0 会让通道压在头数组上，传负数会被 `static_cast<size_t>` 变成一个巨大的偏移；
  上界 `i < connectionCount` 由同一个谓词给出——`i == 连接数` 时该槽位的头与通道都已越出映射
  （头数组只到 `connectionCount - 1` 号）。
- **头数组装得下是编译期不变式**：头数组占映射前 `maxConnectSize_ * sizeof(SingleShmHeader)` 字节，
  必须落在 0 号槽位的 `2 * ShmBufferSize` 之内，否则会压在 1 号连接的通道上。`ShmBase.h` 的 `static_assert`
  按**最大**允许连接数 `MaxSharedMemoryConnectSize` 校验（对任何运行期连接数都更强），故调小 `ShmBufferSize`
  （例如 16 KiB）会让四档构建当场失败。现状是 `2047 * 56 = 114632` 字节 对 `2 MiB`（约 18 倍余量；
  28 字节头年代是 57316 字节、约 37 倍，2026-09-28 加宽到 56 字节后余量减半，仍很宽裕），
  运行期判据**永远不可达** —— 这正是它做成编译期断言而不是 `IsConnectSizeAllowed()` 里一条分支的原因。
- **两侧的连接号必须一致**：服务端在 `ShmServer::Accept` 挑第一个空闲槽位 i 并把它写进控制头的 `DownWriteCount`，
  客户端在 `ShmClient::CheckConnectResult` 从同一字段取回 i，各自用它构造 `ShmBuffer`。
  故控制头的 `DownWriteCount` 在协商期被复用为**槽位号**（不是计数器）；读它构造 `ShmBuffer` 的只有客户端
  （服务端本就知道自己挑的是哪个槽位），`ShmServer::Accept` 的 5 秒超时复位**不再读它**——槽位自 2026-09-28 起
  由回收路径统一清理（第四节）。
- **槽位号必须先于 `Accepted` 落盘**：服务端写槽位号与写 `Accepted` 是两次独立发布，顺序固定为
  「先 `StoreMappedField(DownWriteCount, i)`、后 `StoreStatus(Accepted)`」。客户端以 acquire 语义读到 `Accepted` 时，
  才因此保证槽位号已经随同一条 release 序列对它可见（内存序模型见第四节）。
  反过来写会让客户端读到「`Accepted` 已到、槽位号还是上一次协商的残留」，取到错误甚至越界的 i。
  同型的次序约束还有第四节「清零必须先于 `UnConnected` 发布」（槽位复用那一侧）。
- **连接号的两侧守卫**（2026-09-28）：`ShmBuffer` 的构造函数接收映射容量 `connectionCount`，
  在指针算术之前用 `IsValidConnectionIndex(i, connectionCount)`（要求 `1 <= i < connectionCount`）同时守住上下界，
  故守卫与指针算术同处一地（见下一段的构造期防护）。客户端手里的 i 由**对端**写入
  （`ShmClient::CheckConnectResult` 从控制头取回），那一处**另留一道显式检查**：取回 i 后先过
  `ShmBuffer<Size>::IsConnectionIndexWithinMapping(i, maxConnectSize_)`，不满足即按协商失败处置 ——
  记 Warning（`Reject Connect Index:%zu Out Of Range`；槽位号自 2026-09-28 起是 `size_t`，格式符随之）、
  控制头复位成 `UnConnected`、走既有的 1 秒重试，
  **不构造 `ShmConnect`**。这道检查不是冗余：构造函数那条路只把非法连接号变成「未 Attach」的哑对象，
  而对端写下的越界号是**可诊断的协商失败**，该拒绝、该重试、该留痕。挡下的是三类越界后果：
  通道偏移落出映射（`upBuffer_` / `downBuffer_` 直指映射之外，构造期 `SetConnectStatus` 与首帧读写即踩）、
  `ShmClient::CheckData` 的 `sems_[i]` 越出信号量数组、以及构造期的 `SetConnectStatus` 把 `Status`
  写到头数组之外（别的槽位头或通道上）。判据本身是 `ShmBuffer` 上的 `constexpr` 静态谓词，两侧共用、由单元用例钉住（第五节）。
- **构造期防护**：连接号越界（上界下界同判）或基址为空时**保持「未 Attach」**（`shmHeader_` 为空），
  此后 `Write` / `Read` / `GetWriteBufferSize` / `GetReadBufferSize` 惰性返回 0，
  `SetConnectStatus` / `ResetSharedHeader` / `RevokeUnconfirmedAccept` 一律不生效，
  共享内存一个字节都不动。Debug 档另有 `assert(IsValidConnectionIndex(...))` 当场拦下；
  Release 档没有断言，但守卫与指针算术写在同一个表达式流里，不存在「绕过守卫直接算偏移」的路径。

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
| 缓冲上界 | 无（池多大就能堆多少） | 固定每方向 1 MiB（`ShmBufferSize`，自 2026-09-28 起全部可用） |

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
- **清零必须先于 `UnConnected` 发布**（2026-09-28 修）：`ResetChannelHeader` 里四次计数器写入与
  `StoreStatus(UnConnected)` 是五次独立发布，顺序固定为「先清四个计数器，最后发布 `UnConnected`」——
  与第一节「槽位号必须先于 `Accepted` 落盘」是同一条道理，只是换到槽位复用这一侧。
  服务端挑空闲槽位时以 acquire 读到 `UnConnected`，与清头那次 release 同步，故**接下这一槽位的新一轮协商
  必然看到清零后的计数器**；反过来写（先发布 `UnConnected`、后清）只保证状态可见，四个计数器的零值落在发布之后，
  新一任持有者可能读到上一纪元残留的计数。此时差值算术会给出错误答案：若 `UpWriteCount` 仍是旧值 5000
  而 `UpReadCount` 已读到 0，可读字节算作 `0 - 5000` 回绕成约 2⁶⁴，`(std::min)(len, 可读字节)` 便等于 `len`，
  于是把上一纪元的通道旧字节当成数据交给上层（**静默错数据，不崩溃**）。
  该次序缺陷**早于 2026-09-28 的计数器加宽**（旧方案同样会把旧纪元字节交上去），单调方案只是让差值恒被
  `(std::min)(len, 可读字节)` 截到 `len`，故一次可能多交出的量从「旧纪元的计数差」变成「调用方要多少给多少」
  ——同一缺陷，后果略重。两处细节：
  其一，清零的这几笔写发生在状态仍是 `DisConnected` 的窗口里，而 `WriteIntoChannel` / `ReadFromChannel`
  都以 `Status == Connected` 为闸门，故**通道使用者**读不到半清的计数——受影响的只有「槽位复用」这一条路，
  正是上面那条 release/acquire 覆盖的范围；其二，同一线程内「先清、后连」的次序由程序顺序本身保证
  （服务端 IO 线程清头后随即受理新连接），本次修的是**跨进程**那条路径（客户端作为最后持有者清头）。
  四次计数器存储各自的 release 语义并非必需——它们被最后一笔状态存储的 release 一并覆盖——
  保留只是让全部共享字段走同一组访问入口。
- **服务端协商超时也走同一条路径**（2026-09-28 起）：`ShmServer::CheckConnect` 每轮对每个 `ShmConnect` 调
  `TryReclaimConnect`——槽位头已是 `DisConnected` 即交给既有的延迟删除
  （`disConnectSessionIds_` → `DoDisConnect` → `RemoveConnect`，`connectCount_` 随之减一）；
  仍是 `Accepted` 且距 `ShmConnect::CreateTimePoint` 已过 `HandshakeTimeoutSeconds`（5 秒）时，
  先调 `ShmBuffer::RevokeUnconfirmedAccept()`（在槽位头上做 `Accepted → DisConnected` 的 CAS），成功才入延迟删除。
  `ShmConnect` 析构里的仲裁因此看到「已是 `DisConnected`」→ 返 true → `ResetSharedHeader()`，
  槽位头清零、槽位对下次协商可复用，`connectCount_` 与连接对象一并收回。
  **控制头那侧同点收尾**：`ShmServer::Accept` 的 5 秒超时分支只复位**控制头**（不再碰槽位头），
  两侧计时起点是同一时刻——`CreateTimePoint` 先于 `lastWriteTimePoint_` 取，故回收的计时**不晚于**控制头超时。
- **`RevokeUnconfirmedAccept` 只撤 `Accepted`**：以 `compare_exchange_strong` 置换，已确认（`Connected`）的连接
  不会被回收路径撤掉——对端只要确认过，回收路径就与它无关；重复调用只有第一次返回 true。
  也因此它**不负责**清槽位头：置位后由 `ShmConnect` 析构走上一段的仲裁，回收路径与正常断连共用同一段收尾。
- **客户端确认受理也是一次 CAS，与回收路径争同一个状态字**（2026-09-28 修）：入口是
  `SingleShmHeader::ConfirmAcceptedConnection`，在槽位头上做 `Accepted → Connected` 的 `compare_exchange_strong`
  （成功序 `acq_rel`、失败序 `acquire`）；`ShmClient::CheckConnectResult` **先确认成功、再构造 `ShmConnect`**，
  失败即记一条 Warning 并走既有的 1 秒重试。此前这一步是 `ShmBuffer` 构造函数里的 `SetConnectStatus(Connected)`
  ——一次**盲写**，与上一条的撤回 CAS 相撞时后写者胜：客户端晚于服务端撤回的 5 秒才走到构造，槽位头就停在
  `Connected`，而服务端那侧已无连接对象，该槽位既不会被复用、也不会被回收路径再次看到（详见第六节「孤儿槽位」）；
  反序则客户端自认已连上、通道却因头是 `UnConnected` 静默吞掉每一次 `Write`（返 0）。
  改成 CAS 后两侧争的是同一个字，胜负唯一：撤回在前则客户端 CAS 必失败，客户端在前则撤回 CAS 必失败
  （回收路径不入延迟删除，与上一条「只撤 `Accepted`」同一机制）。
  `ShmBuffer` 的构造函数签名与其中那笔 `SetConnectStatus` 未动——它的发布已确定发生在 CAS 成功之后，
  同值 release store 不会让一个已被撤回的槽位复活；「必须先确认再构造」这一条因此是**次序要求**，
  不能把 CAS 挪到构造之后。残留窗口：CAS 成功后到 `AddConnect` 返回之间若抛异常（池分配 `std::bad_alloc`、
  连接表插入），仍会留下一个停在 `Connected` 的槽位，与修前同形，但已不在正常时序里，且需异常注入才可复现。
- **内存序：`std::atomic_ref` 取代 `volatile` + 手写屏障**（2026-09-28 改动，**取代**归档 `Q.23` 的裁定；原裁定原文与
  登记背景仍在归档里，此处不重写它，只声明其结论已被本次改动覆盖）。`SingleShmHeader` 的全部字段访问统一走
  `LoadStatus` / `StoreStatus` / `LoadMappedField` / `StoreMappedField` 四个静态入口，读一律 `acquire`、写一律 `release`：
  - 通道搬运的发布顺序因此有了**语言级**保证：`WriteIntoChannel` / `ReadFromChannel` 先以 acquire 读入两个计数器、
    搬运完数据、再以 release 写回新计数。对端以 acquire 读计数时，必然看到与之配套的那段数据（`memcpy` 不会越过 release）。
  - `Status` 是唯一的双写字段，写点全部走 CAS 或 release store；读点全部 acquire。
  - 计数器的读入从「同一个操作里读两次」收敛成「读一次、复用到容量计算与写回值」，顺带消掉了两次读之间可能被对端插进来的窗口。
  - `std::atomic_ref<T>::is_always_lock_free` 对 `ConnectStatusType`（`int32_t`）与 `size_t`（LP64 下 8 字节）都是编译期断言：
    若某平台上共享内存里的这两档宽度访问不是无锁的，四档构建当场失败。这是**故意的**——共享内存里塞一把锁没有意义。
    配套的 `static_assert(alignof(SingleShmHeader) == alignof(size_t))` 保证 8 字节计数器自然对齐，无锁访问的前提才成立。
  - `volatile` 已从 `ShmBuffer.h` 与相关调用点全部移除；Harness §6 / `rules/cpp-style.md` §6 都禁止用 `volatile` 做同步，
    此前是依赖归档 `Q.23` 的实践约定豁免，现在不再需要豁免。
- **四个计数器是单调累计索引，取模只发生在寻址那一刻**（2026-09-28 改动，方案取自 `SpscRingBuffer` 的 Mask 写法）：
  - `UpWriteCount` / `UpReadCount` / `DownWriteCount` / `DownReadCount` **只增不减**，不再被夹在 `[0, Size]` 内。
    可读字节 = `writeIndex - readIndex`；可写字节 = `Size - (writeIndex - readIndex)`；通道内位置 = `index & Mask`
    （`Mask = Size - 1`）。故 `Size` 必须是 2 的幂，由 `static_assert` 钉住。
  - 无符号差值在回绕点上仍然正确：`Size` 是 2 的幂、真实未读字节数恒小于 `Size`，故 `size_t` 走完一整圈（2⁶⁴）时
    `writeIndex - readIndex` 与 `Size - (writeIndex - readIndex)` 都仍等于真实值；计数器本身不需要取模或归一化。
  - 单写单读的分工（本节第 1 条）未变，故差值没有撕裂窗口：每个计数器只有一端写、另一端只读。
  - **容量语义随之改变**：通道可用字节从 `Size - 1` 变成 `Size`。旧方案靠「写计数不得等于读计数」区分满/空，白留一个
    字节（1 MiB 通道实际只有 `1 MiB - 1` 可用）；Mask 方案用「差值」区分，1 MiB 全部可用。
    `GetWriteBufferSize()` 在空通道上现在返回 `Size`（旧为 `Size - 1`），`ShmBufferTest` 的容量断言已按新语义改写。
  - **代价：丢了一道 Debug 期的腐蚀哨兵**。旧实现末尾有 `assert(writeCount <= Size)`，读写计数一旦被外部污染（或对端按
    错误布局解释同一段内存）就在 Debug 下当场命中；单调方案下 `size_t` 计数取任何值都「合法」，没有可断言的界。
    替代品是复制助手里的 `assert(length <= Size)`，它只保证**单次搬运**不超过一个通道，**对计数本身的腐蚀不再有任何检查**。
    这是本方案唯一的净损失，已记入 `PROGRESS.md` 待议。
- **这套算术只存在于一处：`RingView<Size>`（2026-09-28 抽取）**。上面那条 Mask 方案原本在两处各写一遍——`SpscRingBuffer`
  自己一份、`ShmBuffer` 一份（后者的 `CountReadableBytes` / `CountWritableBytes` / `CopyIntoChannel` / `CopyOutOfChannel` / `Mask`
  与前者逐行同构），违反 Harness §5。故把「存储 + 一对单调索引 + 掩码算术」抽成 `include/Spark/TemplateLib/Buffer/RingView.h`：
  - `RingView<Size>` 是**视图**，不拥有任何存储：构造只接三个外部量——`char* storage`、`size_t& writeIndex`、`size_t& readIndex`
    （引用成员，故对象名 `View` 名副其实）。索引读写一律走 `std::atomic_ref<size_t>`，读 `acquire`、写 `release`
    （`is_always_lock_free` 由类内 `static_assert` 钉住）。它**不提供任何重置入口**：清零一律由持有者自己决定，
    因为「清四个计数器」在 `ShmBuffer` 里是带发布顺序要求的动作（见上一条 `ResetChannelHeader` 的次序约束），
    环本身无权替调用方决定用哪种内存序。
  - `SpscRingBuffer<Size>` 保留原有的值语义（自己持有 `alignas(64)` 的存储与一对索引）与全部公开签名，内部改为持一个
    自引用的 `RingView` 并逐方法委托；`ResetWhenIdle()` 仍在它自己这里做（relaxed 双清，前提是调用方保证静止）。
    代价与收益如实记：索引成员由 `std::atomic<size_t>` 变为 `size_t`（对外不可见，尺寸/对齐不变），**本端索引的读取
    由 `relaxed` 升为 `acquire`**——这是「统一走一套内存序」的直接后果，在 x86-64（本项目四档的目标）与原来编译成同一条
    `mov`，仅在弱序架构上多一次屏障；`SpscRingBuffer` 既无生产调用者（除聚合头 `TemplateLib.h` 外只有自己的用例），
    这一升格也无调用点受影响。
  - `ShmBuffer<Size>` 的两个方向各对应一个视图，且**不缓存视图对象**：`UpRing()` / `DownRing()` 每次按需构造
    （三个字长的句柄，内联后无痕），因此「未 Attach」时不必存在一个悬空视图，类布局与 `sizeof` 一个字都没变。
    四个搬运入口（`UpWrite` / `UpRead` / `DownWrite` / `DownRead`）现在是「先过一次状态闸门、再交给视图」的一行调用，
    `Status == Connected` 这个闸门由此收敛为唯一一个具名谓词 `IsChannelConnected()`（原先它写在两个搬运助手内部、各一份）。
  - **视图不缓存：按需构造，已裁定维持（2026-09-28）**。曾议「把两个方向的视图存成成员，省掉每次访问的构造」，
    实测后否决。构造一个视图只是把三个句柄写进对象，Release 下这一步要么被完全消解、要么退化成一两次栈写，
    而**存成员并不省这一步**——它把三个句柄存进对象里，每次访问反而得先把它们加载回来。
    四档实测（探针 `out/probe_view_cost4.cpp` 与 `out/probe_view_cost.cpp`，`out/` 不入库；下表是**指令条数**，
    同一次运行内可比，不是 profile）：
    - 查询路径（三种写法都内联到底）：按需构造比 `optional` 成员少 1 条、比指针成员少 2 条，省下的正是那两次
      依赖加载。g++ -O2 为 7 / 8 / 9（按需 / `optional` 成员 / 指针成员），MSVC /O2 为 6 / 7 / 8，差值同向。
    - 写路径 g++ -O2：86 / 88 / 88，差额同样是那两次加载。
    - 写路径 MSVC /O2 **不可直接比**：MSVC 不内联 `RingView<Size>::Write`（函数体 90 余行），按需写法把 24 字节
      视图铺在栈上传地址（3 次栈写 + 一次 `call`），成员写法则退化成尾调用跳板（2 / 1 条）——真正的工作都落在
      被调函数里，只数调用方会得出**反向**结论。
    - 构造一次（建连时）：`std::optional<RingView<Size>>` 与按需构造在四档配置下**逐指令相同**（g++ -O2 下
      `BuildOnDemand` 与 `BuildOptional` 生成的是同一串汇编）；指针成员多一次分配/释放（+19～24 条）。
    - 唯一真实差别在 **Debug**：`/Od` / `-O0` 下按需构造每次访问多一次不内联的 `UpRing()` 调用
      （MSVC /Od 该函数体 17 条，g++ -O0 22 条）与一次视图构造，成员写法把这一层换成一次成员装载。
    故维持现状。若将来为 Debug 的单次调用次数、或为「视图有效性单点」而改，**首选
    `std::optional<RingView<Size>>` 成员**——零分配，且构造路径与现状逐指令相同；堆指针不占优，
    且另有一条与性能无关的反对理由：`ObjectPool::Allocate` 先弹空闲块、后放置构造且**无异常回滚**
    （`ObjectPool.h:51-58`），构造函数里一旦 `new` 抛 `std::bad_alloc`，该块即泄漏出空闲链。
    尺寸：视图 24 字节，两个指针成员 +16、两个 `optional` 成员 +64；`ShmBuffer` 是进程内的池对象，
    三者都不动映射布局与 ABI。最后一条限定：调用点**跨 TU** 时内联是否仍成立未测。

## 五、相关测试与输入输出

`test/unittest/TemplateLib/ShmBufferTest.cpp`（四档构建均跑）。

- `OutOfRangeConnectionIndexTripsAssert`（仅 Debug 且启用 death test）
  - 输入：连接号 0，以及恰在容量上的连接号 2（容量 2）。
  - 输出：两次都断言命中（`IsValidConnectionIndex`）——上下界各钉一次。
- `InvalidConnectionIndexLeavesBufferDetached`（仅 Release，`NDEBUG`）
  - 输入：连接号 0、-1，以及恰在容量上的 2（容量 2）。
  - 输出：三者均 `GetShmHeader() == nullptr`、`Write` 返 0、`GetWriteBufferSize()` 为 0，
    且共享内存首字节的状态仍为 `UnConnected`（构造期一个字节都没写）。
- `UnAttachedBufferRejectsAllChannelAccess`
  - 输入：默认构造（未 Attach）的实例，逐个调用 `Write` / `Read` / 两个 `GetXxxBufferSize` /
    `MarkDisconnectedAndReportWhetherLastHolder` / `RevokeUnconfirmedAccept` / `SetConnectStatus` / `ResetSharedHeader`。
  - 输出：读写与容量查询返 0，两个状态谓词返 `false`，状态写入不生效，`GetShmHeader()` 仍为空。
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
- `MarkDisconnectedAndResetChannelWhenLastHolder_LeavesTheNextEpochAnEmptyChannel`（2026-09-28 新增）
  - 输入：上行写 5 字节且被服务端读走、下行写 5 字节且被客户端读走（四个计数器此刻**都非零**），
    再手动置位断连、随后调用一次（走到「最后一任持有者清头」这条路）。
  - 输出：槽位头状态为 `UnConnected`，四个计数器**全部**回到 0；另以新构造的 Client 与 Server 两份视图
    挂同一槽位，两侧的可写字节都是满额 `Size`、可读字节都是 0——上一纪元的字节对新纪元不可见。
  - 与上一条的分工：上一条只写不读，`UpReadCount` / `DownReadCount` 在清头前本就是 0，
    那两个断言是空转；本条先把四个计数器都推成非零，才谈得上「清干净」。
  - **本条判据不覆盖第四节新修的存储次序**：单进程、单线程下「清计数器」与「发布 `UnConnected`」的先后
    **不可观测**（两笔存储在本线程内按程序顺序生效，x86 的 TSO 更是如此），故用例钉的是清头后的**终态契约**；
    次序的正确性由 release/acquire 配对（语言级内存模型）保证，不由用例保证。要让它可判别须引入
    跨线程或跨进程的观测点，而本仓单测无多线程先例、x86 上此类用例的判别力近 0，按 Harness「最小改动优先」不做。
- `RevokeUnconfirmedAccept_ReclaimsTheChannelWhenThePeerNeverConfirmed`
  - 输入：连接号 1、状态 `Accepted`（模拟「服务端已受理、对端从未确认」），调用两次。
  - 输出：第一次 `true` 且该槽位头状态变为 `DisConnected`；第二次 `false`（CAS 只在第一次成功），
    槽位头**仍停在 `DisConnected`**（清头是析构那侧的事，见第四节）。
- `RevokeUnconfirmedAccept_LeavesAConfirmedChannelAlone`
  - 输入：连接号 1、状态 `Connected`（模拟回收路径碰到一条已确认的连接）。
  - 输出：返 `false`，槽位头状态仍是 `Connected`。
- `ConfirmAcceptedConnectionPublishesConnectedOnlyFromAccepted`（2026-09-28 新增）
  - 输入：栈上零初始化的 `SingleShmHeader`，状态置为 `Accepted`，连调两次。
  - 输出：第一次 `true` 且状态为 `Connected`；第二次 `false`，状态**仍停在 `Connected`**（CAS 只在第一次成功）。
- `ConfirmAcceptedConnectionLeavesARevokedChannelUntouched`（2026-09-28 新增）
  - 输入：状态置为 `DisConnected` 后调用一次；再置为 `UnConnected` 后调用一次。
  - 输出：两次都返 `false`，状态各自保持原值——撤回（`DisConnected`）与清头（`UnConnected`）都不会被这一笔写复活。
  - 与上一条的分工：上一条钉正向发布（`Accepted → Connected` 且只成功一次），本条钉失败侧**不改写槽位头**，
    即第四节「两侧争同一个字、失败的那侧原地不动」这半句。
  - 两条都是纯状态机用例（栈上自造头、无 fixture、无共享内存）：`ShmClient::CheckConnectResult` 那处调用点
    仍没有用例，见第六节末条。

此外**没有 `TEST` 名字的两条编译期断言**（同文件、四档构建均跑，写在文件顶部）：

- 池边界：`HasObjectPoolRoundTrip` concept 分别代入 `ShmBuffer<TestShmBufferSize>` 与一个自带 `Allocate` / `Deallocate`
  的探针类型；前者代入须为 `false`（池往返不出现在已安装头文件里），后者须为 `true`（证明判据不是恒假）。
- 头结构长度与对齐：`sizeof(SingleShmHeader) == 8 + 6 * sizeof(size_t)` 与 `alignof(SingleShmHeader) == alignof(size_t)`
  （第一节的 56 字节 ABI 钉；28 字节头的年代这里是 `7 * sizeof(unsigned)`）。

`test/unittest/Network/ShmInitTest.cpp`（四档构建均跑）：映射自描述的三条用例见
`docs/shm-shared-object-lifecycle.md` 第八节（`Init_RejectsReusedShmObjectWithForeignMappingMagic`、
`Init_AcceptsStampedShmObjectForAClient`、`Init_RejectsUnstampedShmObjectForAClient`）。

第四节的回收路径由同文件的 `ShmConnectLifecycleTest` 两条用例钉住。对端由测试**经映射视图直接扮演**
（写控制头状态），服务端只用 `HandleIoEvent()` 逐轮驱动——`HandleIoEvent` 里的 `Sem::Lock()` 是 100 ms 限时等待，
故整条路径可在单进程内复现：

- `ReclaimsConnectWhosePeerNeverAttached`
  - 输入：控制头写 `Connecting` → 跑一轮（受理成功）；再把控制头改回 `UnConnected`（模拟客户端拒绝那个槽位号）
    → 再跑一轮；随后每 50 ms 驱动一轮，直到回收或 8 秒截止。
  - 输出：第一轮 `OnConnect` 一次、1 号槽位头为 `Accepted`；第二轮**不回收**（`OnDisConnect` 仍 0、槽位头仍 `Accepted`
    ——控制头已不是 `Accepted`/`Rejected`，超时分支不介入）；约 5 秒后 `OnDisConnect` 一次、1 号槽位头回到 `UnConnected`；
    再发一次 `Connecting` 时**仍落在 1 号槽位**（`OnConnect` 累计 2）——槽位确实回了池。
    最后一条断言是「旧代码下必失败」的判据：旧代码的槽位永远停在 `Accepted`，第二次协商会挑走 2 号。
- `ReclaimsConnectWhosePeerNeverConfirmedAndResetsTheControlHeader`
  - 输入：控制头写 `Connecting` → 跑一轮（受理成功）后**不再动控制头**（模拟客户端写完槽位号、确认前死亡），
    随后驱动到「回收已发生**且**控制头已复位」或 8 秒截止。
  - 输出：`OnDisConnect` 一次、控制头回到 `UnConnected`（`Accept` 的超时分支复位它）、1 号槽位头回到 `UnConnected`
    （回收路径清零）。两条路径的计时起点同刻，谁先到期都收敛到同一结果。
- **这两条用例各含一次约 5 秒的真实等待**，四档单测因此各多约 10 秒。`HandshakeTimeoutSeconds` 是编译期常量，
  用例没有注入点；要换假时钟得给服务端开一个缝，按 Harness「最小改动优先」不做。

## 六、已知未覆盖

- **「两端连接数不一致」两侧都已收口**（2026-09-28）：
  - **客户端侧已挡**：越界连接号在 `CheckConnectResult` 被拒（第一节），不再有越界指针/越界 `sems_` 下标/越界槽位 `Status` 改写。
  - **服务端侧的槽位与 `connectCount_` 泄漏已修**（2026-09-28 第二批）：控制头出现 `Accepted` 后若对端
    既不来确认、控制头也停在 `Accepted`（客户端拒绝那个连接号的典型形态），新的回收路径按
    `ShmConnect::CreateTimePoint` 计满 5 秒即撤回并回收（第四节）。此前一次这样的协商会永久占掉一个槽位
    与一份 `connectCount_`，累积 `maxConnectSize_ - 1` 次后服务端开始拒绝一切连接——这正是本条先前记的漏洞。
  - **映射面那另一半已消除**：客户端 `Init()` 不再 `ftruncate`（改为 `fstat` 校验既有对象的大小），
    见 `docs/shm-shared-object-lifecycle.md` 第四节。
- **「孤儿槽位」已修**（2026-09-28）：原缺陷的特征是客户端的确认与服务端的撤回**各自写**同一个状态字而可能
  后写者胜——客户端可以晚于 5 秒把槽位头写成 `Connected`，而服务端那侧已无连接对象，于是该槽位既不被复用、
  又不被回收路径看到（服务端侧永久少一个槽位）；反序则让客户端自认已连上而通道静默吞掉每次 `Write`（返 0）。
  修法是让客户端也走 CAS（`Accepted → Connected`，失败即重试），两侧由同一次仲裁定胜负，见第四节。
  残留：CAS 成功后到 `AddConnect` 返回之间若抛异常（池分配 `std::bad_alloc`、连接表插入），仍会留下同形的槽位；
  该窗口需异常注入才可复现，**无用例**。
  **调用点仍无用例**：被钉住的只有状态机本身（`ShmBufferTest` 两条，见第五节），
  `ShmClient::CheckConnectResult` 里那处调用没有对应用例——与下面「拒绝分支的调用点没有用例」同一情形。
- **首轮创建—打戳之间存在窄窗**（2026-09-28）：服务端 `Init()` 先 `memset` 整个映射（此刻 `MappingMagic` 为 0）、
  再打戳。若客户端恰好在这一瞬间走到复用校验，会读到未打戳的头并把对象判为「外来布局」而拒绝；重试即可恢复。
  窗口长度是服务端 `memset` 整段映射的耗时（连接数越大越长），单进程冒烟与四档单测均未复现。
- **拒绝分支的调用点没有用例**：触发它需要「服务端连接数 > 客户端连接数」的两端搭配。
  **原先造不出这个局面**（客户端 `Init()` 的 `ftruncate` 会把服务端对象截短，见上一批的归档记录）；
  客户端改为 `fstat` 校验后这条障碍已消除，局面理论上可在单进程内构造（同一对象、两个 `IoBase` 实例、连接数不同），
  但**用例仍未写**：被钉住的只是判据本身（`ShmBufferTest`），`ShmClient::CheckConnectResult` 里那处调用没有对应用例。
- **`Sem UnLock Failed.` 是既有抖动**：`Sem` 的 Windows 计数上限是 1（`CreateSemaphoreA(..., 1, 1, ...)`），
  客户端的 `Send` 每写一段就 `UnLock` 一次、服务端每轮 IO 周期 `Lock` 一次，两者失衡时会多释放一次并记 Error。
  2026-09-28 的 7 轮 Shm 冒烟里出现 1 次（第 1 轮，回显照常跑完 10000 次），不在本次改动的路径上，未修。
- **Release 档没有类型兜底**：`ShmBase::AddConnect` 的类型断言在 `NDEBUG` 下不存在，跨模块的错误注册只在 Debug 会被抓到。
- **零号槽位的 2 MiB 是浪费而非错误**：连接数 1 时映射 2 MiB、头数组只占其前几十字节，其余空闲；
  这是「通道偏移不接头数组」这一取法的固有代价，未做紧凑化。
