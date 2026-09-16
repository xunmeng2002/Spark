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
	delete nameBuffer_;
	delete contentBuffer_;
}

void CsvRecord::SetSeparator(char chSeparator)
{
	separator_ = chSeparator;
}

char *CsvRecord::AppendNameToken(const char *pszToken)
{
	int nTokenLen = (int)strlen(pszToken) + 1;
	char *pszTarget = nameBuffer_+nameBufferLen_;
	memcpy(nameBuffer_+nameBufferLen_, pszToken, nTokenLen);
	nameBufferLen_ += nTokenLen;
	return pszTarget;
}

char *CsvRecord::AppendContentToken(const char *pszToken)
{
	int nTokenLen = (int)strlen(pszToken) + 1;
	char *pszTarget = contentBuffer_+contentBufferLen_;
	memcpy(contentBuffer_+contentBufferLen_, pszToken, nTokenLen);
	contentBufferLen_ += nTokenLen;
	return pszTarget;
}

bool CsvRecord::AnalysisFieldName(const char *pszFieldName)
{
	nameBufferLen_ = 0;
	csvFields_.clear();
	csvFieldMap_.clear();
	
	CsvParser csvParser(pszFieldName);
	csvParser.SetSeparator(separator_);
	CsvField field = {nullptr, nullptr};

	do
	{
		char *pszToken = csvParser.GetNextToken();
		if (pszToken == nullptr)
		{
			break;
		}
		field.FieldName = AppendNameToken(pszToken);
		csvFields_.push_back(field);
	}while (csvParser.GetErrorCode() == CsvParserError::HasNext);

	return true;
}

bool CsvRecord::AnalysisFieldContent(const char *pszFieldContent)
{
	contentBufferLen_ = 0;
	
	CsvParser csvParser(pszFieldContent);
	csvParser.SetSeparator(separator_);
	for (unsigned int i = 0; i<csvFields_.size(); i++)
	{
		char *pszToken = csvParser.GetNextToken();
		if (pszToken == nullptr)
		{
			break;
		}
		csvFields_[i].FieldContent = AppendContentToken(pszToken);
		csvFieldMap_[csvFields_[i].FieldName] = csvFields_[i].FieldContent;
	}

	return true;
}

const char* CsvRecord::GetFieldAsString(const char *pszFieldName)
{
	CsvFieldMap::iterator itor = csvFieldMap_.find(pszFieldName);
	if (itor == csvFieldMap_.end())
	{
		return nullptr;
	}

	return (*itor).second;
}

char CsvRecord::GetFieldAsChar(const char* pszFieldName)
{
	const char* pszFieldContent = GetFieldAsString(pszFieldName);
	if (pszFieldContent == nullptr)
	{
		return '\0';
	}
	return *pszFieldContent;
}
int CsvRecord::GetFieldAsInt(const char *pszFieldName)
{
	const char *pszFieldContent = GetFieldAsString(pszFieldName);
	if (pszFieldContent == nullptr)
	{
		return 0;
	}
	return atoi(pszFieldContent);
}

int64_t CsvRecord::GetFieldAsInt64(const char* pszFieldName)
{
	const char* pszFieldContent = GetFieldAsString(pszFieldName);
	if (pszFieldContent == nullptr)
	{
		return 0;
	}
	return atoll(pszFieldContent);
}


double CsvRecord::GetFieldAsDouble(const char *pszFieldName)
{
	const char *pszFieldContent = GetFieldAsString(pszFieldName);
	if (pszFieldContent == nullptr)
	{
		return std::numeric_limits<double>::max();
	}
	if (*pszFieldContent == '\0')
	{
		return std::numeric_limits<double>::max();
	}
	
	return atof(pszFieldContent);
}
}

#if 0
using namespace Spark::Serialization;
int main()
{

	char *pszFieldName = "name,age,money";
	char *pszFieldContent = "\"peter pan\",\"20\",\"123.5\"";
	CsvRecord record;
	if (!record.Analysis(pszFieldName, pszFieldContent))
	{
		printf("Analysis fail\n");
		return -1;
	}

	int i = 0;
	for (i=1; i<=record.GetFieldCount(); i++)
	{
		printf("[%s] : [%s]\n", record.GetFieldName(i), record.GetFieldContent(i));
	}
	
	printf("name = [%s]\n", record.GetFieldAsString("name"));
	printf("age = [%d]\n", record.GetFieldAsInt("age"));
	printf("money = [%lf]\n", record.GetFieldAsDouble("money"));

	printf("error str = [%s]\n", record.GetFieldAsString("error str"));
	printf("error int = [%d]\n", record.GetFieldAsInt("error int"));
	printf("error double = [%lf]\n", record.GetFieldAsDouble("error double"));

	return 0;
}
#endif
