# tools —— 规范检查与冒烟脚本

本目录存放 `rules/cpp-style.md` 的**机器可检查部分**。这些条款属于结构语义，
`clang-format` 无法表达（详见 `docs/cpp-style-clang-format.md`），故用脚本补位。

规范检查脚本一律**只读**：只报告，不修改任何文件。改与不改由人判断。

另有一个**端到端冒烟**脚本 `step_e2e.py`（第 6 节）：它不查规范，而是真的**起进程**收发
STEP 帧并断言字段——只读的是源码，进程与日志当然会动。两者同放本目录，是因为它们都是
「验证」而非「构建」的一环。

---

## 1. 脚本清单

| 脚本 | 覆盖条款 |
| ---- | ---- |
| `s4scan.py` | §4 类内结构：首标签 / 重复标签 / 访问段优先级 / 段内 6 组序 / 标签顶格 / 段间空行 |
| `initcheck.py` | §4 非静态数据成员声明顺序与初始化列表一致 |
| `step_e2e.py` | 非规范检查：STEP 帧端到端（发出 → 解析 → 断言字段），见第 6 节 |

---

## 2. 用法

```bash
# 扫描 git 跟踪的全部 .h/.cpp/.hpp/.cc（推荐；无参数）
python tools/s4scan.py
python tools/initcheck.py

# 只扫指定路径：目录会递归展开，也可直接给文件
python tools/s4scan.py src/Network
python tools/s4scan.py src/Network/Shm/ShmBase.h src/Core/Logger.cpp

# 帮助
python tools/s4scan.py -h
```

两个脚本**可从任意子目录调用**：无参数时的文件列表取自 `git rev-parse --show-toplevel`，
不随当前目录变化（早期版本用 `git ls-files` 而不固定 `cwd`，在 `tools/` 下会得到空列表并
打印「0 命中」，与「真的干净」无从区分）。命令行给的路径则相对**当前目录**解析。

**目录展开的两条规则**：

| 规则 | 说明 |
| ---- | ---- |
| 跳过 git 忽略的子目录 | `out/`、`bin/`、`lib/` 等构建产物与第三方安装目录（判据取自 `.gitignore`，非硬编码目录名）。跳过数在末行报出 |
| 显式点名的文件不受 EXCLUDE 约束 | 用于单独跑 `tools/selfcheck/` 里的语料；目录展开则照常遵守 EXCLUDE |

不跳过会怎样：`out/build/*/vcpkg_installed` 下每个 preset 都有一份第三方头文件副本，
实测 `python tools/s4scan.py ..` 会报 **416 个文件 / 4573 条**，而真实结果是
**176 个文件 / 24 条**——真实发现被第三方头文件整个淹没。

退出码：

| 码 | 含义 |
| ---- | ---- |
| `0` | 扫描完成。**有候选发现也是 0**——条目需人工判读，脚本不替人做结论 |
| `2` | 用法/路径错误，**或扫到 0 个文件**（空输入报「0 命中」是假绿灯，故一律响亮失败） |

发现项以文本列出；末行给出扫描规模（文件数 / 类数 / 初始化列表数）——先看这一行，
规模为 0 或明显偏小时，结论不可信。

> **注意**：`initcheck.py` 的通道 B（类外定义的构造函数）需要**全仓**的
> 「类名 → 成员声明顺序」表。给了路径参数时脚本会打印告警，此时通道 B 的结论不作数，
> 要校验通道 B 必须不带参数运行。

输出中的条目是**候选**，需人工判定——扫描器按语法形状取数，不理解业务语义。

---

## 3. 判据说明

- **基于花括号配对，与缩进风格无关**：Tab、空格、混用均可正确解析。
- **注释与字符串字面量先被遮蔽**：其中的 `class`、`public:` 不会造成误判。
- **「顶格」的判据不是「列 0」**，而是与所属类声明行同列：类本身可以嵌在缩进块里
  （例如测试文件的匿名 namespace），此时标签仍应与 `class` 关键字同列。
- **单例例外（§4 末条）已建模**：私有特殊成员置顶、该区只放特殊成员、随后立即
  `public:` 时，不判为重复标签，也不参与访问优先级单调性判断。
- **`initcheck.py` 通道 B 依赖类名唯一**：同名类若成员表不同则判为歧义并整体跳过，
  宁漏勿误。

---

## 4. 判别力自检

两个脚本都做过「注入的已知正例必须报警、已知反例必须不报警」的验证——
否则「0 命中」可能只是判据失灵，而不是真的合规。语料在 `tools/selfcheck/`，
**每次改动扫描器后请先跑一遍**：

```bash
python tools/s4scan.py    tools/selfcheck/pos.h                                  # 10 条
python tools/s4scan.py    tools/selfcheck/neg.h                                  # 0 条
python tools/s4scan.py    tools/selfcheck/nofirst.h                              # 2 条
python tools/s4scan.py    tools/selfcheck/wrappedsig.h                           # 1 条
python tools/initcheck.py tools/selfcheck/initpos.h tools/selfcheck/initpos.cpp  # 2 条
python tools/initcheck.py tools/selfcheck/initneg.h                              # 0 条
python tools/initcheck.py tools/selfcheck/initwrapped.cpp                        # 1 条
python tools/initcheck.py tools/selfcheck/initbraced.cpp                         # 1 条
python tools/initcheck.py tools/selfcheck/initsentinel.cpp                       # 1 条
```

后三条是 2026-09-17 补的：`initpos.cpp` 只覆盖「`:` 独占一行」一种类外定义形状，
而 clang-format 收敛后仓里的形状变了，判据随之静默漏检（通道 B 由 217 掉到 211），
自检却全绿——**语料的形状没跟上被检对象的写法**，等于没有判别力。
详见 `tools/selfcheck/README.md` 的「形状覆盖」一节。

`wrappedsig.h` 同理，只是方向相反：`s4scan.py` 把**跨行函数声明的续行**当成数据成员，
于是配出「函数出现在数据成员之后」的 ORDER。这条误报出不出现，取决于函数上方的
`template <...>` 是独占一行还是压成一行——**只改写法就能凭空多一条发现**。两个类
（`WrappedOk` 必须 0 条、`WrappedBad` 必须 1 条）缺一不可，详见 `tools/selfcheck/README.md`。

每个文件的期望值与设计意图见 `tools/selfcheck/README.md`。该目录已在 `EXCLUDE` 中，
不会污染无参数的全仓结果。

---

## 5. 作为门禁（尚未接入）

`.workflow/pipeline.yml` 目前只有一个 Build 阶段（`build@gcc`，`gccVersion: 11.1.0`，
执行 `cmake -G 'Unix Makefiles' ../ && make -j2`），且 `trigger: manual`——
**不会随提交自动跑**。若要接门禁，加在 Build 之前一步：

```bash
python tools/s4scan.py <本次改动的 C++ 文件>
python tools/initcheck.py <本次改动的 C++ 文件>
```

两点取舍：

- **只对增量文件启用**。存量 `struct` 的 `NO_FIRST_LABEL` 是既定例外
  （用户裁定 `struct` 不必显式写 `public:`），全仓门禁会一直红；且 `s4scan.py`
  的 `struct` 例外目前只在**报告侧**建模，未在判据侧排除。
- **`initcheck.py` 适合先接**：它要求全仓建表，与「只查增量」冲突，但可全仓跑且当前
  实测 0 乱序，接进来即绿，不会误伤。

---

## 6. 端到端冒烟（`step_e2e.py`）

```bash
python tools/step_e2e.py                 # Debug 配置，跑 40 秒
python tools/step_e2e.py --seconds 60
python tools/step_e2e.py --config Release
```

**它补的是哪个洞**：标准冒烟（`TestServer` + `TestClient`，`TestProtocol` 默认 `Tcp`）走
`ServerIoSubscriberImpl::OnRecv` 的**原样回显**——两端都不解析，`Package::FromStepStream`
一次都没被调用。真正「发出 → 成帧 → 解析 → 断言字段」的路径只在
`StepClient` / `StepServer`（`ProtocolTypeType::Step`，实现分别在 `TestStepClient.*` /
`TestStepServer.*`）里，而它此前只能靠手改
`TestUtility.cpp` 里 `TestProtocol` 的初值来选中（改完还得按字节还原，极易出错）。
现在两个 `main` 都接受命令行第一个参数，本脚本即用 `TestServer.exe Step` /
`TestClient.exe Step` 选中它。

**默认 40 秒、上限 80 秒**，两个数都是被客户端那一侧逼出来的：

- 默认值：客户端每 10000 条才记一行，机器慢一倍时 20 秒只够凑出 2 帧，第 2 帧还可能连着
  缓冲区被 `terminate` 丢掉，于是把「机器慢」误报成协议回归。40 秒给足的余量是 2 倍以上。
- 上限：服务端连上后 90 秒自停（`TestStepServer` 的 `sleep_for(90s)`），跑过这个窗口后段
  就是死时间——帧仍存在、序号仍等间隔，断言看不出「末段没在跑」，于是可能假绿。超过 80
  秒直接按用法错误拒掉。

**断言依据**（`StepClient::SendReqInsertOrder` 的不变量）：`Price == 100 + Volume`、
`Volume == ClientOrderId == index`、三个字符串字段与三个枚举字段恒为定值。服务端每 1000 条
记一行、客户端每 10000 条记一行，故样本序号**必须严格等间隔递增**——这一条同时证明流没有
错位、没有重连。另断言两端日志里不出现 `Garbage Stream Detected` / `CheckSum not Match` /
`FieldId not Match` 等读路径失败字句，也不出现任何 `ERROR` 行。

**每侧至少 2 帧才算数**：客户端每 10000 条才记一行，跑太短就只剩 1 帧，此时「间隔恒定」
无从校验。样本不足按**失败**计（而不是照常给绿灯），否则「条数与字段全部吻合」这句结论
名不副实。客户端跑满 1,000,000 条时会把收尾那一帧连记两遍，脚本会折叠相邻重复序号后再算
间隔，免得把收尾行为误诊成流错位。

**前置**：先构建出 `bin/<配置>/TestServer.exe` 与 `TestClient.exe`（本脚本不会替你构建；
Windows 下用 MSVC，注意先 `vcvars64`，否则 `cl.exe` 找不到 `stdint.h`）。

**就绪判定靠日志、不靠固定等待**：`listen()` 成功没有对应的日志行，可轮询的唯一证据是
`CreateIo ServerType:Server`，故脚本先轮询该行（最多 15 秒）再留 1 秒等 bind+listen 完成。
不做端口探测，是因为探测连接会在服务端留下一个真实会话，可能反过来污染「日志里不得出现
`ERROR`」这条断言。

**注意**：起进程时 stdout 一律 `DEVNULL`。给 `PIPE` 而不读，日志线程写满管道后会阻塞，
表现成「跑十几条就不动了」的假卡死（2026-09-18 已用同型实验刻意复现过，当时的「13 条后
卡死」就是这么来的，不是回归）。子进程的回收放在 `finally` 里，中途抛错也会先杀客户端、
再杀服务端，不会留下占着 `127.0.0.1:20001` 的僵尸进程。

退出码：

| 码 | 含义 |
| ---- | ---- |
| `0` | 断言全过 |
| `1` | 断言未过（有 `ERROR`、字段不符、序号间隔异常、样本不足 2 帧、或压根没读到帧） |
| `2` | 用法/环境错误（参数写错、缺可执行文件、服务端 15 秒未就绪、本次没落下日志文件——**没日志就不给绿灯**） |

---

## 7. 依赖

只用 Python 标准库（`os` / `re` / `subprocess` / `sys` / `time` / `typing`），无第三方依赖。

以下两句针对两个**扫描脚本**（`step_e2e.py` 不吃这套：它的仓库根是从脚本自身位置推出来的，
不查 git）：须在 git 仓库内运行（无参数时的文件列表取自 `git rev-parse --show-toplevel`
与 `git ls-files`，两者都要求有 git 环境；不在仓库内时须显式给路径）。
目录展开另需一次 `git ls-files --others --ignored --exclude-standard --directory`
来取忽略目录表；取不到 git 时退化为「不跳过」而非静默跳过。
