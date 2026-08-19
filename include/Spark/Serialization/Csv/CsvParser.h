#pragma once
#include <Spark/Serialization/SerializationExport.h>

namespace spark::serialization
{
const int TOKEN_MAX_LEN = 64 * 1024; 

enum CSV_PARSER_ERROR
{
	CPE_HAS_NEXT,
	CPE_END,
	CPE_MARK_NOT_MATCH,
	CPE_TOKEN_TOO_LONG,
};

class SERIALIZATION_EXPORTS CSVParser
{
public:
	CSVParser();
	CSVParser(const char *pszData);
	virtual ~CSVParser();
	void SetSeparator(char chSeparator);
	char *GetNextToken();
	void Parse(const char *pszData);
	inline CSV_PARSER_ERROR GetErrorCode();
private:
	void NextChar();
	void MakeWord(const char *pszEnd);
private:
	CSV_PARSER_ERROR m_nErrorCode;
	const char *m_pszData;
	char* m_szCurrWord;
	char *m_pCurr;
	char m_chC;
	char m_chNC;
	char m_chSeparator[2];
};

inline CSV_PARSER_ERROR CSVParser::GetErrorCode()
{
	return m_nErrorCode;
}
}
