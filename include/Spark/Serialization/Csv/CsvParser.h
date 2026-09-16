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
	CsvParser(const char *csvText);
	virtual ~CsvParser();
	void SetSeparator(char separatorChar);
	char *GetNextToken();
	void Parse(const char *csvText);
	inline CsvParserError GetErrorCode();
private:
	void NextChar();
	void MakeWord(const char *stopChars);
private:
	CsvParserError errorCode_;
	const char *csvText_;
	char* currentWord_;
	char *cursor_;
	char currentChar_;
	char nextChar_;
	char separator_[2];
};

inline CsvParserError CsvParser::GetErrorCode()
{
	return errorCode_;
}
}
