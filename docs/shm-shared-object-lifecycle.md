# Shm 共享内存对象的命名、映射长度、残留与复用

本文记录共享内存后端（`src/Network/Shm/`）在**共享内存对象的名字派生、映射长度公式、同名残留对象的复用语义、以及对象的创建/销毁时点**上的约定。
这些约定来自 POSIX / Win32 的共享内存语义与两端必须**逐字同名**的隐含要求，无法从类型或命名读出，
而本仓代码不写注释（Harness §4：命名即文档），故一并写在这里。

## 一、名字是怎么派生出来的

地址串在 `IoFactory::CreateIo` 里按 scheme 分派（`strncmp(address, "shm", 3)` 后跳过 `"shm://"`），再经 `IoBase::IoBase` 调 `ParseAddress` 在**第一个 `:`** 处切分：
前者进 `address_`、后者进 `port_`。`ShmBase` 构造函数随即：

- `shmName_ = address_` —— 共享内存对象名与信号量名的**共同前缀**。
- 用 `StepUtility::ParseInteger(port_, maxConnectSize_)` 解析连接数，解析失败置 0；`Init()` 的第一道门禁 `IsConnectSizeAllowed()` 与它配合，把 0 拒掉。

于是 `shm://TestShm:4` → `shmName_ = "TestShm"`、连接数 4、映射长度 4 × 1 MiB × 2。
**对象名不含连接数**：同一名字下改连接数不会换对象，只会撞上下面第三节的复用检查。

信号量名在同一前缀上派生：连接建立计数用 `shmName_ + "SemConnect"`，第 i 号通道用 `shmName_ + "Sem" + to_string(i)`。
通道数与连接数一一对应，故服务端与客户端**必须给出同一个连接数**，否则双方创建的信号量集合不同、`sems_[index]` 会越界或错配。

## 二、映射长度公式与 2048 处的回绕

`GetSharedMemoryMappingSize()` 返回 `ShmBufferSize * maxConnectSize_ * 2`，`ShmBufferSize` 是 `1 MiB`（`ShmConnect.h`）。
乘 2 是因为每号连接占两个通道（收/发各一）。整个乘法在 `unsigned` 里算，所以：

| 连接数 | 期望长度 | 实际算出 | 结果 |
| ---- | ---- | ---- | ---- |
| 2047 | 4094 MiB | 4094 MiB | 正常（上限） |
| 2048 | 4096 MiB | **0** | 映射必然失败（响亮） |
| 2049 | 4098 MiB | **2 MiB** | 静默欠映射（危险） |

危险的是 2049 及以上那一档：`4 GiB` 回绕成一个小正数，`CreateFileMappingA` / `mmap` 都**会成功**，
随后 `Init()` 在服务端按 `GetSharedMemoryMappingSize()` 清零（清的是这个小值，还好），
但 `ShmBuffer::Write` 按**连接数**算通道偏移，写到映射之外。
故 `MaxSharedMemoryConnectSize = UINT_MAX / (ShmBufferSize * 2) = 2047`，由 `IsConnectSizeAllowed()` 在创建任何 OS 资源之前拒掉，
日志为 `Shm ConnectSize:%u Exceeds Mapping Limit:%u`。

回绕在**服务端与客户端同样**发生，客户端 `mmap` 之后不会清零，首帧读写才踩到越界，故不能指望「映射失败」兜住 —— 必须显式校验。

## 三、同名残留对象：服务端独占创建 + 复用回退

双方的非对称分工是：**服务端创建、客户端打开既有对象**。

| 平台 | 服务端 | 客户端 |
| ---- | ---- | ---- |
| Windows | `CreateFileA(..., CREATE_NEW, ...)` | `OpenFileMappingA` |
| Linux | `shm_open(O_CREAT \| O_EXCL \| O_RDWR, 0666)` | `shm_open(O_RDWR, 0666)` |

独占创建意味着**上一个进程留下的同名对象会让新的服务端永久起不来**（Windows 是 `ERROR_FILE_EXISTS`，Linux 是 `EEXIST`）。
因此服务端在此处补了与 `Sem` 同构的回退（`Sem` 早就是「先独占创建、失败即打开」）：命中「对象已存在」才改为打开，其余错误照样失败。

- Windows：`ERROR_FILE_EXISTS` → `CreateFileA(..., OPEN_EXISTING, ...)`，日志 `Shm Object Exists, Reuse It.`。
- Linux：`EEXIST` → `shm_open(O_RDWR, 0666)`，同一条日志。
- **只有服务端有这条回退**，客户端本来就是「打开既有」，没有可回退的东西。

## 四、复用的两平台语义不同，故校验方式不同

复用是**接受别人留下的对象**，而长度必须正好是本节公式值，故复用前必须确认尺寸。两个平台能用的手段不一样：

- **Windows 无截断风险**：`CreateFileMappingA` 遇到已存在的同名对象时返回该对象的句柄，**`dwMaximumSize` 参数被忽略**。
  于是尺寸不符会在随后的 `MapViewOfFile(..., GetSharedMemoryMappingSize())` 上**响亮失败**（视图不可能大于对象）。
  故 Windows 侧无需额外校验，靠既有的失败出口即可。
- **Linux 必须自己校验**：`ftruncate` 在复用对象上是**截断**语义 —— 使用它会把还活着的对端脚下的映射截短（对端下次触碰越界页即 `SIGBUS`）。
  故复用时不截断，改 `fstat` 读 `st_size`：**够用就复用且不动尺寸，不够则拒绝**（`Shm Object Exists Smaller Than Needed. ObjectSize:%lld, NeededSize:%u`）。
  新建对象仍走 `ftruncate`（把新对象从 0 撑到目标长度）。

这一条不是理论担忧：把 `fstat` 校验撤掉、同时不在复用路径上 `ftruncate`，`ShmInitTest.Init_RejectsReusedShmObjectSmallerThanNeeded` **当场 `SIGBUS`**
（4 MiB 视图落在 2 MiB 对象上，服务端 `Init()` 的清零先踩到）；换成「复用也 `ftruncate`」这种看似更顺手的写法，该用例**变红但不再崩**
（对象被悄悄撑大、`Init()` 返回了 `true`）。两种撤法都试过，前者崩、后者红。

另注：**客户端也会走到那条 `ftruncate`**（与改动前逐字相同）。它以 `O_RDWR` 打开、长度与期望相同，故是空操作；
但若两端给出**不同的连接数**，客户端的这次 `ftruncate` 会把服务端对象截短 —— 这是既有的、未加防护的隐患（见第九节）。

## 五、fd 在 `mmap` 之后即关闭

POSIX 规定：`mmap` 成功之后 `close(fd)` 不影响映射的有效性（映射自己持引用；对象在最后一个映射/描述符消失后才真正销毁）。
故 `LinuxInit()` 在 `mmap` 成功后立刻 `close(fd)`。

改动前这一步**完全不存在**：`LinuxInit()` 在所有出口（成功、`ftruncate` 失败、`mmap` 失败）都不关 `fd`，即每次 `Init()` 漏一个描述符。
除泄漏本身之外，它还改变对象的生命周期 —— **描述符是引用**，未关的描述符会让 `shm_unlink` 只是摘掉名字、对象活到进程退出；
现在关掉之后，析构路径的 `shm_unlink` 才真的把对象销毁。本次一并补齐了 `fstat` / `ftruncate` / `mmap` 三个失败出口的 `close(fd)`。

## 六、销毁时点，以及一条已知缺口

服务端对象由**服务端进程的析构函数**销毁：Windows 走 `UnmapViewOfFile` + `CloseHandle` + `DeleteFileA`，Linux 走 `munmap` + `shm_unlink`；
客户端只解映射、不删对象（以 `serverType_` 区分）。信号量先于共享内存释放。

**缺口**：析构里的「删除对象」以 `serverType_ == Server` 为唯一判据，不看本次 `Init()` 是否真的创建/复用了对象。
若 `Init()` 因尺寸超限、或 Linux 上因「复用对象偏小」而失败，这个服务端对象**并不属于本进程**，析构却仍会 `shm_unlink` / `DeleteFileA` 掉它 ——
跨进程场景下会把别人的对象摘掉。测试里表现为 `shm_unlink: No such file or directory` 之类的噪音（自己那一份已被前一个对象删过）。
该缺口未修，登记在 `PROGRESS.md` 的 ❓ 区。

同处另更正一处日志文案：`munmap` 失败的分支此前打的是 `perror("shm_unlink")`，现改为 `perror("munmap")`（纯文案，行为未变）。

## 七、与冒烟的关系

`Shm` 冒烟（`TestServer.exe Shm` + `TestClient.exe Shm`，地址 `shm://TestShm:4`）的既有规矩是**每轮先杀掉两端进程、再删掉 `TestShm`**。
该规矩的理由现在只剩一半：

- 旧理由「不删则服务端 `CREATE_NEW` 失败、整轮不跑」**已随复用回退消失**；实测保留残留的那一轮两端 0 ERROR、客户端照样跑完 10000 次回显，
  服务端日志恰有 1 条 `Shm Object Exists, Reuse It.`。
- 仍成立的理由是**信号量与 credit**：残留进程持有的具名信号量句柄会让新进程与旧进程争抢同一份 credit（曾是 3 条 `Sem UnLock Failed.` 的来源）。
  杀掉两端即可回收句柄，故「先杀进程」不可省，「先删文件」可以省略。

冒烟的完整结果与命令见本批的变更摘要。

## 八、相关测试与其输入输出

`test/unittest/Network/ShmInitTest.cpp`（四档构建均跑）。

- `Init_RejectsNonNumericConnectSize`
  - 输入：`shm://…:abc`。
  - 输出：`Init() == false`（连接数解析失败置 0，被门禁拒）。
- `Init_RejectsZeroConnectSize`
  - 输入：`shm://…:0`。
  - 输出：`Init() == false`。
- `Init_AcceptsSmallestPositiveConnectSize`
  - 输入：`shm://…:1`。
  - 输出：`Init() == true`，映射 2 MiB。
- `Init_RejectsConnectSizeWhoseMappingWouldWrap`
  - 输入：`shm://…:2049`。
  - 输出：`Init() == false`；**改动前返回 `true`**（长度回绕成 2 MiB 的静默欠映射），故该用例是这条门禁的判据。
- `Init_ReusesShmObjectLeftByAPreviousRun`
  - 输入：同一地址上两个先后 `Init()` 的服务端对象。
  - 输出：第一个 `true`、第二个 `true`（走了复用回退）；改动前第二个 `false`。
- `Init_RejectsReusedShmObjectSmallerThanNeeded`
  - 输入：同一名字，先 `:1`（2 MiB）后 `:2`（4 MiB）。
  - 输出：第一个 `true`、第二个 `false`（复用对象偏小）。

用例的地址都带时间戳（`MakeUniqueShmName`），同一进程内先后运行不会互相干扰；清理依赖析构顺序（先声明的后析构：第二个对象先 `unlink`，第一个对象的后析构收尾）。

## 九、已知未覆盖

- `ShmBase::Send` 的「通道写满」分支（写返 0 → 等 1 ms 重试 / 对端断连则丢弃）：冒烟每轮 10000 次往返也难填满 1 MiB 通道，正确性依赖 `ShmBuffer::Write` 的返 0 语义（由 `ShmBufferTest` 钉住）。
- 「两端连接数不一致」：客户端那次 `ftruncate` 会截短服务端对象（第四节末），无用例、无防护。
- Windows 侧「复用对象尺寸不足」没有独立用例：该平台的判据是 `MapViewOfFile` 自己失败，故不存在可撤的代码分支。
- `Init()` 失败后析构仍删除对象（第六节缺口）。
- `ShmBase::DoSend`（override 了 `IoBase` 纯虚）在 Shm 后端**没有调用点**；保留与否属于 `IoBase` 接口层面的裁定，未在本批内。

**通道在映射里的摆放、连接对象的类型不变式与两端的并发契约见 `docs/shm-channel-and-connect-model.md`**（本文只覆盖对象本身）。
