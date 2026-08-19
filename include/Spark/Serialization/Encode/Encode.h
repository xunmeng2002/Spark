#pragma once
#include <Spark/Serialization/SerializationExport.h>
#include <string>
#include <cstring>


// ============================================================================
//  NOTE: wstring_convert / codecvt_utf8 / codecvt_byname are deprecated in
//  C++17 and will be removed in C++26.  The deprecation warning is suppressed
//  below because no portable, non-deprecated standard equivalent exists yet
//  (codecvt<wchar_t,char> in <locale> is NOT deprecated but its locale-based
//  behaviour is broken on MSVC for UTF-8, so it is not a drop-in replacement).
//  Revisit for C++26 — expected alternatives: <text_encoding> or <unicode>.
// ============================================================================
namespace spark::serialization
{
std::wstring SERIALIZATION_EXPORTS GbkToUnicode(const std::string& str);
std::string SERIALIZATION_EXPORTS UnicodeToGbk(const std::wstring& str);
std::wstring SERIALIZATION_EXPORTS Utf8ToUnicode(const std::string& str);
std::string SERIALIZATION_EXPORTS UnicodeToUtf8(const std::wstring& str);
std::string SERIALIZATION_EXPORTS GbkToUtf8(const std::string& str);
std::string SERIALIZATION_EXPORTS Utf8ToGbk(const std::string& str);

template <size_t N>
void TrunsferUtf8ToGbk(char (&src)[N])
{
    std::string gbk = Utf8ToGbk(src);
    memset(src, 0, N);
    memcpy(src, gbk.c_str(), (std::min)(gbk.length(), N-1));
}
template <size_t N>
void TrunsferGbkToUtf8(char (&src)[N])
{
    std::string utf8 = GbkToUtf8(src);
    memset(src, 0, N);
    memcpy(src, utf8.c_str(), (std::min)(utf8.length(), N-1));
}
}
