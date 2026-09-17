#include <Spark/Serialization/Csv/CsvRecord.h>
#include <Spark/Serialization/Csv/CsvParser.h>
#include <limits>
#include <stdlib.h>

namespace Spark::Serialization
{
CsvRecord::CsvRecord()
{
    nameBufferLen_ = 0;
    contentBufferLen_ = 0;
    separator_ = ',';

    nameBuffer_ = new char[CsvRecordMaxHeadSize];
    contentBuffer_ = new char[CsvRecordMaxContentSize];
}

CsvRecord::~CsvRecord()
{
    delete[] nameBuffer_;
    delete[] contentBuffer_;
}

void CsvRecord::SetSeparator(char separatorChar)
{
    separator_ = separatorChar;
}

char* CsvRecord::AppendNameToken(const char* token)
{
    int tokenLen = (int)strlen(token) + 1;
    char* target = nameBuffer_ + nameBufferLen_;
    memcpy(nameBuffer_ + nameBufferLen_, token, tokenLen);
    nameBufferLen_ += tokenLen;
    return target;
}

char* CsvRecord::AppendContentToken(const char* token)
{
    int tokenLen = (int)strlen(token) + 1;
    char* target = contentBuffer_ + contentBufferLen_;
    memcpy(contentBuffer_ + contentBufferLen_, token, tokenLen);
    contentBufferLen_ += tokenLen;
    return target;
}

bool CsvRecord::AnalysisFieldName(const char* fieldName)
{
    nameBufferLen_ = 0;
    csvFields_.clear();
    csvFieldMap_.clear();

    CsvParser csvParser(fieldName);
    csvParser.SetSeparator(separator_);
    CsvField field = {nullptr, nullptr};

    do
    {
        char* token = csvParser.GetNextToken();
        if (token == nullptr)
        {
            break;
        }
        field.FieldName = AppendNameToken(token);
        csvFields_.push_back(field);
    } while (csvParser.GetErrorCode() == CsvParserError::HasNext);

    return true;
}

bool CsvRecord::AnalysisFieldContent(const char* fieldContent)
{
    contentBufferLen_ = 0;

    CsvParser csvParser(fieldContent);
    csvParser.SetSeparator(separator_);
    for (unsigned int i = 0; i < csvFields_.size(); i++)
    {
        char* token = csvParser.GetNextToken();
        if (token == nullptr)
        {
            break;
        }
        csvFields_[i].FieldContent = AppendContentToken(token);
        csvFieldMap_[csvFields_[i].FieldName] = csvFields_[i].FieldContent;
    }

    return true;
}

const char* CsvRecord::GetFieldAsString(const char* fieldName)
{
    CsvFieldMap::iterator fieldIterator = csvFieldMap_.find(fieldName);
    if (fieldIterator == csvFieldMap_.end())
    {
        return nullptr;
    }

    return fieldIterator->second;
}

char CsvRecord::GetFieldAsChar(const char* fieldName)
{
    const char* fieldContent = GetFieldAsString(fieldName);
    if (fieldContent == nullptr)
    {
        return '\0';
    }
    return *fieldContent;
}
int CsvRecord::GetFieldAsInt(const char* fieldName)
{
    const char* fieldContent = GetFieldAsString(fieldName);
    if (fieldContent == nullptr)
    {
        return 0;
    }
    return atoi(fieldContent);
}

int64_t CsvRecord::GetFieldAsInt64(const char* fieldName)
{
    const char* fieldContent = GetFieldAsString(fieldName);
    if (fieldContent == nullptr)
    {
        return 0;
    }
    return atoll(fieldContent);
}

double CsvRecord::GetFieldAsDouble(const char* fieldName)
{
    const char* fieldContent = GetFieldAsString(fieldName);
    if (fieldContent == nullptr)
    {
        return std::numeric_limits<double>::max();
    }
    if (*fieldContent == '\0')
    {
        return std::numeric_limits<double>::max();
    }

    return atof(fieldContent);
}
}
