// 判别力哨兵（initcheck.py 通道 B，形状「整条构造函数压成一行、函数体为空体 `{}`」）。
// 期望报告：只有 1 条（Gamma 真乱序）；Alpha 与 Delta 都合规。
//
// 哨兵怎么起作用：`Alpha::Alpha() : first_(1), second_(2) {}` 本身合规，但它的空体 `{}`
// 也是 `{`。判据若按「配对 `}` 是否在同一行」判 braced-init，就会把 `{}` 误认成成员初始化，
// 于是越过函数体继续往后扫——把 Gamma 的 `second_(2), first_(1)` 和 Delta 的
// `first_(1), second_(2)` 一并收进 Alpha 的成员序列，Alpha 于是被判成乱序，**多报**一条。
// 故期望条数是钉住「多报」的：越界则 2 条，正确则 1 条。
//
// 末尾的 Delta 是刻意留的收尾点，且它的函数体**跨行且非空**：跨行才能让越界扫描停下来，
// 非空才不会被 clang-format 塌成 `{}`、让收尾点自己变成又一个 `{}`。没有这个收尾点，
// 越界扫描会一路扫到文件尾、判空返回 None，症状变成「漏报 0 条」，与真实仓里的现象
// （收进下一个函数的限定名）不是同一回事。
//
// Alpha 与 Gamma 的成员名刻意相同，好让越界内容落进 Alpha 的成员表里被 judge 看见。
class Alpha
{
public:
    Alpha();

private:
    int first_;
    int second_;
};

class Gamma
{
public:
    Gamma();

private:
    int first_;
    int second_;
};

class Delta
{
public:
    Delta();

private:
    int first_;
    int second_;
};

Alpha::Alpha() : first_(1), second_(2) {}
Gamma::Gamma() : second_(2), first_(1) {}
Delta::Delta() : first_(1), second_(2)
{
    first_ += second_;
}
