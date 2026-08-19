#include <Spark/Serialization/Csv/CsvRecord.h>
#include <Spark/Serialization/Csv/CsvParser.h>
#include <limits>
#include <stdlib.h>

namespace spark::serialization
{
CSVRecord::CSVRecord()
{
	m_nNameBufferLen = 0;
	m_nContentBufferLen = 0;
	m_chSeparator = ',';

	m_chNameBuffer = new char[CSV_RECORD_MAX_HEAD_SIZE];
	m_chContentBuffer = new char[CSV_RECORD_MAX_CONTENT_SIZE];
}

CSVRecord::~CSVRecord()
{
	delete m_chNameBuffer;
	delete m_chContentBuffer;
}

void CSVRecord::SetSeparator(char chSeparator)
{
	m_chSeparator = chSeparator;
}

char *CSVRecord::AppendNameToken(const char *pszToken)
{
	int nTokenLen = (int)strlen(pszToken) + 1;
	char *pszTarget = m_chNameBuffer+m_nNameBufferLen;
	memcpy(m_chNameBuffer+m_nNameBufferLen, pszToken, nTokenLen);
	m_nNameBufferLen += nTokenLen;
	return pszTarget;
}

char *CSVRecord::AppendContentToken(const char *pszToken)
{
	int nTokenLen = (int)strlen(pszToken) + 1;
	char *pszTarget = m_chContentBuffer+m_nContentBufferLen;
	memcpy(m_chContentBuffer+m_nContentBufferLen, pszToken, nTokenLen);
	m_nContentBufferLen += nTokenLen;
	return pszTarget;
}

bool CSVRecord::AnalysisFieldName(const char *pszFieldName)
{
	m_nNameBufferLen = 0;
	m_CSVFields.clear();
	m_mapCSVField.clear();
	
	CSVParser csvParser(pszFieldName);
	csvParser.SetSeparator(m_chSeparator);
	TCSVField field = {nullptr, nullptr};

	do
	{
		char *pszToken = csvParser.GetNextToken();
		if (pszToken == nullptr)
		{
			break;
		}
		field.pFieldName = AppendNameToken(pszToken);
		m_CSVFields.push_back(field);
	}while (csvParser.GetErrorCode() == CPE_HAS_NEXT);

	return true;
}

bool CSVRecord::AnalysisFieldContent(const char *pszFieldContent)
{
	m_nContentBufferLen = 0;
	
	CSVParser csvParser(pszFieldContent);
	csvParser.SetSeparator(m_chSeparator);
	for (unsigned int i = 0; i<m_CSVFields.size(); i++)
	{
		char *pszToken = csvParser.GetNextToken();
		if (pszToken == nullptr)
		{
			break;
		}
		m_CSVFields[i].pFieldContent = AppendContentToken(pszToken);
		m_mapCSVField[m_CSVFields[i].pFieldName] = m_CSVFields[i].pFieldContent;
	}

	return true;
}

const char* CSVRecord::GetFieldAsString(const char *pszFieldName)
{
	CCSVFieldMap::iterator itor = m_mapCSVField.find(pszFieldName);
	if (itor == m_mapCSVField.end())
	{
		return nullptr;
	}

	return (*itor).second;
}

char CSVRecord::GetFieldAsChar(const char* pszFieldName)
{
	const char* pszFieldContent = GetFieldAsString(pszFieldName);
	if (pszFieldContent == nullptr)
	{
		return '\0';
	}
	return *pszFieldContent;
}
int CSVRecord::GetFieldAsInt(const char *pszFieldName)
{
	const char *pszFieldContent = GetFieldAsString(pszFieldName);
	if (pszFieldContent == nullptr)
	{
		return 0;
	}
	return atoi(pszFieldContent);
}

int64_t CSVRecord::GetFieldAsInt64(const char* pszFieldName)
{
	const char* pszFieldContent = GetFieldAsString(pszFieldName);
	if (pszFieldContent == nullptr)
	{
		return 0;
	}
	return atoll(pszFieldContent);
}


double CSVRecord::GetFieldAsDouble(const char *pszFieldName)
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
using namespace spark::serialization;
int main()
{

	char *pszFieldName = "name,age,money";
	char *pszFieldContent = "\"peter pan\",\"20\",\"123.5\"";
	CSVRecord record;
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
