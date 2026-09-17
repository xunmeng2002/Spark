// 判别力反例（s4scan.py）：完全合规，扫描器必须 0 命中。
// 含 §4 末条例外的正确写法：单例私有特殊成员置顶、该区只放特殊成员、随后立即 public:。
class Neg
{
public:
    using Ptr = Neg*;
    static constexpr int Max = 10;

    Neg();
    ~Neg();
    Neg& operator=(const Neg&) = delete;
    bool operator==(const Neg&) const;

    void Process();

protected:
    void Validate();

private:
    Neg(int id);

    void Initialize();

    int id_;
};

class Singleton
{
private:
    Singleton();
    ~Singleton();

public:
    static Singleton& GetInstance();

private:
    int state_;
};
