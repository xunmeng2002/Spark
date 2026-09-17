// 判别力语料：§4 段内组序（s4scan.py 的 ORDER）
//
// 期望：实跑报 **1 条** ORDER（仅 WrappedBad）。多一条少一条都算失败。
//
// 钉住的是一个「判据与被检写法耦合」的缺陷：类作用域里**跨行的函数声明**，其续行
// 不含 `(` 却以 `;` 结尾，会被 s4scan 的 classify() 当成**数据成员**，于是凭空配出
// 「普通成员函数出现在数据成员之后」的 ORDER 误报。
//
// 该误报出不出来，取决于同一个函数上方的 `template <...>` 是独占一行、还是与函数
// 压成一行——classify() 对独占一行的 `template <...>` 返回 ('other')，对压在函数前
// 的整行也返回 ('other')，唯独函数名落到自己那行时才返回 ('func')。也就是说：
// **只改写法、不改语义，就能让扫描结果凭空多出一条**。2026-09-17 把全仓 template
// 改回独占一行时，它就在 TimeUtility 上现形了（该类的 public 段里没有任何数据成员）。
//
// WrappedOk  —— 跨行声明 + 其下是独占一行的 template 函数，类内无数据成员 => 必须 0 条
// WrappedBad —— 跨行声明 + 真数据成员 + 其后又有函数                 => 必须 1 条
//
// 两者缺一不可：没有 WrappedOk 就测不出误报，没有 WrappedBad 就测不出「把续行一律
// 丢掉」这种过度修复——那会把跨行声明也吞掉，反而漏掉真正的组序违规。
#pragma once

class WrappedOk
{
public:
    static void CalculateRealMinuteBarTime(const char* exchangeId, const char* instrumentId, int calculateBarTime, int& realBarTime,
                                           int& realUpdateTs);

    template <typename T>
    static long long GetDuration(long long start)
    {
        return static_cast<long long>(start) + sizeof(T);
    }
};

class WrappedBad
{
public:
    static void CalculateRealMinuteBarTime(const char* exchangeId, const char* instrumentId, int calculateBarTime, int& realBarTime,
                                           int& realUpdateTs);

    int cachedBarTime;

    static void Refresh();
};
