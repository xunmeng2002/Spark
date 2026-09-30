# IO 层订阅者通知契约

本文定义 `IoBase` 向 `IoSubscriber` 投递通知时的契约：通知与登记的关系、通知之间的配对、
以及回调抛异常时由谁承担。四个后端（Tcp Select / Tcp Epoll / Tcp Iocp / Shm）共用 `IoBase` 的登记与拆除，
故本契约对四者一致；收包通知（`OnRecv`）与连接通知共用同一份兜底，差别只在调用点，见「收包通知」一节。

## 契约

- **回调不得让异常穿出 IO 层**。`OnConnect` / `OnDisConnect` / `OnRecv` 任一回调抛出的异常由 `IoBase` 收住并记一条
  `Error` 日志（含异常自身的 `what()`），不再向调用方传播。四个后端的 `AddConnect` 调用点
  （`ShmServer::Accept`、`TcpEpollBase`、`TcpSelectClient`、`TcpIocpClient`）里只有 `ShmClient` 一侧有 `try`，
  故此前一个订阅者 bug 会从其余三处穿出 `HandleIoEvent`、直达应用主循环。
- **每个连接恰好一次 `OnConnect`、恰好一次 `OnDisConnect`**。配对以**登记**（`connects_` 里有该会话）为准。
- **拆除动作不依赖通知完成**：连接表摘除与连接对象归还排在通知之后，但通知抛异常不再能跳过它们。
- **从未宣告过的连接不产生 `OnDisConnect`**：`RemoveConnect` 遇到未登记的连接只归还对象、不通知。
- **收包通知不改变后端自身的收尾**：`OnRecv` 由后端在自己的收包路径上调起，回调之后紧跟一段只属于后端的收尾，
  兜底收住异常后控制流照常往下走，故该收尾无论回调成败都会执行（见「收包通知」一节）。

## 收包通知

收包通知与连接通知的**兜底规则完全相同**，但它的调用点在四个后端各自的收包路径上，且每条路径在回调之后
都有一段与订阅者无关、却必须跑完的收尾：

| 调用点 | 回调之后的收尾 | 此前漏掉收尾的后果 |
| --- | --- | --- |
| `ShmBase::DoRecv` | `buffer->Deallocate()` | 缓冲块泄漏；异常穿出 `HandleIoEvent` |
| `TcpBase::DoRecv` | `buffer->Deallocate()` | 缓冲块泄漏；异常穿出 `HandleIoEvent` |
| `TcpIocpBase::OnRecvComplete` | `PostRecv(overlapped)` | 该连接此后再收不到包；异常穿出即 `std::terminate` |

IOCP 一列的后果最重：IO 循环跑在 `IoThread` 上，而 `ThreadBase` 链路上没有任何 `catch`，
一个从 `OnRecvComplete` 穿出的异常直接终止进程。

**收包缓冲由 IO 层持有、仅在本次回调期间有效**（`IoBase.h` 的 `OnRecv` 声明处已注明）：订阅者不得归还、
不得留存该指针，需要留存请当场拷贝；`data` 不保证以 NUL 结尾，有效字节数以 `length` 为准。
正因为缓冲不属于订阅者，后端把它还回池这一步**不能**挂在回调成功上——这也是兜底收在
`IoBase` 而非各后端各写一份的原因。

## 为什么以「登记」为准

「已登记」与「已向订阅者宣告过」是等价的，这一点由两件事保证：

1. `AddConnect` 先插入连接表、后通知，且通知不再抛穿；故**登记在通知之前，且通知一旦开始就必定被投递**。
2. `AddConnect` 抛异常只可能来自插入之前的两步（`WriteLog` 与 `connects_.insert`）；故**抛出即未登记、即未宣告**。

未登记而调用 `RemoveConnect` 有两处来源，二者都从未宣告过，都不该通知：

- **插入失败**（`std::bad_alloc`）：客户端确认路径的补偿走 `ShmClient::EstablishConfirmedConnection` 的 `catch`，
  它调 `RemoveConnect` 抹掉可能已插入的对象；插入失败时这一步不通知，正好对应该连接从未宣告。
- **从未 `AddConnect`**：`TcpSelectClient::CheckConnect` 与 `TcpEpollBase` 的连接失败路径，
  连接对象建在 `connectings_` 里、从未进过 `connects_`，拆除时只为把它还回池。
  此前这一步会向订阅者补发一次无配对的 `OnDisConnect`（`Protocol::OnDisConnect` 查不到 `PackageReader`
  即静默略过，再到应用层就是一次「没连上却断开」的通知）。

## 实现落点

`IoBase.cpp` 里一个文件局部的 `NotifySubscriberSafely` 模板（`try` / `catch (const std::exception&)` / `catch (...)`，
名称与 `SessionIdType` 由调用方传入，回调本身以泛型可调用对象传入），由 `AddConnect` 与 `RemoveConnect` 各调一次；
`RemoveConnect` 在通知前先取一次锁判断该会话是否登记在册。

三个后端的收包点经 `IoBase::NotifySubscriberRecvSafely`（一个 `protected` 成员）进入同一份兜底，
故整条链路上 `catch` 只有一处实现：泛型可调用对象是 `IoBase.cpp` 文件局部的实现细节，
`IoBase.h` 上只多一个非模板成员，既不用把 `Logger.h` 拉进公开头文件，也避开了 `std::function`
在收包热路径上的堆分配。

`ShmClient::EstablishConfirmedConnection` 的 `catch` 一字未改（仍走 `RemoveConnect`）：此时该连接未登记、
故不通知，而 `Deallocate` 照常执行 —— `~ShmConnect` 自会把槽位由 `Connected` 置成 `DisConnected`，
把钥匙交回服务端，与既有的两笔撤回语义一致（见 `docs/shm-channel-and-connect-model.md` 第四节）。

## 取舍：回调抛异常时连接保留

`OnConnect` 抛异常的连接**不会**被拆掉：通知被投递过（异常发生在回调内部），连接照常供收发，
拆除时也照常收到配对的 `OnDisConnect`。选它而不是「抛即拆」的理由是四个调用点的处境不同——
三个没有 `try` 的调用点无法感知失败，若在 `AddConnect` 内部拆掉再让异常穿出，连接对象已经归还而调用方
仍以为登记成功（要表达失败就得改 `AddConnect` 的签名，属公开 API 变更）；而把它留在原地，
订阅者仍可在配对的 `OnDisConnect` 里清理自己未完成的初始化。

## 未覆盖与未决

- **`Protocol::OnRecv` 的取包循环仍非异常安全**：`Protocol` 自身也是 `IoSubscriber`，本文的兜底只保证
  异常不穿出 IO 层；而 `Protocol::OnRecv` 内部的取包 `while` 循环一旦被 `ProtocolSubscriber` 抛出打断，
  已取出但尚未派发的包会滞留在 reader 里，直到下一批数据到达才被一并处理（顺序不变，但延迟不定）。
  该点在 `Protocol` 一层，已单独登记。
- **重复 `RemoveConnect` 仍会二次归还连接对象**：第二次调用时该连接已不在册，故不再通知，
  但 `Deallocate()` 照常执行。未验证该形态是否可达。
- 回调抛异常后连接保留，订阅者须自负其未完成初始化的后果；本契约只保证它仍会收到配对的 `OnDisConnect`。

## 相关测试

`test/unittest/Network/IoSubscriberNotificationTest.cpp` 四条，均以只含计数器的订阅者探针与
自造的 `IoBase` 子类钉住判据（不挂共享内存、不依赖时序）：

| 用例 | 输入 | 输出 |
| --- | --- | --- |
| `AnOnConnectFailureIsContainedAndTheConnectionStaysRegistered` | `OnConnect` 抛异常 | 不传播；`OnConnect` 被调用 1 次；连接仍在册；未归还对象 |
| `AnOnDisConnectFailureDoesNotSkipTheTeardown` | `OnDisConnect` 抛异常 | 不传播；`OnDisConnect` 被调用 1 次；连接已出册；对象已归还 |
| `AnOnRecvFailureIsContainedAndDoesNotSkipWhatFollows` | `OnRecv` 抛异常 | 不传播；`OnRecv` 被调用 1 次；`NotifySubscriberRecvSafely` 返回其调用方 |
| `AnUnregisteredConnectionIsReturnedWithoutADisConnectNotification` | 对未登记的连接调 `RemoveConnect` | `OnConnect` 0 次、`OnDisConnect` 0 次；对象已归还 |

第 1、2、4 条经 A/B 实测：撤掉 `IoBase.cpp` 的改动重跑，前两条以「C++ exception … thrown in the test body」失败、
第四条以 `DisConnectCount` 期望 0 实得 1 失败。第 3 条不适用这一手（它调的 `NotifySubscriberRecvSafely` 本身
就是本批新增的，撤掉即无法编译），它的 A/B 由下面那条真后端用例承担。

`test/unittest/Network/ShmInitTest.cpp` 的 `AThrowingOnRecvNeitherEscapesTheIoLoopNorStopsLaterPayloads`
在真后端上钉住同一件事：服务端订阅者只在第一次收包抛异常，客户端连发 100 个 5 字节包，
每发一个驱动一次服务端 —— 期望服务端收到恰好 100 次、最后一次长度 5、全程无 `OnDisConnect`。
它同时覆盖了「异常不穿出 IO 循环」与「抛过异常的连接照常收后续包」；撤掉 `ShmBase.cpp` 的单行改动重跑，
该用例以同样的 `C++ exception … thrown in the test body` 失败。
