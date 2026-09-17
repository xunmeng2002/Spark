// 判别力正例（initcheck.py 通道 A）：类内构造函数，初始化列表顺序与声明顺序相反。
// 期望报告：INIT_ORDER/A
class InitPos
{
public:
    InitPos() : second_(2), first_(1) {}

private:
    int first_;
    int second_;
};
