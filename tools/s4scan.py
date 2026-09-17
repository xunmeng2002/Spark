# -*- coding: utf-8 -*-
"""`rules/cpp-style.md` §4「类内顺序」结构检查（只读，不修改任何文件）。

覆盖条款：
  * 显式写访问标签，不依赖 `class` 默认私有            -> NO_FIRST_LABEL
  * 每段尽量只出现一次（不重复开标签）                  -> DUP_LABEL
  * 访问优先 `public` -> `protected` -> `private`       -> ACCESS_ORDER
  * 段内 6 组序（别名/常量/特殊成员/其他操作符/普通函数/数据成员） -> ORDER
  * 标签顶格（与所属类声明同列）、段间空行              -> NOT_FLUSH / NO_BLANK
  * 数据成员置于段末                                    -> 由 ORDER（第 6 组最靠后）覆盖

判据基于花括号配对，故与缩进风格无关（Tab / 空格 / 混用均可）。注释与字符串字面量
先被遮蔽（见 `mask`），避免其中的 `class` / `public:` 造成误判。

用法：
  python tools/s4scan.py                 # 扫描 git 跟踪的全部 .h/.cpp/.hpp/.cc
  python tools/s4scan.py src/Network     # 只扫指定路径（目录会递归展开）
  python tools/s4scan.py a.h b.cpp       # 也可直接给文件

**可从任意子目录调用**：无参数时的文件列表取自 `git rev-parse --show-toplevel`，
不随当前目录变化。扫描到 0 个文件时**响亮失败**并返回非 0 退出码——扫描器对空输入
报「0 命中」是假绿灯，必须与「真的干净」区分开。

设计取舍：本脚本一律「只报告不修复」。理由见 docs/cpp-style-clang-format.md——
§4 的这几条属于结构语义，clang-format 无法表达，改与不改须由人判断。
"""
import os
import re
import subprocess
import sys

EXCLUDE = ("Serialization/json/", "tools/selfcheck/")
CPP_EXTS = ('.h', '.hpp', '.hxx', '.cpp', '.cc', '.cxx')
OPENER = re.compile(r'(?<!enum )\b(class|struct|union)\s+([A-Za-z_]\w*)')
LABEL = re.compile(r'^\s*(public|protected|private)\s*:\s*$')
RANK = {'public': 0, 'protected': 1, 'private': 2}
GROUP_NAMES = {1: 'type-alias/嵌套类型', 2: '常量', 3: '特殊成员函数', 4: '其他操作符重载',
               5: '普通成员函数', 6: '数据成员'}
USAGE = """用法: python tools/s4scan.py [路径...]

  无参数   扫描 git 跟踪的全部 .h/.cpp/.hpp/.cc（可从任意子目录调用）
  路径     文件或目录；目录会递归展开成其中的 C++ 文件
  -h       显示本帮助

目录展开时会跳过 git 忽略的子目录（`out/`、`bin/`、`lib/` 等构建产物），跳过数在末行
报出；显式点名的文件不受此约束，也不受 EXCLUDE 约束。

退出码: 0 = 扫描完成（有候选发现也算 0，条目需人工判读）；2 = 用法/路径错误或扫到 0 个文件。"""

_ROOT = []
_SKIPPED = []
_IGNORED = []


def fail(msg, code=2):
    """用法级错误：必须响亮失败，不能退回「0 命中」。"""
    print('错误: ' + msg, file=sys.stderr)
    raise SystemExit(code)


def repo_root():
    """仓库根目录（缓存）；不在 git 仓库内返回 None。"""
    if not _ROOT:
        r = subprocess.run(['git', 'rev-parse', '--show-toplevel'],
                           capture_output=True, text=True)
        _ROOT.append(r.stdout.strip() if r.returncode == 0 else None)
    return _ROOT[0]


def ignored_dirs():
    """git 忽略的目录（绝对路径，已 normcase）。

    递归展开用户给的目录时跳过它们。不跳过会怎样：`out/build/*/vcpkg_installed`
    下每个 preset 都有一份第三方头文件副本，全仓累计两万余个 `.h`，扫描结果
    会被它们整个淹没（实测 `..` 报 4573 条，而真实结果只有 24 条）。

    判据取自 `.gitignore`（`out/*`、`bin/`、`lib/` 等）**而不是硬编码目录名**——
    改预设目录时不会失效。取不到 git 时返回空集，退化为「不跳过」而非静默跳过。
    """
    if not _IGNORED:
        found = set()
        root = repo_root()
        if root:
            r = subprocess.run(['git', 'ls-files', '--others', '--ignored',
                                '--exclude-standard', '--directory'],
                               cwd=root, capture_output=True, text=True)
            for line in r.stdout.split('\n'):
                line = line.strip()
                if line.endswith('/'):
                    found.add(os.path.normcase(os.path.normpath(os.path.join(root, line))))
        _IGNORED.append(found)
    return _IGNORED[0]


def _is_ignored_dir(path):
    return os.path.normcase(os.path.normpath(os.path.abspath(path))) in ignored_dirs()


def parse_targets(args):
    """把命令行参数解析成待扫描的**绝对路径**列表。

    用户给的路径相对**当前目录**解析（这是调用方的自然预期）；
    默认列表相对**仓库根**解析，故与当前目录无关。

    任一分支得到空列表时抛 SystemExit：扫描 0 个文件却报「0 命中」，
    与「真的干净」无从区分，属假绿灯。
    """
    if any(a in ('-h', '--help') for a in args):
        print(USAGE)
        raise SystemExit(0)
    for a in args:
        if a.startswith('-'):
            fail('未知选项 %r（本脚本没有可配置项，用 -h 看用法）' % a)
    if args:
        out, pinned = [], set()
        for a in args:
            if os.path.isdir(a):
                dir_total = 0
                for dirpath, subdirs, names in os.walk(a):
                    subdirs[:] = [d for d in subdirs if not _is_ignored_dir(os.path.join(dirpath, d))]
                    got = [os.path.join(dirpath, n) for n in sorted(names) if _is_cpp(n)]
                    dir_total += len(got)
                    out += got
                if not dir_total:
                    fail('目录下没有可扫的 .h/.cpp/.hpp/.cc 文件（被 git 忽略的子目录会被跳过）: %s' % a)
            elif os.path.isfile(a):
                # 显式点名的文件一律照扫，不受 EXCLUDE / 忽略规则约束——
                # 否则 `tools/selfcheck/` 这种「本就属于 EXCLUDE、但要单独拿来跑」的语料没法用。
                out.append(a)
                pinned.add(os.path.abspath(a))
            else:
                fail('路径不存在或不可读: %s' % a)
        expanded = [os.path.abspath(p) for p in out]
        targets = [p for p in expanded
                   if p in pinned or not any(e in p.replace(os.sep, '/') for e in EXCLUDE)]
        _SKIPPED.append(len(expanded) - len(targets))
        if not targets:
            if expanded:
                fail('展开出 %d 个 C++ 文件，但全部落在 EXCLUDE %s 内: %s'
                     % (len(expanded), list(EXCLUDE), ' '.join(args)))
            fail('指定的路径下没有 .h/.cpp/.hpp/.cc 文件: %s' % ' '.join(args))
    else:
        root = repo_root()
        if root is None:
            fail('当前不在 git 仓库内，无法自动取文件列表；请显式给出要扫描的路径')
        r = subprocess.run(['git', 'ls-files', '*.h', '*.cpp', '*.hpp', '*.cc'],
                           cwd=root, capture_output=True, text=True)
        targets = [os.path.join(root, f) for f in r.stdout.split()]
        targets = [p for p in targets
                   if not any(e in p.replace(os.sep, '/') for e in EXCLUDE)]
        if not targets:
            fail('git 未跟踪任何 C++ 文件——请确认在仓库内、且工作区不为空')
    known = {t for t in targets if os.path.isfile(t)}
    if len(known) != len(targets):
        missing = sorted(set(targets) - known)[:5]
        fail('下列路径不是文件（git 索引与工作区不一致？）: %s' % ', '.join(missing))
    return sorted(targets)


def _is_cpp(name):
    return os.path.splitext(name)[1].lower() in CPP_EXTS


def rel(path):
    """输出用路径：在仓库内则相对仓库根显示（更短），否则原样。"""
    root = repo_root()
    if root and path.startswith(root + os.sep):
        return os.path.relpath(path, root).replace(os.sep, '/')
    return path


def mask(text):
    """遮蔽注释与字符串/字符字面量，保留偏移与换行（行号因此不漂移）。"""
    out = []
    i, n = 0, len(text)
    while i < n:
        c = text[i]
        if c == '/' and i + 1 < n and text[i + 1] == '/':
            while i < n and text[i] != '\n':
                out.append(' ')
                i += 1
            continue
        if c == '/' and i + 1 < n and text[i + 1] == '*':
            out.append('  ')
            i += 2
            while i < n and not (text[i] == '*' and i + 1 < n and text[i + 1] == '/'):
                out.append('\n' if text[i] == '\n' else ' ')
                i += 1
            out.append('  ')
            i += 2
            continue
        if c in '"\'':
            q = c
            out.append(' ')
            i += 1
            while i < n and text[i] != q:
                if text[i] == '\\' and i + 1 < n:
                    out.append('  ')
                    i += 2
                    continue
                out.append('\n' if text[i] == '\n' else ' ')
                i += 1
            if i < n:
                out.append(' ')
                i += 1
            continue
        out.append(c)
        i += 1
    return ''.join(out)


def matching_brace(masked, open_idx):
    depth = 0
    for j in range(open_idx, len(masked)):
        if masked[j] == '{':
            depth += 1
        elif masked[j] == '}':
            depth -= 1
            if depth == 0:
                return j
    return -1


def real_name(first, between):
    """`class CORE_EXPORTS Logger` -> 导出宏在前、真类名在后；宏是全大写，故取下一个词。"""
    tail = between.split()
    if first.isupper() and tail:
        return tail[0]
    return first


def find_classes(masked):
    """产出 (kw, name, open_idx, close_idx)；不跳过嵌套类，由调用方循环各自处理。"""
    i = 0
    while True:
        m = OPENER.search(masked, i)
        if not m:
            return
        i = m.end()
        j = m.end()
        while j < len(masked) and masked[j] not in '{;()':
            j += 1
        if j >= len(masked) or masked[j] != '{':
            continue
        close = matching_brace(masked, j)
        if close < 0:
            continue
        yield m.group(1), real_name(m.group(2), masked[m.end():j]), j, close


def indent_of(line):
    return len(line) - len(line.lstrip())


def classify(line):
    d = line.strip()
    if not d:
        return None
    if LABEL.match(d):
        return ('label', d.rstrip(':').strip())
    if re.match(r'^(class|struct|union|enum)\b', d):
        return ('type', d)
    if re.match(r'^(using|typedef)\b', d):
        return ('alias', d)
    if re.match(r'^static_assert\b', d):
        return ('static_assert', d)
    if re.match(r'^template\s*<', d) or d.startswith('friend ') and '(' not in d:
        return ('other', d)
    if '(' in d:
        is_special = bool(re.search(r'(?<![A-Za-z0-9_])~\w+\s*\(', d))
        # 构造函数：前导标识符（可带 explicit/constexpr/inline）后紧跟 (
        if re.match(r'^(?:constexpr\s+|inline\s+|explicit\s+)*[A-Z]\w*\s*\(', d):
            is_special = True
        if re.match(r'^(?:constexpr\s+|inline\s+|explicit\s+)*[A-Z]\w*\s*&\s*operator\s*=(?!=)', d):
            is_special = True
        is_assign = bool(re.search(r'\boperator\s*=(?!=)', d))
        is_other_op = bool(re.search(r'\boperator\b', d)) and not is_assign
        return ('func', d, is_special, is_assign, is_other_op)
    if d.endswith(';') or d.endswith('}') or d.endswith('};') or re.match(r'^[A-Za-z_].*[A-Za-z0-9_>\*&\]]\s*$', d):
        return ('data', d)
    return ('other', d)


def class_entries(body_raw, body_masked, open_line):
    """返回类体顶层的 (绝对行号, 原文, 遮蔽文本)，已剔除多行声明续行与纯符号行。"""
    entries = []
    depth = 0
    for idx, (raw_line, m_line) in enumerate(zip(body_raw.split('\n'), body_masked.split('\n'))):
        if depth == 0 and m_line.strip():
            entries.append((open_line + idx, raw_line, m_line))
        depth += m_line.count('{') - m_line.count('}')
    entries = [(ln, r, m) for (ln, r, m) in entries
               if not r.strip().startswith('//') and not r.strip().startswith('*')]
    # 丢掉多行声明的续行（典型是多行 static_assert 的第二行：无 `(` 却以 `;` 结尾，
    # 会被误判成数据成员）。
    #
    # 判据是**括号配平**：类作用域的一条完整声明，要么以 `;` 收尾、要么（Allman）
    # 在下一行以 `{` 开体，两种写法的括号都是配平的；而**跨行声明的首行**必然左括号
    # 多于右括号。故首行保留、其后的续行全部丢掉，直到括号回平。
    #
    # 早期版本只对 `static_assert` 设 pending，于是多行**函数声明**的续行漏了过来：
    #   static void CalculateRealMinuteBarTime(const char* exchangeId, ..., int& realBarTime,
    #                                          int& realUpdateTs);
    # 首行含 `(` 被认成函数并保留，续行 `int& realUpdateTs);` 不含 `(`、又以 `;` 结尾，
    # 落进 classify() 的最后一条分支被当成**数据成员**，凭空配出「普通成员函数出现在
    # 数据成员之后」的 ORDER 误报（实测命中 TimeUtility，而该类的 public 段里一个数据
    # 成员都没有）。
    #
    # 该误报长期被另一种写法掩盖：`template <...> void GetDuration(...)` 压成一行时，
    # classify() 走 `^template\s*<` 分支返回 ('other', ...)，配不出那个组合。2026-09-17
    # 把 template 改回独占一行（.clang-format 的 BreakTemplateDeclarations: Yes）之后，
    # 函数名落到自己那行、重新被认成函数，误报才浮出来。判据修好后与写法无关。
    kept, pending, paren_depth = [], False, 0
    for ln, r, m in entries:
        if pending:
            if ';' in m:
                pending = False
            continue
        if paren_depth > 0:
            paren_depth += m.count('(') - m.count(')')
            continue
        k0 = classify(m)
        if k0 and k0[0] == 'static_assert' and ';' not in m:
            pending = True
            continue
        kept.append((ln, r, m))
        paren_depth += m.count('(') - m.count(')')
        if paren_depth < 0:
            paren_depth = 0
    # 丢掉续行：构造函数初始化列表（`:` 起头）、续参、花括号行
    return [(ln, r, m) for (ln, r, m) in kept
            if not m.strip().startswith((':', ',', '{}', '};')) and m.strip() not in ('{', '}')]


def check_structure(path):
    """§4 结构条款：首标签 / 重复标签 / 访问优先级 / 段内组序。"""
    raw = open(path, encoding='utf-8', errors='replace').read()
    masked = mask(raw)
    findings = []
    for kw, name, o, c in find_classes(masked):
        open_line = masked[:o + 1].count('\n') + 1
        entries = class_entries(raw[o + 1:c], masked[o + 1:c], open_line)
        if not entries:
            continue
        order = []
        for ln, r, m in entries:
            k = classify(m)
            if k is None:
                continue
            order.append((ln, k, r.strip()))
        # --- 规则：显式首标签
        if classify(entries[0][2])[0] != 'label':
            findings.append(('NO_FIRST_LABEL', kw, name, open_line, entries[0][0],
                             f'首个成员前无访问标签（依赖 {kw} 默认访问）: {entries[0][1].strip()!r}'))
        # --- 切段：每段 = 一个访问标签 + 其后的成员
        sections = []                 # (label, 标签行号, [成员条目])
        cur_label, cur_ln, cur_items = None, None, []
        for ln, k, txt in order:
            if k[0] == 'label':
                if cur_label is not None:
                    sections.append((cur_label, cur_ln, cur_items))
                cur_label, cur_ln, cur_items = k[1], ln, []
            elif cur_label is not None:
                cur_items.append((ln, k, txt))
        if cur_label is not None:
            sections.append((cur_label, cur_ln, cur_items))

        # N7 例外：私有特殊成员置顶、该区只放特殊成员、随后立即 public:
        def is_n7_head(i):
            if sections[i][0] != 'private' or not sections[i][2]:
                return False
            for _ln, k, _t in sections[i][2]:
                if k[0] != 'func' or not (k[2] or k[3]):
                    return False          # 混进了非特殊成员，不适用例外
            return i + 1 < len(sections) and sections[i + 1][0] == 'public'

        n7 = {i for i in range(len(sections)) if is_n7_head(i)}

        # --- 规则：同一标签重复开启
        seen = {}
        for i, (lab, ln, _items) in enumerate(sections):
            if i in n7:
                continue                  # 例外区不计入重复
            seen.setdefault(lab, []).append(ln)
        for lab, ls in seen.items():
            if len(ls) > 1:
                findings.append(('DUP_LABEL', kw, name, open_line, ls[0],
                                 f'{lab}: 重复出现于 {ls}'))
        # --- 规则：public -> protected -> private
        last = RANK['private'] if classify(entries[0][2])[0] != 'label' and kw == 'class' else (
            RANK['public'] if classify(entries[0][2])[0] != 'label' else -1)
        for i, (lab, ln, _items) in enumerate(sections):
            if i in n7:
                continue                  # 例外区不参与优先级单调性判断
            if RANK[lab] < last:
                findings.append(('ACCESS_ORDER', kw, name, open_line, ln,
                                 f'{lab}: 出现在更低优先级段之后'))
            last = max(last, RANK[lab])
        # --- 规则：段内组序
        # 组序游标必须按「段」重置：N7 例外下 private 会合法地出现两次，
        # 若沿用上一段的游标，第二段的首个成员会被拿去和第一段的成员比大小。
        cur = None
        last_group = {}
        for ln, k, txt in order:
            if k[0] == 'label':
                cur = k[1]
                last_group.clear()
                continue
            if cur is None:
                cur = 'private' if kw == 'class' else 'public'
            if k[0] in ('alias', 'type'):
                g = 1
            elif k[0] == 'static_assert':
                continue
            elif k[0] == 'func':
                if k[2] or k[3]:
                    g = 3
                elif k[4]:
                    g = 4
                else:
                    g = 5
            elif k[0] == 'data':
                # 仅 `static constexpr` / `static const` 属第 2 组「常量」；
                # 非 static 的 `const char* x_;` 是普通数据成员（第 6 组）
                g = 2 if re.match(r'^static\s+(?:constexpr|const)\b', txt) else 6
            else:
                continue
            prev = last_group.get(cur)
            if prev is not None and g < prev[0]:
                findings.append(('ORDER', kw, name, open_line, ln,
                                 f'{cur} 段内：{GROUP_NAMES[g]} 出现在 {GROUP_NAMES[prev[0]]} 之后'
                                 f'（后者在第 {prev[1]} 行）：{txt!r}'))
            last_group[cur] = (g, ln)
    return findings


def check_label_layout(path):
    """§4「标签顶格，段间空行」。顶格的判据不是「列 0」，而是与所属类声明行同列：
    类本身可以嵌在缩进块里（例如测试文件的匿名 namespace），此时标签仍应与
    `class` 关键字同列。"""
    raw = open(path, encoding='utf-8', errors='replace').read()
    masked = mask(raw)
    lines = raw.split('\n')
    out = []
    for kw, name, o, c in find_classes(masked):
        # 注意：o / c 是 masked **字符串**里的字符偏移，必须用字符串切片数行；
        # 写成 `mlines[:o].count('\n')` 会变成「按行列表切片」，静默得到错误行号。
        cls_line = masked[:o].count('\n')          # 类声明行（0 基）
        base = indent_of(lines[cls_line])
        body_start = masked[:o + 1].count('\n')    # 类体首行（0 基）
        body_end = masked[:c].count('\n')          # 类闭合 `}` 行（0 基）
        for i in range(body_start, body_end):
            ln = lines[i]
            if not LABEL.match(ln):
                continue
            ind = indent_of(ln)
            if ind != base:
                out.append(('NOT_FLUSH', kw, name, cls_line + 1, i + 1,
                            f'缩进 {ind} 列，类声明在 {base} 列'))
            elif i and lines[i - 1].strip().endswith((';', '}')):
                out.append(('NO_BLANK', kw, name, cls_line + 1, i + 1,
                            f'上一行为 {lines[i - 1].strip()!r}'))
    return out


def check(path):
    return check_structure(path) + check_label_layout(path)


def main():
    files = parse_targets(sys.argv[1:])
    total_classes, total, failed = 0, 0, 0
    for f in files:
        try:
            found = check(f)
            n_cls = len(list(find_classes(mask(open(f, encoding='utf-8', errors='replace').read()))))
        except Exception as exc:  # noqa: BLE001 —— 单个文件解析失败不应中断整仓扫描
            print(f'!! {rel(f)}: {type(exc).__name__}: {exc}')
            failed += 1
            continue
        total_classes += n_cls
        if found:
            print(f'===== {rel(f)}  ({n_cls} 个类)')
            for kind, kw, name, oline, ln, detail in sorted(found, key=lambda x: x[4]):
                total += 1
                print(f'  [{kind}] {kw} {name} (第 {oline} 行起) 第 {ln} 行: {detail}')
    print('---- 扫描 %d 个文件、%d 个类/结构体，候选发现: %d%s%s'
          % (len(files), total_classes, total,
             '' if not failed else '（另有 %d 个文件解析失败）' % failed,
             '' if not _SKIPPED or not _SKIPPED[0] else '（跳过 EXCLUDE 内 %d 个文件）' % _SKIPPED[0]))


if __name__ == '__main__':
    main()
