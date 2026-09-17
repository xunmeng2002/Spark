# clang-format 与 `rules/cpp-style.md` §4 的对应关系

仓库根目录新增 `.clang-format`，把 §4「类内顺序」中**能被格式化工具表达**的那一半固化下来。
本文说明覆盖边界、验证方式，以及无法用 clang-format 表达时的替代手段。

---

## 1. 结论速览

| §4 条款 | clang-format 能否表达 | 手段 |
| :--- | :--- | :--- |
| 标签顶格 | ✅ | `AccessModifierOffset: -4`（相对成员再左移 4 列，与 `class` 同列） |
| 成员缩进 4 空格 | ✅ | `IndentWidth: 4` + `IndentAccessModifiers: false` |
| 段间空行 | ✅ | `EmptyLineBeforeAccessModifier: LogicalBlock` |
| Allman 括号 | ✅ | `BreakBeforeBraces: Allman` |
| 访问段优先 `public` → `protected` → `private` | ❌ | 结构检查脚本（见 §3.1） |
| 段内 6 组序（类型别名 → 常量 → 特殊成员 → 其他操作符 → 普通函数 → 数据成员） | ❌ | 结构检查脚本（见 §3.1） |
| 数据成员置于段末 | ❌ | 结构检查脚本（见 §3.1） |
| 每段尽量只出现一次（不重复开标签） | ❌ | 结构检查脚本（见 §3.1） |
| 显式写访问标签，不依赖 `class` 默认私有 | ❌ | 结构检查脚本（见 §3.1） |
| 非静态数据成员声明顺序 → 初始化列表一致 | ❌ | 检查脚本 + 编译器警告 `-Wreorder`（双保险，见 §3.1 / §3.2） |
| 段内子组之间的空行（如常量与特殊成员之间） | ❌ | 手工；`EmptyLineBeforeAccessModifier` 只管访问标签，不管段内分组 |

---

## 2. 可表达部分的实测验证

**正例**：把 §4 规范里的 `Example` 示例去掉缩进后交给 clang-format，
输出与规范原文逐字一致——标签顶格、成员缩进 4 空格、`public`/`protected`/`private`
三段之间有且仅有一个空行，且 `{` 之后的首个标签上方**不**补空行（`LogicalBlock` 的行为）。

**反例（证明边界）**：构造一个同时含四类违规的类——

- 数据成员出现在构造函数之前；
- 数据成员未置于段末；
- `public:` 重复出现两次；
- `private:` 出现在 `protected:` 之前；
- 初始化列表顺序与声明顺序相反。

`clang-format --style=file` 的输出与输入**零差异**。即这五类问题 clang-format 一律不管，
不能指望它兜底。

---

## 3. 不可表达部分的替代手段

### 3.1 结构性问题用检查脚本

`tools/` 下的两个脚本覆盖上表**全部** ❌ 结构项：

| 脚本 | 覆盖条款 |
| :--- | :--- |
| `tools/s4scan.py` | 标签顶格 / 段间空行 / 访问段优先级 / 段内 6 组序 / 数据成员置段末 / 每段只出现一次 / 显式首标签 |
| `tools/initcheck.py` | 非静态数据成员声明顺序与初始化列表一致 |

两个脚本都是「只读扫描 + 报告」，不修改文件；且都以「注入的已知正例必须报警、已知反例必须不报警」
做过判别力验证，避免出现「0 命中」其实是判据失灵的情况。语料入库在 `tools/selfcheck/`，
可照抄命令复现——只报「0 命中」而不先自证判别力的扫描器，其绿灯不具意义。
用法与判据细节见 `tools/README.md`。

### 3.2 初始化列表顺序另有编译器警告兜底

脚本之外，同一条款还有编译器侧的第二道防线（二者互补：脚本查得出、警告查得准）：

- GCC / Clang：`-Wreorder`
- MSVC：`C5038`（`/W4` 起）

**注意**：`-Wreorder` **不在** GCC/Clang 的默认告警集内（含在 `-Wall` 中，但本仓库未开 `-Wall`），
必须在顶层 `CMakeLists.txt` 显式打开。本仓库已按「只加这一条、不带其它警告」的最小改动落地：

```cmake
if(NOT MSVC)
    add_compile_options(-Wreorder)
endif()
```

MSVC 侧未启用：`C5038` 属 `/W4` 级，本项目未开 `/W4`，单独用 `/w15038` 亦需要
在 Windows 工具链上实测确认，故留待有可用 MSVC 环境的时机再评估。

---

## 4. 勿在全仓直接 `clang-format -i`

本文件声明的是**目标状态**，不是「当前全仓已经满足」。下列项会让全仓 `-i` 产生
§4 范围之外的大量改动，需要先决策：

| 项 | 原因 |
| :--- | :--- |
| Tab ↔ 空格 | `.editorconfig` 声明 `indent_style = space`，但仓库现有 176 个源文件中 101 个纯 Tab、44 个纯空格、18 个混用。本配置按 `.editorconfig` 取 `UseTab: Never`，跑 `-i` 会把那 101 个文件全部改写缩进 |
| 初始化列表的 `:` 后空格 | clang-format 固定输出 `: member(...)`（冒号后有空格）；仓库现写法是 `:member(...)`。此项 clang-format 无开关可关，属必然改写 |
| 初始化列表续行缩进 | clang-format 由 `ConstructorInitializerIndentWidth: 4` 控制（4 空格），仓库部分文件用 Tab |
| 超长函数签名的换行 | clang-format 按代价模型对齐填充；仓库手写风格与之不一定相同 |
| 连续空行 | `MaxEmptyLinesToKeep: 1` 会折叠连续两行以上的空行 |

---

## 5. 用法

只检查、不改文件（适合做门禁）：

```bash
clang-format --style=file --dry-run --Werror <files...>
```

格式化单个文件（改前先看 diff）：

```bash
clang-format --style=file path/to/File.h > /tmp/new.h
diff -u path/to/File.h /tmp/new.h
```
