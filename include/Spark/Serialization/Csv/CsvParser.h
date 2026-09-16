#pragma once
#include <Spark/Serialization/SerializationExport.h>

namespace Spark::Serialization
{
const int TokenMaxLen = 64 * 1024; 

enum class CsvParserError
{
	HasNext,
	End,
	MarkNotMatch,
	TokenTooLong,
};

class SERIALIZATION_EXPORTS CsvParser
{
public:
	CsvParser();
	CsvParser(const char *pszData);
	virtual ~CsvParser();
	void SetSeparator(char chSeparator);
	char *GetNextToken();
	void Parse(const char *pszData);
	inline CsvParserError GetErrorCode();
private:
	void NextChar();
	void MakeWord(const char *pszEnd);
private:
	CsvParserError errorCode_;
	const char *data_;
	char* currWord_;
	char *curr_;
	char chC_;
	char chNC_;
	char separator_[2];
};

inline CsvParserError CsvParser::GetErrorCode()
{
	return errorCode_;
}
}
