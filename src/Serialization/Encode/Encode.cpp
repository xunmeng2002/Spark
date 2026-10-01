#include <Spark/Serialization/Encode/Encode.h>
#include <locale>
#include <codecvt>

namespace Spark::Serialization
{
class codecvt_gbk : public std::codecvt_byname<wchar_t, char, std::mbstate_t>
{
public:
    codecvt_gbk()
#ifdef _WIN32
        : codecvt_byname("zh_CN")
#else
        : codecvt_byname("zh_CN.GB18030")
#endif
    {
    }
};

static std::wstring_convert<codecvt_gbk> GbkConvert;
static std::wstring_convert<std::codecvt_utf8<wchar_t>> Utf8Convert;

std::wstring GbkToUnicode(const std::string& str)
{
    return GbkConvert.from_bytes(str);
}
std::string UnicodeToGbk(const std::wstring& str)
{
    return GbkConvert.to_bytes(str);
}
std::wstring Utf8ToUnicode(const std::string& str)
{
    return Utf8Convert.from_bytes(str);
}
std::string UnicodeToUtf8(const std::wstring& str)
{
    return Utf8Convert.to_bytes(str);
}
std::string GbkToUtf8(const std::string& str)
{
    return Utf8Convert.to_bytes(GbkConvert.from_bytes(str));
}
std::string Utf8ToGbk(const std::string& str)
{
    return GbkConvert.to_bytes(Utf8Convert.from_bytes(str));
}
}
