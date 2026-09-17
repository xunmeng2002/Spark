// 判别力正例（s4scan.py）：首个成员前无访问标签，依赖默认访问。
// 期望报告：NO_FIRST_LABEL ×2（class 与 struct 各一条，共 2 条）。
// 注意 struct 那条**会**被报出：用户裁定「struct 不必显式写 public:」是**报告侧的既定例外**
// （全仓 24 条同此），检查器并未在判据侧排除 struct。故本文件是正例，不是反例。
class NoFirst
{
    void A();
};

struct NoFirstStruct
{
    int x;
};
