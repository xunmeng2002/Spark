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
# 扫描 git 跟踪的全部 .h/.cpp/.hpp/.cc
python tools/s4scan.py
python tools/initcheck.py

# 只扫指定路径
python tools/s4scan.py src/Network
```

退出码恒为 `0`，发现项以文本列出；末行给出扫描规模（类数 / 初始化列表数）。
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
否则「0 命中」可能只是判据失灵，而不是真的合规。复现方式：

```bash
mkdir -p /tmp/s4corpus
python tools/s4scan.py    /tmp/s4corpus/pos.h     # 应逐条报警
python tools/s4scan.py    /tmp/s4corpus/neg.h     # 应 0 命中
python tools/initcheck.py /tmp/s4corpus/initpos.h # 应报 INIT_ORDER/A
python tools/initcheck.py /tmp/s4corpus/initneg.h # 应 0 命中
```

---

## 5. 作为门禁

如需纳入 CI，在 Build 阶段之前加一步，对新增/改动的 C++ 文件运行
`python tools/s4scan.py <files>` 与 `python tools/initcheck.py <files>`，
非空输出即失败。建议**只对增量文件**启用——存量 `struct` 的
`NO_FIRST_LABEL` 是既定例外，全仓门禁会一直红。

---

## 6. 依赖

只用 Python 标准库（`re` / `os` / `subprocess` / `sys`），无第三方依赖。
须在 git 仓库内运行（默认文件列表取自 `git ls-files`）。
