#pragma once
#include <Spark/Serialization/SerializationExport.h>
#include <vector>
#include <map>
#include <cstring>
#include <stdint.h>


namespace spark::serialization
{
const int CSV_RECORD_MAX_HEAD_SIZE = 1024;
const int CSV_RECORD_MAX_CONTENT_SIZE = 64 * 1024;

class SERIALIZATION_EXPORTS CSVRecord
{
public:
	CSVRecord();
	virtual ~CSVRecord();
	inline int GetFieldCount();
	inline const char *GetFieldName(int nIndex);
	inline const char *GetFieldContent(int nIndex);

	bool AnalysisFieldName(const char *pszFieldName);
	bool AnalysisFieldContent(const char *pszFieldContent);

	const char *GetFieldAsString(const char *pszFieldName);
	char GetFieldAsChar(const char* pszFieldName);
	int GetFieldAsInt(const char *pszFieldName);
	int64_t GetFieldAsInt64(const char* pszFieldName);
	double GetFieldAsDouble(const char *pszFieldName);
	void SetSeparator(char chSeparator);
private:
	char *AppendNameToken(const char *pszToken);
	char *AppendContentToken(const char *pszToken);
private:
	struct TCSVField
	{
		char *pFieldName;
		char *pFieldContent;
	};

	std::vector<TCSVField> m_CSVFields;

	struct ltstr
	{
	  bool operator()(const char* s1, const char* s2) const
	  {
		return std::strcmp(s1, s2) < 0;
	  }
	};
	typedef std::map<const char*, const char *, ltstr> CCSVFieldMap;
	CCSVFieldMap m_mapCSVField;

	char* m_chNameBuffer;
	int m_nNameBufferLen;
	char* m_chContentBuffer;
	int m_nContentBufferLen;
	char m_chSeparator;
};

inline int CSVRecord::GetFieldCount()
{
	return (int)m_CSVFields.size();
}

inline const char * CSVRecord::GetFieldName(int nIndex)
{
	return m_CSVFields[nIndex-1].pFieldName;
}

inline const char * CSVRecord::GetFieldContent(int nIndex)
{
	return m_CSVFields[nIndex-1].pFieldContent;
}
}
