# tools —— 规范检查脚本

本目录存放 `rules/cpp-style.md` 的**机器可检查部分**。这些条款属于结构语义，
`clang-format` 无法表达（详见 `docs/cpp-style-clang-format.md`），故用脚本补位。

脚本一律**只读**：只报告，不修改任何文件。改与不改由人判断。

---

## 1. 脚本清单

| 脚本 | 覆盖条款 |
| ---- | ---- |
| `s4scan.py` | §4 类内结构：首标签 / 重复标签 / 访问段优先级 / 段内 6 组序 / 标签顶格 / 段间空行 |
| `initcheck.py` | §4 非静态数据成员声明顺序与初始化列表一致 |

---

## 2. 用法

```bash
# 扫描 git 跟踪的全部 .h/.cpp/.hpp/.cc（推荐；无参数）
python tools/s4scan.py
python tools/initcheck.py

# 只扫指定路径：目录会递归展开，也可直接给文件
python tools/s4scan.py src/Network
python tools/s4scan.py src/Network/Shm/SingleShm.h src/Core/Logger.cpp

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

## 6. 依赖

只用 Python 标准库（`re` / `os` / `subprocess` / `sys`），无第三方依赖。
须在 git 仓库内运行（无参数时的文件列表取自 `git rev-parse --show-toplevel`
与 `git ls-files`，两者都要求有 git 环境；不在仓库内时须显式给路径）。
目录展开另需一次 `git ls-files --others --ignored --exclude-standard --directory`
来取忽略目录表；取不到 git 时退化为「不跳过」而非静默跳过。
