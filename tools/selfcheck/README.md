# selfcheck —— 扫描器的判别力语料

这六个文件是 `tools/s4scan.py` 与 `tools/initcheck.py` 的**自检输入**：
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
| `initpos.h` | `initcheck.py` 报 1 条 INIT_ORDER/A |
| `initpos.cpp` | `initcheck.py` 报 1 条 INIT_ORDER/B（须与 `initpos.h` 同时传入） |
| `initneg.h` | `initcheck.py` 报 0 条 |

各文件头部注释里也写了对应的期望值。

## 复现

```bash
python tools/s4scan.py    tools/selfcheck/pos.h                                  # 10 条
python tools/s4scan.py    tools/selfcheck/neg.h                                  # 0 条
python tools/s4scan.py    tools/selfcheck/nofirst.h                              # 2 条
python tools/initcheck.py tools/selfcheck/initpos.h tools/selfcheck/initpos.cpp  # 2 条
python tools/initcheck.py tools/selfcheck/initneg.h                              # 0 条
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
