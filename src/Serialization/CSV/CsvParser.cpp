#include <Spark/Serialization/Csv/CsvParser.h>
#include <cstring>


namespace spark::serialization
{
CSVParser::CSVParser()
{
	m_pszData = nullptr;
	m_pCurr = nullptr;
	m_nErrorCode = CPE_END;

	m_szCurrWord = new char[TOKEN_MAX_LEN + 1];
}

CSVParser::CSVParser(const char *pszData)
{
	m_chSeparator[0] = ',';
	m_chSeparator[1] = '\0';
	m_pszData = pszData;
	m_pCurr = (char *)m_pszData;
	NextChar();

	m_szCurrWord = new char[TOKEN_MAX_LEN + 1];
}

void CSVParser::Parse(const char *pszData)
{
	m_pszData = pszData;
	m_pCurr =  (char *)m_pszData;
	m_nErrorCode = CPE_HAS_NEXT;
	NextChar();

	m_szCurrWord = new char[TOKEN_MAX_LEN + 1];
}

CSVParser::~CSVParser()
{
	delete m_szCurrWord;
}
void CSVParser::SetSeparator(char chSeparator)
{
	m_chSeparator[0] = chSeparator;
}

void CSVParser::NextChar()
{
	m_chC = *m_pCurr;
	if(m_chC == '\0' || m_chC == '\r' || m_chC == '\n')
	{
		m_chC = '\0';
		m_chNC = '\0';
	}
	else{
		m_pCurr++;
		m_chNC = *m_pCurr;
	}
}

void CSVParser::MakeWord(const char *pszEnd)
{
	int i=0;
	for (;i<TOKEN_MAX_LEN; i++)
	{
		if (m_chC == '\0')
		{
			break;
		}
		if (strchr(pszEnd, m_chC) != nullptr)
		{
			if (m_chC == '"' && m_chNC == '"')
			{
				NextChar();
			}
			else
			{
				break;
			}
		}
		m_szCurrWord[i]=m_chC;
		NextChar();
	}
	m_szCurrWord[i]='\0';
}

char *CSVParser::GetNextToken()
{
	switch (m_chC)
	{
	case '"':
		NextChar();
		MakeWord("\"");
		if (m_chC != '"')
		{
			m_nErrorCode = CPE_MARK_NOT_MATCH;
			return nullptr;
		}
		NextChar();
		break;
	default:
		MakeWord(m_chSeparator);
	}
	if (m_chC == m_chSeparator[0])
	{
		m_nErrorCode = CPE_HAS_NEXT;
		NextChar();
		return m_szCurrWord;
	}
	if (m_chC == '\0')
	{
		m_nErrorCode = CPE_END;
		return m_szCurrWord;
	}

	m_nErrorCode = CPE_TOKEN_TOO_LONG;
	return nullptr;
}
}
