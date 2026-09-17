// 判别力反例（initcheck.py）：初始化列表顺序与声明顺序一致，必须 0 命中。
class Ok
{
public:
    Ok() : first_(1), second_(2) {}

private:
    int first_;
    int second_;
};
