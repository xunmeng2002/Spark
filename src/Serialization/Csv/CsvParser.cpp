#include <Spark/Serialization/Csv/CsvParser.h>
#include <cstring>


namespace Spark::Serialization
{
CsvParser::CsvParser()
{
	data_ = nullptr;
	curr_ = nullptr;
	errorCode_ = CsvParserError::End;

	currWord_ = new char[TokenMaxLen + 1];
}

CsvParser::CsvParser(const char *pszData)
{
	separator_[0] = ',';
	separator_[1] = '\0';
	data_ = pszData;
	curr_ = (char *)data_;
	NextChar();

	currWord_ = new char[TokenMaxLen + 1];
}

void CsvParser::Parse(const char *pszData)
{
	data_ = pszData;
	curr_ =  (char *)data_;
	errorCode_ = CsvParserError::HasNext;
	NextChar();

	currWord_ = new char[TokenMaxLen + 1];
}

CsvParser::~CsvParser()
{
	delete[] currWord_;
}
void CsvParser::SetSeparator(char chSeparator)
{
	separator_[0] = chSeparator;
}

void CsvParser::NextChar()
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

void CsvParser::MakeWord(const char *pszEnd)
{
	int i=0;
	for (;i<TokenMaxLen; i++)
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

char *CsvParser::GetNextToken()
{
	switch (chC_)
	{
	case '"':
		NextChar();
		MakeWord("\"");
		if (chC_ != '"')
		{
			errorCode_ = CsvParserError::MarkNotMatch;
			return nullptr;
		}
		NextChar();
		break;
	default:
		MakeWord(separator_);
	}
	if (chC_ == separator_[0])
	{
		errorCode_ = CsvParserError::HasNext;
		NextChar();
		return currWord_;
	}
	if (chC_ == '\0')
	{
		errorCode_ = CsvParserError::End;
		return currWord_;
	}

	errorCode_ = CsvParserError::TokenTooLong;
	return nullptr;
}
}
