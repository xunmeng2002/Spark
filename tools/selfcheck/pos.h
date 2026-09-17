// 判别力正例（s4scan.py）：每一处违反 §4 结构条款，扫描器必须报警。
// 期望报告：ACCESS_ORDER ×3 / DUP_LABEL ×2 / ORDER ×2 / NO_BLANK ×2 / NOT_FLUSH ×1（共 10 条）
class Pos
{
private:
    int a_;
public:
    int Value;
    void Process();
public:
    void Other();

protected:
    void Hook();

private:
    void Helper();
    int b_;
    void Late();
};

namespace N
{
    class Indented
    {
        public:
        void A();
    };
}
