// 判别力正例（initcheck.py 通道 B，形状「初始化列表里含成员 braced-init」）。
// 期望报告：INIT_ORDER/B 一条。
//
// `second_{2}` 是成员的花括号初始化，也是 `{`。判据若把第一个 `{` 一律当函数体，
// 扫描会在 `second_{2}` 处提前收尾，得到单元素序列 second_ 而无从比较——于是**漏报**。
// 函数体刻意写非空（理由同 initwrapped.cpp）：空体的跨行写法会被 clang-format 塌成 `{}`。
class Braced
{
public:
    Braced();

private:
    int first_;
    int second_;
};

Braced::Braced() : second_{2}, first_(1)
{
    first_ += second_;
}
