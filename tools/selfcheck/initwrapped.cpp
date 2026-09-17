// 判别力正例（initcheck.py 通道 B，形状「签名与初始化列表同行、`{` 独占次行」）。
// 期望报告：INIT_ORDER/B 一条。
//
// 这是 clang-format 对**放不进一行**的初始化列表的固定输出形状（本仓 `BreakBeforeBraces:
// Allman`），判据若要求 `:` 与 `{` 同行就会整条漏检——实测本仓通道 B 曾由 217 掉到 211。
// 函数体刻意写非空：空体的跨行写法会被 clang-format 塌成 `{}`（另一份语料 initsentinel.cpp
// 专测那个形状），语料一旦被格式化就换了形状，判别力悄然变形。非空体对格式化是稳定的。
class Wrapped
{
public:
    Wrapped();

private:
    int first_;
    int second_;
};

Wrapped::Wrapped() : second_(2), first_(1)
{
    first_ += second_;
}
