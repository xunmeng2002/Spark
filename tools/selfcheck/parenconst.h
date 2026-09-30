// 判别力语料：§4 段内组序（s4scan.py 的 ORDER），专测「带括号初始化式的常量」。
//
// 期望：实跑报 **1 条** ORDER（仅 ParenConstBad）。多一条少一条都算失败。
//
// 钉住的是 classify() 的一个误判：常量与静态成员函数的区别不在有没有括号
// （初始化式 `= (std::numeric_limits<unsigned>::max)() / 2` 本身带括号，函数的默认
// 实参 `void F(int x = 5)` 也带括号），而在**次序**——数据成员的括号在 `=` 之后、
// 函数的括号在名字之后、任何 `=` 之前。旧判据先按 `'(' in d` 判函数，于是这类常量被
// 归进第 5 组：位置写对（第 2 组，常量先于特殊成员）时被报出**与事实相反**的 ORDER，
// 位置写错（排在构造函数之后）时反而沉默——两头都错。
//
// ParenConstOk  —— 带括号初始化式的静态常量写在特殊成员之前 => 必须 0 条
// ParenConstBad —— 同一个常量写在构造函数之后                 => 必须 1 条
//
// 两者缺一不可：没有 Ok 就测不出「位置正确却被报」的误报，没有 Bad 就测不出
// 「把这类常量一律当函数、漏掉真实组序违规」的漏报。
#pragma once

class ParenConstOk
{
public:
    static constexpr unsigned MaxSharedSize = (std::numeric_limits<unsigned>::max)() / 2;

    ParenConstOk();

    void Process();
};

class ParenConstBad
{
public:
    ParenConstBad();

    static constexpr unsigned MaxSharedSize = (std::numeric_limits<unsigned>::max)() / 2;

    void Process();
};
