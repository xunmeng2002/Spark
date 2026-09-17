# -*- coding: utf-8 -*-
"""`rules/cpp-style.md` §4「非静态数据成员声明顺序决定初始化顺序，初始化列表需一致」检查。

与 `tools/s4scan.py` 分工：s4scan 管类内**结构**，本脚本管**初始化列表与声明顺序是否同序**。

两条独立通道，互不覆盖：
  A. 类体内定义的构造函数
  B. 类外定义的 `Foo::Foo(...) : a_(1), b_(2)` —— 需先全仓建「类名 -> 成员声明顺序」表

通道 B 依赖类名唯一的假设：同名类若成员表不同则判为歧义，整体跳过（宁漏勿误）。

只读。用法：
  python tools/initcheck.py
  python tools/initcheck.py src/Network
"""
import os
import re
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import s4scan as S

IDENT = re.compile(r'[A-Za-z_]\w*')
CTOR_DEF = re.compile(r'\b(\w+)::(\w+)\s*\(')


def member_name(decl):
    """`std::atomic<FreeNode*> freeList_ = nullptr;` -> 'freeList_'"""
    d = decl.strip().rstrip(';').strip()
    d = d.split('=')[0].strip()                       # 去掉初始化器
    d = re.sub(r'\[[^\]]*\]\s*$', '', d).strip()      # 去掉数组维度
    d = d.rstrip('{').strip()
    names = IDENT.findall(d)
    return names[-1] if names else None


def declared_members(body_masked, open_line):
    """按声明顺序返回 [(成员名, 绝对行号)]。"""
    out = []
    depth = 0
    for idx, m_line in enumerate(body_masked.split('\n')):
        if depth == 0 and m_line.strip():
            k = S.classify(m_line)
            if k and k[0] == 'data':
                nm = member_name(m_line)
                if nm:
                    out.append((nm, open_line + idx))
        depth += m_line.count('{') - m_line.count('}')
    return out


def init_at(msk, i):
    """若第 i 行是某构造函数头，返回 (初始化列表成员名, 下一行号)；否则 None。"""
    m = msk[i]
    # 单行：`Node() : data(nullptr), next(nullptr) {}`
    # 成员初始化也可用花括号（`buffer_{ 0 }`），故以**最后一个** `{` 作为函数体起点
    same = re.search(r'\)\s*:\s*(.*)\{', m)
    if same:
        return re.findall(r'([A-Za-z_]\w*)\s*[({]', same.group(1)), i + 1
    # Allman：`Ctor()` 换行后 `:a_(...), b_(...)`，直到以 `{` 打头的函数体行
    if m.rstrip().endswith(')') and i + 1 < len(msk):
        j = i + 1
        while j < len(msk) and not msk[j].strip():
            j += 1
        if j < len(msk) and msk[j].strip().startswith(':'):
            buf, k = [], j
            while k < len(msk):
                s = msk[k].strip()
                if s.startswith('{'):   # 函数体起点；不能用 `'{' in s`，否则撞上 `buffer_{ 0 }`
                    break
                buf.append(s)
                k += 1
            return re.findall(r'([A-Za-z_]\w*)\s*[({]', ' '.join(buf)), k
    return None


def in_class_init_lists(body_masked):
    """通道 A：类体内的构造函数初始化列表。"""
    msk = body_masked.split('\n')
    out, i = [], 0
    while i < len(msk):
        got = init_at(msk, i)
        if got:
            names, nxt = got
            out.append((i, names))
            i = nxt
            continue
        i += 1
    return out


def out_of_class_init_lists(raw, masked):
    """通道 B：`Foo::Foo(...)` 类外定义。返回 [(行号, 类名, 初始化列表成员名)]。"""
    out = []
    for m in CTOR_DEF.finditer(masked):
        cls, ctor = m.group(1), m.group(2)
        if cls != ctor:
            continue                      # 不是构造函数
        # 参数可能折行，先把括号配平，再找初始化列表
        depth, j = 0, m.end() - 1
        while j < len(masked):
            if masked[j] == '(':
                depth += 1
            elif masked[j] == ')':
                depth -= 1
                if depth == 0:
                    break
            j += 1
        line_no = masked[:m.start()].count('\n')
        tail_msk = masked[j + 1:].split('\n')
        same = re.match(r'\s*:\s*(.*)\{', tail_msk[0]) if tail_msk else None
        if same:
            out.append((line_no + 1, cls, re.findall(r'([A-Za-z_]\w*)\s*[({]', same.group(1))))
            continue
        if tail_msk and tail_msk[0].strip() == '':
            k = 1
            while k < len(tail_msk) and not tail_msk[k].strip():
                k += 1
            if k < len(tail_msk) and tail_msk[k].strip().startswith(':'):
                buf = []
                while k < len(tail_msk):
                    s = tail_msk[k].strip()
                    if s.startswith('{'):
                        break
                    buf.append(s)
                    k += 1
                out.append((line_no + 1, cls, re.findall(r'([A-Za-z_]\w*)\s*[({]', ' '.join(buf))))
    return out


def judge(seq, index_of, declared):
    """seq 是初始化列表里属于本类的成员名序列；返回第一条乱序的说明，否则 None。"""
    idx = [(nm, index_of[nm]) for nm in seq if nm in index_of]
    for (n1, i1), (n2, i2) in zip(idx, idx[1:]):
        if i2 < i1:
            return (f'初始化列表顺序 {n1} → {n2}，但声明顺序是先 {n2}（第 {declared[i2][1]} 行）'
                    f'后 {n1}（第 {declared[i1][1]} 行）；实际初始化顺序为 {n2} 先于 {n1}')
    return None


def main():
    files = sys.argv[1:] or S.repo_files()
    parsed = {}
    for f in files:
        try:
            raw = open(f, encoding='utf-8', errors='replace').read()
        except OSError as exc:
            print(f'!! {f}: {type(exc).__name__}: {exc}')
            continue
        parsed[f] = (raw, S.mask(raw))

    # 类名 -> 成员声明顺序表；同名类出现多个且成员表不同则视为歧义，跳过通道 B
    table, ambiguous = {}, set()
    for f, (raw, masked) in parsed.items():
        for kw, name, o, c in S.find_classes(masked):
            declared = declared_members(masked[o + 1:c], masked[:o + 1].count('\n') + 1)
            if not declared:
                continue
            if name in table and table[name] != declared:
                ambiguous.add(name)
            table.setdefault(name, declared)

    total_a = total_b = 0
    hits = []
    for f, (raw, masked) in parsed.items():
        # 通道 A
        for kw, name, o, c in S.find_classes(masked):
            declared = declared_members(masked[o + 1:c], masked[:o + 1].count('\n') + 1)
            if not declared:
                continue
            index_of = {nm: n for n, (nm, _) in enumerate(declared)}
            open_line = masked[:o + 1].count('\n') + 1
            for rel, names in in_class_init_lists(masked[o + 1:c]):
                total_a += 1
                why = judge(names, index_of, declared)
                if why:
                    hits.append(('A', f, open_line + rel, kw, name, why))
        # 通道 B
        for line_no, cls, names in out_of_class_init_lists(raw, masked):
            if cls in ambiguous or cls not in table:
                continue
            declared = table[cls]
            index_of = {nm: n for n, (nm, _) in enumerate(declared)}
            total_b += 1
            why = judge(names, index_of, declared)
            if why:
                hits.append(('B', f, line_no, 'class', cls, why))

    for ch, f, ln, kw, name, why in hits:
        print(f'  [INIT_ORDER/{ch}] {f}:{ln}  ({kw} {name})')
        print(f'        {why}')
    print(f'---- 通道 A（类内定义）检查 {total_a} 个初始化列表；通道 B（类外定义）检查 {total_b} 个；'
          f'乱序 {len(hits)} 处')
    if ambiguous:
        print(f'---- 因类名歧义跳过通道 B 的类: {sorted(ambiguous)}')


if __name__ == '__main__':
    main()
