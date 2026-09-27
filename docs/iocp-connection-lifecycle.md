# IOCP 连接建立与收尾的所有权约定

本文集中记录 Windows IOCP 后端在**连接建立（`AcceptEx` / `ConnectEx`）与连接收尾**上的所有权与内核契约。这些约定来自 Win32 内核行为与 Winsock 的隐含要求，
无法从类型或命名读出，而本仓代码不写注释（Harness §4：命名即文档），故一并写在这里。对应实现见 `src/Network/Tcp/TcpIocp/`。

## 一、在途的建立类请求为什么不能提前归还

- 内核在**投递完成包之前**会写回 `OVERLAPPED` 的内存（`Internal` / `InternalHigh`）。
- `CancelIoEx` 是异步的：它返回时，被取消的那个 IRP 可能尚未收尾。
- 因此一块 `MyOverlapped` 只要提交成功，就不得归还对象池，直到它的完成包被取回。

「提交成功」的两种形态都算：`AcceptEx` / `ConnectEx` 返回 `TRUE`（当场发起），或返回 `FALSE` + `ERROR_IO_PENDING`（挂起）。
判据一律写成 `!ret && lastError != ERROR_IO_PENDING` 才是失败。

若提前归还，池会把这块内存再发给别人（`TcpIocpBase::Send` 与 `PostRecv` 都从池领 `MyOverlapped`），内核的写回就落在别人的对象上。

## 二、登记表的四个时点

- **提交成功**：登记该 `OVERLAPPED` —— `TcpIocpClient::PostConnect`、`TcpIocpServer::PostAccept`。
- **正常完成**：注销 —— `TcpIocpClient::OnConnectComplete`、`TcpIocpServer::OnAcceptComplete`。
- **被取消**：完成端口返回 `ERROR_OPERATION_ABORTED` 时注销，并就地整对归还 —— `TcpIocpBase::HandleTcpEvent`。
- **对象析构**：把仍登记着的逐条取消、取回、归还 —— `TcpIocpBase::ReleaseAllInFlightConnectRequests`。

登记表（`inFlightConnectRequestList_` 与它的互斥量）只为析构路径与取消路径服务，收发数据的热路径不触碰它。

## 三、取消必须用发起时那个句柄

`CancelIoEx(hFile, lpOverlapped)` 只认发起该 IRP 的那个句柄。用错句柄只会得到 `ERROR_NOT_FOUND`，请求照旧在途，完成包永远等不来。两类建立请求的 IRP 挂在不同 socket 上：

- `EventAccept`：IRP 挂在**监听 socket** `socket_` 上。
- `EventConnect`：IRP 挂在**连接 socket** `TcpIocpConnect::SocketId` 上。

## 四、取回策略：限时轮询，取不回就漏着

析构路径取消之后，以 10 ms 为间隔、最多 1000 ms 轮询完成端口，取到**指针相等**的那一条才算取回。两个取舍：

- 取到的若不是本条（取消前它已完成，或别的残留包），直接丢弃。该函数只在析构路径上跑：框架约定要求先停并 `Join` IO 线程、再销毁 IO 对象，此时没有别的取包人。
- 限时内取不回，就**宁可把这一对对象漏着**并记一条 `Warning`（`Reap Cancelled Connect Request Timeout, Leak It.`）。漏一个对象的代价有界；归还一块内核还可能写回的内存，会写坏别人。

## 五、连接收尾的重复断开竞态（`WSAENOTCONN`）

一条连接上挂着两个收包请求（`OnAcceptComplete` / `OnConnectComplete` 各投两次 `PostRecv`）。对端异常断开时，两个完成包都会带 `ERROR_NETNAME_DELETED`（64）走到 `PostDisConnect`：

- 先到的那个把 `DisconnectEx` 发出去（返回 `FALSE` + `ERROR_IO_PENDING`，尚在挂起）。
- 后到的再调只会拿到 `WSAENOTCONN`（10057）。
- **会话收尾（`OnDisConnectComplete` → `RemoveConnect` → 归还连接对象）只归先到的那次**；后到的那次只把自己的 `overlapped` 还回池就走。
- 若后到的那次也走收尾，就会拿一个已经归还给池的 `TcpIocpConnect` 再 `Deallocate` 一次，日志上表现为 `Socket:-1` 与 `Errno:10057`。

后到那次记的日志是 `DisConnectEx Skipped, Socket Already Disconnecting.`，属**预期**噪声，不是故障。

## 六、`addrinfo` 的释放点

`TcpUtility::GetAddrinfo` 的结果（`addressInfo_` / `clientLocalAddressInfo_`）曾经无人释放，每次调用直接覆写指针。现在的释放点与各自的安全依据：

- `TcpBase::~TcpBase` 与 `SocketNotify::~SocketNotify` 释放 `addressInfo_`：析构顺序保证它排在 `TcpIocpBase` 取回在途请求之后（基类析构晚于派生析构体）；其余三个 TCP 后端的 `connect()` 是同步调用。
- `TcpIocpClient::~TcpIocpClient` 释放 `clientLocalAddressInfo_`：它只被 `WSASocket`（取 family 值）与 `TcpUtility::Bind`（同步调用）使用，没有任何在途 IRP 引用它。
- `TcpIocpClient::ConnectToServer` 在重取之前释放上一个 `addressInfo_`：重连只在上一笔连接请求已落定之后才调到它——`TcpBase::TryAutoReconnect` 要求 `autoConnectPending_ == false` 且 `connects_` 为空，而 `AddConnect` / `RemoveConnect` 会复位该标志。此时没有在途请求还引用那块地址。

## 七、「首连失败不报错」是有意的

`TcpIocpClient::Init()` 无条件返回 `true`、吞掉 `PostConnect` 的结果，与 `TcpBase::Init()` 的客户端分支同构：首连同步失败时复位 `autoConnectPending_`，交由 IO 循环的 `TryAutoReconnect`（3 秒一次）兜底。

故 `Init()` 的返回值含义是「初始化完成、重连机制已就位」，**不是**「已经连上」。调用方不应把 `Init()` 成功当作连接已建立。

## 八、相关测试与其输入输出

`test/unittest/Network/TcpIocpTest.cpp`（Windows-only）。

- `ServerAbandonedPendingAcceptsKeepProcessHandleCountFlat`
  - 输入：`tcp://127.0.0.1:0` 经 `IoFactory::CreateIo` + `IoBase::Init` 建/毁 20 轮；不跑 IO 线程，一条连接都不进。
  - 输出：Error 级日志 0 条；进程句柄增长 ≤ 2。
- `AbandonedPendingConnectKeepsProcessHandleCountFlat`
  - 输入：`tcp://192.0.2.1:9`（TEST-NET-1）同上 20 轮。
  - 输出：Error 级日志 0 条；进程句柄增长 ≤ 2（漏一个请求就是漏一个 socket）。
- `RepeatedlyRejectedConnectAttemptsKeepProcessHandleCountFlat`
  - 输入：`tcp://127.0.0.1:0` 同上 20 轮。
  - 输出：Error 级日志**恰 20 条**（每次都被同步拒绝）；进程句柄增长 ≤ 2。

三个地址的取舍：

- 目的端口 0 让 `ConnectEx` **当场拒绝**（返回 `false` 且 error 非 `ERROR_IO_PENDING`），请求根本进不了登记表。
- TEST-NET-1（`192.0.2.0/24`）是 IANA 保留、从不路由的网段，`ConnectEx` 一直挂在 `ERROR_IO_PENDING`，请求进得了登记表且不会落定。
- 两者对 `ErrorLogCount` 的断言方向相反（前者恰 20、后者为 0），失败模式一变就变红。

敏感性对照（各做一遍、随即恢复）：

- 撤掉 `PostConnect` 里的登记，客户端用例变红，句柄涨 20。
- 撤掉 `PostAccept` 里的登记，服务端用例变红，句柄涨 100（20 轮 × 每轮 5 个 accept 请求）。

探针期的一个坑：`Logger` 未 `Init` 时 `GetWriteLogFunc()` 为空、`WriteErrorLog` **整条被跳过**，故这类用例必须先 `SetExternLogger` 装替身日志器，才数得到日志。

## 九、已知未覆盖

- 取回超时那条「漏着并记 `Warning`」的分支，无用例可量化触发。
- `WSAENOTCONN` 子分支目前只由 `tools/step_e2e.py --io-model Iocp` 覆盖（日志里那一条预期的 `Warning` 即它）。
