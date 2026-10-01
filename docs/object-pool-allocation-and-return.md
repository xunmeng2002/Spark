# 对象池的取还契约与失败回退

本文记录 `include/Spark/TemplateLib/ObjectPool/ObjectPool.h` 的**取还契约**与**每一条失败路径把状态退回到哪里**。
池的线程本地链、共享链、槽位步长等做法无法从类型或命名读出，而本仓代码不写注释（Harness §4：命名即文档），故一并写在这里。

## 一、两段式取还

每个 `T` 一个单例池（`GetInstance()` 的函数内静态），池内两级空闲链：

| 层 | 归属 | 同步手段 | 容量 |
| ---- | ---- | ---- | ---- |
| 线程本地链（`threadLocalCache_`） | 每线程每 `T` 一份（`static thread_local`） | 无 | 至多 `blockUnitNum_` 个节点 |
| 共享链（`sharedFreeList_`） | 全池共用 | `mutex_` | 不限（按块增长） |

`Allocate` 只碰本地链：空则先 `RefillThreadLocalFreeList()` 从共享链整批搬一块过来，再从本地链头弹一个。
`Deallocate` 也只碰本地链：把节点前插回去，本地链超过一块时由 `ReturnExcessThreadLocalNodesToSharedList()` 整批交回共享链。

**为什么本地链不能省**：若各线程直接在共享链上 `pop`/`push`，「读 `Next` 到 CAS」这段窗口里链头可能被别的线程弹出、构造、再推回（ABA），
CAS 就会把一个**正在使用中**的对象发布成新的链头。把 pop/push 收进线程私有区，共享链只在 `mutex_` 内整批搬运，ABA 便无从发生。

槽位步长不是 `sizeof(T)`，而是**按空闲节点所需的对齐向上取整**（`ObjectPoolDetail::SlotByteCountFor`）：
空闲节点与 `T` 复用同一片内存，`Next` 压在其首 8 字节上，`sizeof(T)` 不是 `alignof(FreeNode)` 的整数倍时 `&objects[i]` 会落在对齐要求之外。
两条编译期门槛随之而来：`sizeof(FreeNode) <= sizeof(T)`、`alignof(T) <= __STDCPP_DEFAULT_NEW_ALIGNMENT__`。

取还契约：

- `Allocate` **不返回 nullptr**——要么交出已构造对象，要么抛出；调用方无须判空。
- `Deallocate(nullptr)` 安全，直接返回。
- 只有池自己发出的指针才可归还。Debug 构建下每条取出的对象都登记在册（`RegisterAllocatedItem`），
  归还时先摘牌再析构（`AssertAndUnregisterOwnedItem`）：摘牌与析构分成两步，是为了先放开登记表的锁，
  使「析构里归还本池的另一个对象」不会自锁。重复归还或交进外来指针会就地报出类型与指针，而不是留到别处以越界写现形。
- **归还必须发生在取得该对象的同一个模块内**。`GetInstance()` 与 Debug 期的登记表都是**模板内的函数局部 static**，
  MSVC 不跨模块唯一化：`Networkd.dll` 与 `UnitTests.exe` 各持一份池、也各持一份登记表。实测把 DLL 内分配的
  `LinearBuffer<65536>` 交给测试侧 mock 归还，Debug 会报 `Deallocate got an item that is not currently held` 并就地断言；
  分配与归还都放回 DLL 则通过。两个模块**都未定义 `NDEBUG`**，故不是编译开关不一致，而是登记表本身有两份。
  生产侧各模块只归还自己分配的，这条约束因此一直成立——但它此前没有写下来，任何一次「跨模块转交池对象所有权」的改动都会踩到。
- `SetBlockUnitNum` 只在**启动期**调用，其后不得再改：它不在 `mutex_` 内，而热路径读 `blockUnitNum_` 是非原子的，
  热路径上改它就是数据竞争。约定即「启动期可调、此后禁改」，当前调用点只有测试与启动期两处，与此相符。
- 线程退出时，本地链上剩余的空闲节点由 `ThreadLocalCache` 的析构整批交回共享链。不回收的话这些槽位
  再不参与流通，池只能靠新申请块补足，反复创建/销毁线程时按块累积（默认一块 64 个槽位，
  `LinearBuffer<BufferSize>` 一块约 4 MiB）。

这条回收依赖一条次序保证：**同一线程内，线程存储期对象的析构先于静态存储期对象的析构**，故析构发生时池单例必定还活着。
2026-09-30 在 MSVC 与 GCC 上各测了四种组合（本地对象先/后于池初始化、本地对象有无用户声明的构造函数），
本地对象与 DLL 内的池都成立（DLL 侧另测一遍）。
代价是一条宿主纪律：**线程不得比进程活得久**（进程退出前 join），模块也不得在线程存活时卸载——
这两种情形下线程本地对象的析构会晚于池单例，与池自身的实现无关。

## 二、失败路径各自退回到哪里

池里的失败有四种，前两种在改链之前发生、池自然无变化，后两种本批补了回退：

| 失败点 | 抛出者 | 池的状态 |
| ---- | ---- | ---- |
| 槽位步长与块容量的乘积溢出 | `std::length_error` | 未改链，无变化 |
| 块内存或块记录申请失败 | `std::bad_alloc` | 未改链，无变化；块记录申请失败时那块内存已被释放 |
| 对象的**构造函数**抛出 | 由 `T` 决定 | **本批新增**：把刚弹出的节点原样推回本地链 |
| `shared_ptr` 的**控制块**申请失败 | `std::bad_alloc` | **本批新增**：归还该对象（析构在此正常执行） |

第三条是重点。改前的写法是「弹节点 → 落构造 → 返回」，构造抛出时节点已经离开空闲链、而对象又没建成，
既无人持有这块内存、它也回不到链上——**一次 `bad_alloc` 就永久漏掉一个槽位**。
现在构造包在 `try` 里，`catch (...)` 先把节点推回本地链再原样重抛，语义是「弹出没成功就当作没弹过」。

这条路径让**每一个 `Allocate` 调用点**都变安全：以前「构造会抛」只影响调用点自己，现在连池的账也不会错。
`ShmClient::EstablishConfirmedConnection` 的撤回正建立在这个前提上（见 `docs/shm-channel-and-connect-model.md` 第四节）。

第四条同理：控制块申请失败时对象已经构造完毕却无人接管，`catch` 里归还即可。此路径当前**无生产调用点**（`AllocateShared` 仅测试在用）。

归还节点这一段（前插 + 计数 + 超限整批交回）抽成了 `ReturnNodeToThreadLocalFreeList`，
`Deallocate` 与构造抛出的回退共用同一份，不再各写一遍。

## 三、为什么回退走「推回节点」而不是 `Deallocate`

构造抛出的对象**从未完整构造**，调 `Deallocate` 会去执行它的析构函数、并在 Debug 下摘一次从未登记的牌，两件事都是错的。
故回退只做「把节点前插回链」这一半——`Deallocate` 里析构之后的那一段。

由此还带出一个附带结论：回退**不可能触发整批交回**。本地链长度在每个操作结束时恒有 `count <= blockUnitNum_`
（`Refill` 搬来的至多一块、`Deallocate` 超限即整批交回、`Allocate` 弹出只会减），
而回退是把计数还原成弹出前的值，故还原后仍不超限，`ReturnExcessThreadLocalNodesToSharedList` 在那条路上不会被调用。

交回共享链的搬运只有一处：`SpliceThreadLocalNodesToSharedListLocked(nodeCount)`（调用方须持 `mutex_`）；
两个调用点各自算出要交回的个数——「本地链积压超过一块」交 `min(count, blockUnitNum_)` 个，线程退出交剩下的全部。
后者能一次交完，靠的正是上面这条「本地链长度恒 ≤ 一块」的不变量。该函数开头有一条 Debug 期断言
（`1 <= nodeCount <= count`）；它拦的是「算错个数」这类改动，Release 下零成本。

## 四、相关测试与其输入输出

| 用例 | 输入 | 输出 |
| ---- | ---- | ---- |
| `ObjectPoolTest.Allocate_ReturnsTheSlotToTheFreeListWhenConstructionThrows` | 块容量 4，先取满 4 个槽位再全部归还；随后连做 3 次**必然抛出**的构造；最后再取 4 个 | 取回的 4 个地址与最初的 4 个**逐一相同**：丢一个节点就会去申请新块、地址集合随即不等 |
| `ObjectPoolTest.AllocateAndDeallocate_Recycles` | 取一个、归还、再取一个 | 复用同一槽位 |
| `ObjectPoolTest.RecycleAfterExcessReturnToSharedList` | 块容量 8、40 个对象反复取还 | 40 个地址互不重复，且全部来自原槽位 |
| `ObjectPoolTest.ConcurrentBatchAllocateDeallocate` | 8 线程 × 40 轮 × 每轮 12 个 | 共享集合里无重复登记（同一地址不得同时属于两个线程） |
| `ObjectPoolTest.ThreadExit_ReturnsThreadLocalNodesToSharedList` | 块容量 4；子线程取满 4 个、全部归还后**退出**；主线程再取 4 个 | 主线程取回的 4 个地址与子线程那 4 个**逐一相同**：不回收就会去申请新块，地址集合随即不等 |

新用例对旧实现的判据已实测：把回退那两行去掉重跑，4 个槽位里只有 1 个能取回，另外 3 个来自新申请的块，
断言在「地址集合不等」处失败（`ObjectPoolTest.cpp` 第 115 行）。

线程退出那条同样实测过判别力：把 `ThreadLocalCache` 的析构改成 `= default`（即回到回收之前）重跑，
主线程取回的 4 个地址与子线程那 4 个**无一相同**——它们来自新申请的块。

## 五、已知未覆盖与未决

- **`AllocateShared` 的控制块失败路径无用例**：`std::bad_alloc` 无法注入，只能靠代码审读。
- 构造抛出这条路径的**计数与登记表一致性**由上述用例间接覆盖（Debug 下登记表若有偏差会走 `ReportOwnershipViolation` 直接断言失败），无独立用例。
