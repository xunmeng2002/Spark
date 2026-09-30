# IO 层订阅者通知契约

本文定义 `IoBase` 向 `IoSubscriber` 投递通知时的契约：通知与登记的关系、通知之间的配对、
以及回调抛异常时由谁承担。四个后端（Tcp Select / Tcp Epoll / Tcp Iocp / Shm）共用 `IoBase` 的登记与拆除，
故本契约对四者一致；收包通知（`OnRecv`）与连接通知共用同一份兜底，差别只在调用点，见「收包通知」一节。
`Protocol` 自身也是 `IoSubscriber`，它在 `OnRecv` 之后还要把包转交给 `ProtocolSubscriber::OnMessage`，
那一层另有取包循环自己的责任，见「取包通知」一节。登记的边界另有一条：同一会话号第二次登记会怎样，
见「重复会话号」一节。

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

## 取包通知

`Protocol` 自己也是 `IoSubscriber`，上一节只保证异常不穿出 IO 层。异常到了 `Protocol` 这一层之后，
`Protocol::OnRecv` 的取包 `while` 循环还有自己的一份责任——它在这个循环里向
`ProtocolSubscriber::OnMessage` 移交包的所有权，故三件事必须一起定：抛出怎么处理、那一帧归谁、
没订阅者时归谁。

- **一帧抛出不打断这一批**。`OnMessage` 抛出的异常由 `Protocol` 收住并记一条 `Error`（含 `what()`），
  循环继续取下一帧。此前异常直接穿出循环，同一段字节里其后的帧留在 reader 里、等下一次流量才被解析
  （实测：两帧一批，第 1 帧抛出后 reader 里仍压着 76 字节，正是第 2 帧的长度）——顺序不变但延迟不定，
  对「对端发一批就等回话」的应用协议足以形成死锁。
- **抛出时 `Protocol` 不归还那一帧**。`OnMessage` 的形参是 `Package* ownedPackage`，即所有权移交；
  抛出时 `Protocol` 无法分辨订阅者是否已归还，替它归还就是把同一指针二次推入池的空闲链
  （`ObjectPool::Deallocate` 对已归还指针没有防护）。故归 subscriber 自理。
- **没有订阅者时由 `Protocol` 归还**。`subscriber_ == nullptr` 时包同样不投递，但它从未交出去过，
  必须由 `Protocol` 放回池；此前这一帧随指针出作用域丢掉，是一次静默泄漏。
  `UnSubscribe()` 之后会话照旧连着，该分支可达。

三条合起来是一句话：**没交出去的要归还，交出去的不管**。

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

## 重复会话号：留先来的那个

`connects_` 以会话号为键，故同一会话号第二次登记时 `insert` 不会覆盖先来的表项。此前这一步的返回值被丢弃，
后来的连接**既不进表、也不失败**：它照常被调用方使用，而 `RemoveConnect` 按会话号抹掉的是**在册的那条**，
在册的那条从此不在表里、也不会再被归还（池槽与套接字一并滞留到进程结束）。同样的形态在 `Protocol` 一侧
多一个后果：那一次为重复会话号取出的 `PackageReader` 无处归还。

两处现在都把这一步摆到明处：

- **`IoBase::AddConnect`** 插入未成功即记一条 `Error` 并返回：不向订阅者宣告（与该层「登记在通知之前」一致），
  **也不代调用方归还连接对象**——`TcpIocpServer::OnAcceptComplete` 这类调用点在登记之后还要接着用它投递接收，
  在拒绝处就地归还等于制造悬垂指针。
- **`Protocol::OnConnect`** 遇到同一会话号已有 `PackageReader`：把新取的那个还回池、记一条 `Error` 后返回，不宣告。

两处都只把「静默」变成「记 `Error` 并拒绝」，**会话号本身的回绕没有改**：`IoBase::GetSessionId` 按
`毫秒 * 100 + (++序号) % 100` 取号，同一毫秒内第 101 个连接会拿到与在册连接相同的号，而一次事件循环把
backlog 收完即可批量接入。故它仍需一次专门的决定（见 `PROGRESS.md` 的 ❓ 条）。被拒的连接若由调用方改用
`RemoveConnect` 收尾，仍会按会话号抹掉在册的那条——这是「拒绝」而非「就地拆」留下的已知界限。

## 实现落点

兜底只有一份实现，即 `src/Network/Io/SubscriberNotification.h` 里的 `NotifySubscriberSafely` 模板
（`try` / `catch (const std::exception&)` / `catch (...)`，名称与 `SessionIdType` 由调用方传入，
回调本身以泛型可调用对象传入）。它是模块内部的头文件（`src/Network` 是模块的 PRIVATE 包含目录），
不进公开面：`IoBase.cpp` 与 `Protocol.cpp` 各包含一次，因此三层通知（连接、收包、取包）共用同一个 `catch`。

- `IoBase.cpp` 的 `AddConnect` 与 `RemoveConnect` 各调一次，后者的通知前先取一次锁判断该会话是否登记在册。
- 三个后端的收包点经 `IoBase::NotifySubscriberRecvSafely`（一个 `protected` 成员）进入同一份兜底。
  泛型可调用对象是文件局部的实现细节，`IoBase.h` 上只多一个非模板成员，既不用把 `Logger.h`
  拉进公开头文件，也避开了 `std::function` 在收包热路径上的堆分配。
- `Protocol::OnRecv` 的取包循环直接把这个模板用在 `OnMessage` 上。

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

- **`OnMessage` 抛出时那一帧归 subscriber 自理**：本文只保证 `Protocol` 不再二次归还，也保证这一批里
  其后的帧照常派发；订阅者若在自己的 `OnMessage` 里抛出且未在抛出前 `Deallocate`，那一帧就漏了，
  本契约不代偿（见「取包通知」一节的理由）。
- **重复 `RemoveConnect` 仍会二次归还连接对象**：第二次调用时该连接已不在册，故不再通知，
  但 `Deallocate()` 照常执行。未验证该形态是否可达。
- **会话号的回绕未修**：`IoBase::AddConnect` 只是拒绝并记 `Error`（见「重复会话号」一节），
  `GetSessionId` 仍会在同一毫秒内发出重复号；被拒的连接若改用 `RemoveConnect` 收尾，
  按会话号抹掉的还是在册的那条。这两点都留待专门的决定。
- 回调抛异常后连接保留，订阅者须自负其未完成初始化的后果；本契约只保证它仍会收到配对的 `OnDisConnect`。

## 相关测试

`test/unittest/Network/IoSubscriberNotificationTest.cpp` 五条，均以只含计数器的订阅者探针与
自造的 `IoBase` 子类钉住判据（不挂共享内存、不依赖时序）：

| 用例 | 输入 | 输出 |
| --- | --- | --- |
| `AnOnConnectFailureIsContainedAndTheConnectionStaysRegistered` | `OnConnect` 抛异常 | 不传播；`OnConnect` 被调用 1 次；连接仍在册；未归还对象 |
| `AnOnDisConnectFailureDoesNotSkipTheTeardown` | `OnDisConnect` 抛异常 | 不传播；`OnDisConnect` 被调用 1 次；连接已出册；对象已归还 |
| `AnOnRecvFailureIsContainedAndDoesNotSkipWhatFollows` | `OnRecv` 抛异常 | 不传播；`OnRecv` 被调用 1 次；`NotifySubscriberRecvSafely` 返回其调用方 |
| `AnUnregisteredConnectionIsReturnedWithoutADisConnectNotification` | 对未登记的连接调 `RemoveConnect` | `OnConnect` 0 次、`OnDisConnect` 0 次；对象已归还 |
| `ARepeatedSessionIdIsRefusedWithoutASecondConnectNotification` | 同一会话号登记两次 | `OnConnect` 被调用 1 次；在册的是先来的那个；两个对象都未归还 |

第 1、2、4 条经 A/B 实测：撤掉 `IoBase.cpp` 的改动重跑，前两条以「C++ exception … thrown in the test body」失败、
第四条以 `DisConnectCount` 期望 0 实得 1 失败。第 3 条不适用这一手（它调的 `NotifySubscriberRecvSafely` 本身
就是本批新增的，撤掉即无法编译），它的 A/B 由下面那条真后端用例承担。第 5 条同样经 A/B：撤掉
`IoBase.cpp` 里那段拒绝重跑，它以 `ConnectCount` 期望 1 实得 2 失败（重复那条被静默宣告了第二次）。

`test/unittest/Network/ShmInitTest.cpp` 的 `AThrowingOnRecvNeitherEscapesTheIoLoopNorStopsLaterPayloads`
在真后端上钉住同一件事：服务端订阅者只在第一次收包抛异常，客户端连发 100 个 5 字节包，
每发一个驱动一次服务端 —— 期望服务端收到恰好 100 次、最后一次长度 5、全程无 `OnDisConnect`。
它同时覆盖了「异常不穿出 IO 循环」与「抛过异常的连接照常收后续包」；撤掉 `ShmBase.cpp` 的单行改动重跑，
该用例以同样的 `C++ exception … thrown in the test body` 失败。

`test/unittest/Network/ProtocolTest.cpp` 三条，钉在 `Protocol` 一层。用例装一个自造的 `IoBase` 当 `ioBase_`、
把 `PackageReader` 插进受保护的 `sessionPackageReaders_`，故取包循环可在不挂共享内存、不依赖时序的情况下
逐帧驱动；包由 `PackageAccountingFactory` 的 `CreatePackage` 发放记名的 `LedgeredPackage`，
交出去几个、`Deallocate` 收回来几个两数一比即知：

| 用例 | 输入 | 输出 |
| --- | --- | --- |
| `AThrowingOnMessageNeitherEscapesTheRecvLoopNorStrandsTheRestOfTheBatch` | 两帧一批，订阅者在第 1 帧抛出 | 不传播；两帧都派发（末帧为第 2 帧）；发放 2 / 归还 2 |
| `APackageParsedWithoutASubscriberIsReturnedToTheFactory` | 无订阅者，送入一帧 | 不投递；发放 1 / 归还 1 |
| `ARepeatedOnConnectForOneSessionKeepsTheFirstReaderAndReturnsTheSecondToThePool` | 同一会话号 `OnConnect` 两次 | 第二个 reader 已还回池（再取到的还是同一个槽）；在册的仍是先来的那个 |

第 3 条量「已还回池」用的是池的空闲链是后进先出：先把一个刚归还的 reader 取回、再触发那次重复登记，
之后能取到同一个槽就说明中间那个 reader 确实还了回去（`ProtocolTest.cpp` 内 `EXPECT_EQ(nextReader, recycledReader)`）。
它经 A/B 实测：撤掉 `Protocol.cpp` 的改动重跑，该断言以两个不同地址失败（池里没还回去的那个）。

第 1 条经 A/B 实测：把 `Protocol.cpp` 里 `OnMessage` 那行改回直接调用重跑，该用例以
「C++ exception … thrown in the test body」失败。第 2 条同理——加 `else` 分支之前，
它以 `DisposedCount` 期望 1 实得 0 失败。
两条的「归还 2 / 归还 1」同时也钉住了另一半：`Protocol` 没有替订阅者二次归还。

第 1 条**没有**断言「滞留」这一半（抛出已经把控制流带走，断言读不到），故它由一只临时探针单独量过：
在旧代码下把同一批两帧喂进去，异常照常穿出，`MessageCount` 停在 1 而 reader 里仍压着 76 字节
（正是第 2 帧的长度），随后一次 `OnRecv(sessionId, frames, 0)` 才把它取出去（`MessageCount` 变 2、
序号为第 2 帧）。探针已删，不留在仓库里。
