// 判别力正例（initcheck.py 通道 B）：类外构造函数定义，初始化列表顺序与声明顺序相反。
// 期望报告：INIT_ORDER/B。须与 initpos.h 一起传给脚本（通道 B 靠全仓建类表）。
#include "initpos.h"

class Wide
{
public:
    Wide();

private:
    int first_;
    int second_;
};

Wide::Wide()
    :second_(2), first_(1)
{
}
