# -*- coding: utf-8 -*-
"""`rules/cpp-style.md` §4「非静态数据成员声明顺序决定初始化顺序，初始化列表需一致」检查。

与 `tools/s4scan.py` 分工：s4scan 管类内**结构**，本脚本管**初始化列表与声明顺序是否同序**。

两条独立通道，互不覆盖：
  A. 类体内定义的构造函数
  B. 类外定义的 `Foo::Foo(...) : a_(1), b_(2)` —— 需先全仓建「类名 -> 成员声明顺序」表

通道 B 依赖类名唯一的假设：同名类若成员表不同则判为歧义，整体跳过（宁漏勿误）。

只读。用法：
  python tools/initcheck.py              # 全仓（推荐；通道 B 需要全仓的类表）
  python tools/initcheck.py src/Network  # 只扫指定路径（路径相对当前目录）

**给定的路径只是子集时，通道 B 的结论不作数**：它要先全仓建「类名 → 成员声明顺序」
表，子集里看不到的类会被静默跳过（`cls not in table`）。脚本会在这种情况下打印告警。

退出码: 0 = 扫描完成（有乱序也算 0）；2 = 用法/路径错误或扫到 0 个文件。
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
    """通道 B：`Foo::Foo(...)` 类外定义。返回 [(行号, 类名, 初始化列表成员名)]。

    初始化列表的书写形状有三种，都要认（前两种都要求 `:` 与 `{` 同行，第三种不用）：
      1. 签名、初始化列表、`{` 全在同一行：`Foo::Foo() : a_(1) {`
      2. 初始化列表整体在签名下一行：`Foo::Foo()\n    : a_(1)\n{`
      3. 签名与初始化列表同行、`{` 在下一行（Allman）：
         `Timer::Timer() : timeInterval_(60000), eventCount_(600)\n{`

    第 3 种是本脚本原先漏掉的形状，**不是**理论情形：clang-format 会把较短的初始化
    列表并到签名行，于是类外定义由「多行」变成「同行 + 次行 `{`」，旧判据要求 `:` 与
    `{` 同行、结果整条初始化列表**静默脱离检查**——实测通道 B 从 217 掉到 211。
    这类「检查器的口径与被检对象的写法同步失效」是本仓反复吃亏的同一失败型，故这里
    不再按「哪种形状」分支，而是统一从 `:` 扫到函数体的 `{` 为止。
    """
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
        members = init_list_members(masked, j + 1)
        if members is not None:
            out.append((line_no + 1, cls, members))
    return out


def init_list_members(masked, tail_start):
    """从闭括号之后扫到函数体的 `{`，取出初始化列表成员名；不是初始化列表则返回 None。

    难点是「哪个 `{` 才是函数体」。两侧都栽过：

    - 一律把第一个 `{` 当函数体：成员的 **braced-init** 也是花括号（`buff_{0}`、
      `sockets_{INVALID_SOCKET, INVALID_SOCKET}`、`lastConnectAttemptTime_{}`），于是
      初始化列表在最外层那个 `{` 处提前收尾，整条判空或截断——实测由此丢掉 3 条
      （`PackageReader` / `SocketNotify` 归零、`TcpBase` 少 3 个成员）。
    - 按「配对 `}` 是否在同一行」判 braced-init：**空函数体** `{}` 的配对 `}` 也在同一行，
      被误认成成员初始化，于是越过函数体继续往后扫，把下一个函数的限定名也收进成员表
      ——实测 `Logger::Logger() : ... logData_(nullptr) {}` 之后连 `Logger::GetInstance`
      一起收了进来（clang-format 会把短构造函数连空体收成一行，这不是理论情形）。
      该噪声目前被 `judge()` 的 `nm in index_of` 挡掉，故未变成误报，但判据本身是错的。

    正确的区分依据来自语法：本仓 `BreakBeforeBraces: Allman`，函数体的 `{` 要么独占一行，
    要么紧跟在构造函数形参列表的 `)`（或 braced-init 的 `}`）之后；而 braced-init 的 `{`
    一定**紧贴一个标识符或模板/数组闭合符**。故判据为：括号外的 `{`，左侧跳空白后若紧贴
    标识符、`>`、`]` 则是 braced-init，否则是函数体起点。
    括号深度仍需跟踪，只认**括号外**的 `{`；遇到 `;` 说明已越过定义，一并放弃。
    宁可漏报不误报：形状认不出时返回 None，绝不猜。
    """
    i = tail_start
    n = len(masked)
    while i < n and masked[i] in ' \t\r\n':
        i += 1
    if i >= n or masked[i] != ':' or (i + 1 < n and masked[i + 1] == ':'):
        return None                       # 闭括号后不是 `:`，说明没有初始化列表
    i += 1
    depth = 0
    start = i
    while i < n:
        c = masked[i]
        if c == '{' and depth == 0:
            k = i - 1
            while k >= 0 and masked[k] in ' \t\r\n':
                k -= 1
            prev = masked[k] if k >= 0 else ''
            if prev.isascii() and (prev.isalnum() or prev in '_>]'):
                depth += 1                # 紧贴标识符/模板闭合 = 成员的 braced-init
            else:
                break                     # 前接 `)` 或处在行首 = 函数体起点
        elif c in '([{':
            depth += 1
        elif c in ')]}':
            if depth == 0:
                return None               # 未配平，形状不认识
            depth -= 1
        elif c == ';' and depth == 0:
            return None
        i += 1
    if i >= n:
        return None
    return re.findall(r'([A-Za-z_]\w*)\s*[({]', masked[start:i])


def judge(seq, index_of, declared):
    """seq 是初始化列表里属于本类的成员名序列；返回第一条乱序的说明，否则 None。"""
    idx = [(nm, index_of[nm]) for nm in seq if nm in index_of]
    for (n1, i1), (n2, i2) in zip(idx, idx[1:]):
        if i2 < i1:
            return (f'初始化列表顺序 {n1} → {n2}，但声明顺序是先 {n2}（第 {declared[i2][1]} 行）'
                    f'后 {n1}（第 {declared[i1][1]} 行）；实际初始化顺序为 {n2} 先于 {n1}')
    return None


def main():
    args = sys.argv[1:]
    files = S.parse_targets(args)
    if args:
        all_files = S.parse_targets([])
        print('注意: 只给了 %d 个文件（全仓 %d 个），通道 B 只在给定文件里建类表——'
              '跨文件的类外构造函数定义会被漏检，通道 B 的结论不作数；'
              '要校验通道 B 请不带参数全仓运行。' % (len(files), len(all_files)))
    parsed = {}
    for f in files:
        try:
            raw = open(f, encoding='utf-8', errors='replace').read()
        except OSError as exc:
            print(f'!! {S.rel(f)}: {type(exc).__name__}: {exc}')
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
            for rel_line, names in in_class_init_lists(masked[o + 1:c]):
                total_a += 1
                why = judge(names, index_of, declared)
                if why:
                    hits.append(('A', f, open_line + rel_line, kw, name, why))
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
        print(f'  [INIT_ORDER/{ch}] {S.rel(f)}:{ln}  ({kw} {name})')
        print(f'        {why}')
    skipped = S._SKIPPED[0] if S._SKIPPED else 0
    print(f'---- 扫描 {len(files)} 个文件；通道 A（类内定义）检查 {total_a} 个初始化列表；'
          f'通道 B（类外定义）检查 {total_b} 个；乱序 {len(hits)} 处'
          + (f'（跳过 EXCLUDE 内 {skipped} 个文件）' if skipped else ''))
    if ambiguous:
        print(f'---- 因类名歧义跳过通道 B 的类: {sorted(ambiguous)}')


if __name__ == '__main__':
    main()
