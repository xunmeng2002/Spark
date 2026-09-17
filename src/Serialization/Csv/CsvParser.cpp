#include <Spark/Serialization/Csv/CsvParser.h>
#include <cstring>


namespace Spark::Serialization
{
CsvParser::CsvParser()
{
    csvText_ = nullptr;
    cursor_ = nullptr;
    errorCode_ = CsvParserError::End;

    currentWord_ = new char[TokenMaxLen + 1];
}

CsvParser::CsvParser(const char *csvText)
{
    separator_[0] = ',';
    separator_[1] = '\0';
    csvText_ = csvText;
    cursor_ = (char *)csvText_;
    NextChar();

    currentWord_ = new char[TokenMaxLen + 1];
}

void CsvParser::Parse(const char *csvText)
{
    csvText_ = csvText;
    cursor_ =  (char *)csvText_;
    errorCode_ = CsvParserError::HasNext;
    NextChar();

    currentWord_ = new char[TokenMaxLen + 1];
}

CsvParser::~CsvParser()
{
    delete[] currentWord_;
}
void CsvParser::SetSeparator(char separatorChar)
{
    separator_[0] = separatorChar;
}

void CsvParser::NextChar()
{
    currentChar_ = *cursor_;
    if(currentChar_ == '\0' || currentChar_ == '\r' || currentChar_ == '\n')
    {
        currentChar_ = '\0';
        nextChar_ = '\0';
    }
    else{
        cursor_++;
        nextChar_ = *cursor_;
    }
}

void CsvParser::MakeWord(const char *stopChars)
{
    int i=0;
    for (;i<TokenMaxLen; i++)
    {
        if (currentChar_ == '\0')
        {
            break;
        }
        if (strchr(stopChars, currentChar_) != nullptr)
        {
            if (currentChar_ == '"' && nextChar_ == '"')
            {
                NextChar();
            }
            else
            {
                break;
            }
        }
        currentWord_[i]=currentChar_;
        NextChar();
    }
    currentWord_[i]='\0';
}

char *CsvParser::GetNextToken()
{
    switch (currentChar_)
    {
    case '"':
        NextChar();
        MakeWord("\"");
        if (currentChar_ != '"')
        {
            errorCode_ = CsvParserError::MarkNotMatch;
            return nullptr;
        }
        NextChar();
        break;
    default:
        MakeWord(separator_);
    }
    if (currentChar_ == separator_[0])
    {
        errorCode_ = CsvParserError::HasNext;
        NextChar();
        return currentWord_;
    }
    if (currentChar_ == '\0')
    {
        errorCode_ = CsvParserError::End;
        return currentWord_;
    }

    errorCode_ = CsvParserError::TokenTooLong;
    return nullptr;
}
}
