# IO 层订阅者通知契约

本文定义 `IoBase` 向 `IoSubscriber` 投递连接事件时的契约：通知与登记的关系、通知之间的配对、
以及回调抛异常时由谁承担。四个后端（Tcp Select / Tcp Epoll / Tcp Iocp / Shm）共用 `IoBase` 的登记与拆除，
故本契约对四者一致；收包通知（`OnRecv`）不在本文范围内，见末节。

## 契约

- **回调不得让异常穿出 IO 层**。`OnConnect` / `OnDisConnect` 任一回调抛出的异常由 `IoBase` 收住并记一条
  `Error` 日志（含异常自身的 `what()`），不再向调用方传播。四个后端的 `AddConnect` 调用点
  （`ShmServer::Accept`、`TcpEpollBase`、`TcpSelectClient`、`TcpIocpClient`）里只有 `ShmClient` 一侧有 `try`，
  故此前一个订阅者 bug 会从其余三处穿出 `HandleIoEvent`、直达应用主循环。
- **每个连接恰好一次 `OnConnect`、恰好一次 `OnDisConnect`**。配对以**登记**（`connects_` 里有该会话）为准。
- **拆除动作不依赖通知完成**：连接表摘除与连接对象归还排在通知之后，但通知抛异常不再能跳过它们。
- **从未宣告过的连接不产生 `OnDisConnect`**：`RemoveConnect` 遇到未登记的连接只归还对象、不通知。

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

`IoBase.cpp` 里一个文件局部的 `NotifySubscriberSafely`（`try` / `catch (const std::exception&)` / `catch (...)`），
由 `AddConnect` 与 `RemoveConnect` 各调一次，回调本身的差异以成员函数指针传入；
`RemoveConnect` 在通知前先取一次锁判断该会话是否登记在册。

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

- **`OnRecv` 未加兜底**：三处调用点（`ShmBase::DoRecv`、`TcpBase`、`TcpIocpBase`）仍会把异常抛出 IO 循环。
  它不是配对问题（收包通知次数不固定），已单独登记。
- **重复 `RemoveConnect` 仍会二次归还连接对象**：第二次调用时该连接已不在册，故不再通知，
  但 `Deallocate()` 照常执行。未验证该形态是否可达。
- 回调抛异常后连接保留，订阅者须自负其未完成初始化的后果；本契约只保证它仍会收到配对的 `OnDisConnect`。

## 相关测试

`test/unittest/Network/IoSubscriberNotificationTest.cpp` 三条，均以只含计数器的订阅者探针与
自造的 `IoBase` 子类钉住判据（不挂共享内存、不依赖时序）：

| 用例 | 输入 | 输出 |
| --- | --- | --- |
| `AnOnConnectFailureIsContainedAndTheConnectionStaysRegistered` | `OnConnect` 抛异常 | 不传播；`OnConnect` 被调用 1 次；连接仍在册；未归还对象 |
| `AnOnDisConnectFailureDoesNotSkipTheTeardown` | `OnDisConnect` 抛异常 | 不传播；`OnDisConnect` 被调用 1 次；连接已出册；对象已归还 |
| `AnUnregisteredConnectionIsReturnedWithoutADisConnectNotification` | 对未登记的连接调 `RemoveConnect` | `OnConnect` 0 次、`OnDisConnect` 0 次；对象已归还 |

三条均经 A/B 实测：撤掉 `IoBase.cpp` 的改动重跑，前两条以「C++ exception … thrown in the test body」
失败、第三条以 `DisConnectCount` 期望 0 实得 1 失败。
