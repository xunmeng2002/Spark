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
  python tools/s4scan.py src/Network     # 只扫指定路径

设计取舍：本脚本一律「只报告不修复」。理由见 docs/cpp-style-clang-format.md——
§4 的这几条属于结构语义，clang-format 无法表达，改与不改须由人判断。
"""
import re
import subprocess
import sys

EXCLUDE = ("Serialization/json/",)
OPENER = re.compile(r'(?<!enum )\b(class|struct|union)\s+([A-Za-z_]\w*)')
LABEL = re.compile(r'^\s*(public|protected|private)\s*:\s*$')
RANK = {'public': 0, 'protected': 1, 'private': 2}
GROUP_NAMES = {1: 'type-alias/嵌套类型', 2: '常量', 3: '特殊成员函数', 4: '其他操作符重载',
               5: '普通成员函数', 6: '数据成员'}


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
    # 会被误判成数据成员）。以「上一条声明尚未收尾」判断。
    kept, pending = [], False
    for ln, r, m in entries:
        if pending:
            if ';' in m:
                pending = False
            continue
        k0 = classify(m)
        if k0 and k0[0] == 'static_assert' and ';' not in m:
            pending = True
            continue
        kept.append((ln, r, m))
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


def repo_files():
    listed = subprocess.run(['git', 'ls-files', '*.h', '*.cpp', '*.hpp', '*.cc'],
                            capture_output=True, text=True).stdout.split()
    return [f for f in listed if not any(e in f for e in EXCLUDE)]


def main():
    files = sys.argv[1:] or repo_files()
    total_classes, total = 0, 0
    for f in files:
        try:
            found = check(f)
            n_cls = len(list(find_classes(mask(open(f, encoding='utf-8', errors='replace').read()))))
        except Exception as exc:  # noqa: BLE001 —— 单个文件解析失败不应中断整仓扫描
            print(f'!! {f}: {type(exc).__name__}: {exc}')
            continue
        total_classes += n_cls
        if found:
            print(f'===== {f}  ({n_cls} 个类)')
            for kind, kw, name, oline, ln, detail in sorted(found, key=lambda x: x[4]):
                total += 1
                print(f'  [{kind}] {kw} {name} (第 {oline} 行起) 第 {ln} 行: {detail}')
    print(f'---- 扫描类数: {total_classes}，候选发现: {total}')


if __name__ == '__main__':
    main()
