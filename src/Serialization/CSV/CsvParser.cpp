#include <Spark/Serialization/Csv/CsvParser.h>
#include <cstring>


namespace Spark::Serialization
{
CSVParser::CSVParser()
{
	data_ = nullptr;
	curr_ = nullptr;
	errorCode_ = CPE_END;

	currWord_ = new char[TOKEN_MAX_LEN + 1];
}

CSVParser::CSVParser(const char *pszData)
{
	separator_[0] = ',';
	separator_[1] = '\0';
	data_ = pszData;
	curr_ = (char *)data_;
	NextChar();

	currWord_ = new char[TOKEN_MAX_LEN + 1];
}

void CSVParser::Parse(const char *pszData)
{
	data_ = pszData;
	curr_ =  (char *)data_;
	errorCode_ = CPE_HAS_NEXT;
	NextChar();

	currWord_ = new char[TOKEN_MAX_LEN + 1];
}

CSVParser::~CSVParser()
{
	delete currWord_;
}
void CSVParser::SetSeparator(char chSeparator)
{
	separator_[0] = chSeparator;
}

void CSVParser::NextChar()
{
	chC_ = *curr_;
	if(chC_ == '\0' || chC_ == '\r' || chC_ == '\n')
	{
		chC_ = '\0';
		chNC_ = '\0';
	}
	else{
		curr_++;
		chNC_ = *curr_;
	}
}

void CSVParser::MakeWord(const char *pszEnd)
{
	int i=0;
	for (;i<TOKEN_MAX_LEN; i++)
	{
		if (chC_ == '\0')
		{
			break;
		}
		if (strchr(pszEnd, chC_) != nullptr)
		{
			if (chC_ == '"' && chNC_ == '"')
			{
				NextChar();
			}
			else
			{
				break;
			}
		}
		currWord_[i]=chC_;
		NextChar();
	}
	currWord_[i]='\0';
}

char *CSVParser::GetNextToken()
{
	switch (chC_)
	{
	case '"':
		NextChar();
		MakeWord("\"");
		if (chC_ != '"')
		{
			errorCode_ = CPE_MARK_NOT_MATCH;
			return nullptr;
		}
		NextChar();
		break;
	default:
		MakeWord(separator_);
	}
	if (chC_ == separator_[0])
	{
		errorCode_ = CPE_HAS_NEXT;
		NextChar();
		return currWord_;
	}
	if (chC_ == '\0')
	{
		errorCode_ = CPE_END;
		return currWord_;
	}

	errorCode_ = CPE_TOKEN_TOO_LONG;
	return nullptr;
}
}
