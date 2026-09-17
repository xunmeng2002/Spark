# selfcheck —— 扫描器的判别力语料

这十个文件是 `tools/s4scan.py` 与 `tools/initcheck.py` 的**自检输入**：
`pos*` 是故意违规的正例，`neg*` 是完全合规的反例。

## 为什么需要它

扫描器报「0 命中」时，有两种可能：代码真的合规，或者判据失灵了（正则没匹配上、
行号算错、文件根本没被读进去）。只看「0 命中」分不出这两者。所以每次改动扫描器后，
必须先用这些语料自证「已知正例必报、已知反例不报」，再相信它在真实代码上的结论。

本目录曾长期只存在于 `/tmp`（随系统清理消失），等于这份自检只能被当时那个人跑一次。
现已入库，命令可直接照抄。

## 目录

| 文件 | 期望结果 |
| ---- | ---- |
| `pos.h` | `s4scan.py` 报 10 条：ACCESS_ORDER ×3、DUP_LABEL ×2、ORDER ×2、NO_BLANK ×2、NOT_FLUSH ×1 |
| `neg.h` | `s4scan.py` 报 0 条（含 §4 单例例外的正确写法） |
| `nofirst.h` | `s4scan.py` 报 2 条 NO_FIRST_LABEL（`class` 与 `struct` 各一） |
| `wrappedsig.h` | `s4scan.py` 报 1 条 ORDER（仅 `WrappedBad`） |
| `initpos.h` | `initcheck.py` 报 1 条 INIT_ORDER/A |
| `initpos.cpp` | `initcheck.py` 报 1 条 INIT_ORDER/B（须与 `initpos.h` 同时传入） |
| `initneg.h` | `initcheck.py` 报 0 条 |
| `initwrapped.cpp` | `initcheck.py` 报 1 条 INIT_ORDER/B |
| `initbraced.cpp` | `initcheck.py` 报 1 条 INIT_ORDER/B |
| `initsentinel.cpp` | `initcheck.py` 报 1 条 INIT_ORDER/B（仅 Gamma；Alpha 与 Delta 合规） |

各文件头部注释里也写了对应的期望值。

## 形状覆盖（2026-09-17 补）

`initcheck.py` 通道 B 要在 `:` 之后扫到函数体的 `{` 为止。**「哪个 `{` 才是函数体」这件事
没有唯一答案，判据必须对写法免疫**——而 `initpos.cpp` 只覆盖了「`:` 独占一行」一种形状，
于是 clang-format 收敛把仓里的形状改掉之后，判据静默失效：通道 B 由 217 掉到 211，
而本目录的自检**全绿**（`initpos.cpp` 那条恰好是新旧判据都能认的老形状）。

补的三份语料把类外定义的四种形状铺开：

| 文件 | 类外定义形状 |
| ---- | ---- |
| `initpos.cpp` | `Foo::Foo()` 换行 + `: a_(1), b_(2)` + `{` 独占一行 |
| `initwrapped.cpp` | `Foo::Foo() : a_(1), b_(2)` + `{` 独占次行（clang-format 对放不进一行的列表的输出） |
| `initbraced.cpp` | 同上，但列表里含成员 braced-init `a_{1}` |
| `initsentinel.cpp` | `Foo::Foo() : a_(1), b_(2) {}` 整条压一行、函数体空 |

三份新语料对三个历史错判据的判别力互不重复（实测结果如下表），缺一份就漏掉一类：

| 判据 | `initwrapped` | `initbraced` | `initsentinel` |
| ---- | ---- | ---- | ---- |
| 要求 `:` 与 `{` 同行 | **漏报 0** | **漏报 0** | 1 ✓ |
| 第一个 `{` 即函数体 | 1 ✓ | **漏报 0** | 1 ✓ |
| 配对 `}` 同行算 braced-init | 1 ✓ | 1 ✓ | **多报 2** |

`initsentinel.cpp` 测的是**多报**：末尾刻意留的 `Delta` 是越界扫描的收尾点，它的函数体
必须跨行（否则扫描一路扫到文件尾判空，症状变成漏报，与真实仓的现象不是一回事）
且非空（否则会被 clang-format 塌成 `{}`）。

三份新语料的函数体都写成非空，**因而对 clang-format 稳定**：空体的跨行写法会被
clang-format 塌成一行，语料就换了形状、判别力悄然变形——这正是下面「防好心地被修好」
一节警告的事。

## 跨行声明的续行：判据与被检写法耦合（2026-09-17 补）

`s4scan.py` 要按「组」判段内顺序，而组是由**类作用域里的每条声明**分类出来的。
一条**跨行的函数声明**，其续行不含 `(` 却以 `;` 结尾，会被 `classify()` 落到最后
一条分支当**数据成员**——于是凭空配出「普通成员函数出现在数据成员之后」的 ORDER。

它出不出来，取决于同一个函数上方的 `template <...>` 是独占一行、还是与函数压成
一行：函数名落到自己那行时 `classify()` 才返回 `func`，压行时整行归 `other`。
**只改写法、不改语义，扫描结果就会凭空多出一条。** 2026-09-17 把全仓 `template`
改回独占一行（`.clang-format` 的 `BreakTemplateDeclarations: Yes`）时，它就在
`TimeUtility` 上现形了——而该类的 public 段里一个数据成员都没有。

修法是给类作用域的条目加**括号配平**判据：完整声明的括号必然配平（以 `;` 收尾、
或 Allman 在次行开体），跨行声明的首行必然左括号多于右括号，故首行保留、续行全丢，
直到括号回平。这条判据与写法无关。

| 类 | 形状 | 期望 |
| -- | ---- | ---- |
| `WrappedOk` | 跨行声明 + 其下是独占一行的 template 函数，类内无数据成员 | **0 条**（旧判据误报 1 条） |
| `WrappedBad` | 跨行声明 + 真数据成员 + 其后又有函数 | **1 条** |

两者缺一不可：没有 `WrappedOk` 就测不出误报；没有 `WrappedBad` 就测不出
「把续行一律丢掉」这种过度修复——那会把跨行声明也吞掉，反而漏掉真正的组序违规。

## 复现

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

> **注意**：`initcheck.py` 收到路径参数时会打印「通道 B 的结论不作数」的告警。
> 自检时这是预期的——本语料只为验证通道 B **能命中**，不要求它在子集里给出完整结论。
>
> **注意**：必须**点名文件**（如上），不要给目录 `tools/selfcheck` —— 目录展开遵守
> `EXCLUDE`，本目录在 `EXCLUDE` 内，会以「全部落在 EXCLUDE 内」`exit 2` 响亮失败。
> 显式点名的文件不受 `EXCLUDE` 约束，这是刻意留的口子。

## 与全仓扫描的关系

本目录已在 `s4scan.py` 的 `EXCLUDE` 中，**不会**进入无参数的全仓扫描——
否则这些故意写坏的文件会混进真实结果里。显式给路径时照常扫描。

改动 `EXCLUDE` 或文件位置后，请确认无参数运行的规模行仍是「176 个文件、24 条」。

## 防「好心地被修好」

本目录带一个 `.clang-format`（`DisableFormat: true`），把语料挡在 clang-format 之外。
**这不是洁癖，是必要保护**：`pos.h` 里两处 `NO_BLANK` 与一处 `NOT_FLUSH` 是**故意**造的，
而 clang-format 的职责恰好就是修它们——实测它会补上空行、并去掉 namespace 内的缩进。
一旦语料被格式化，10 条期望发现里会**静默少掉 3 条**，而自检看起来仍在正常工作。
维护语料时请注意：这些「不合规」是输入，不是待修的缺陷。
