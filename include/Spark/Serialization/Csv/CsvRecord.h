#pragma once
#include <Spark/Serialization/SerializationExport.h>
#include <vector>
#include <map>
#include <cstring>
#include <stdint.h>


namespace Spark::Serialization
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
		char *FieldName;
		char *FieldContent;
	};

	std::vector<TCSVField> csvFields_;

	struct ltstr
	{
	  bool operator()(const char* s1, const char* s2) const
	  {
		return std::strcmp(s1, s2) < 0;
	  }
	};
	typedef std::map<const char*, const char *, ltstr> CCSVFieldMap;
	CCSVFieldMap m_mapCSVField;

	char* nameBuffer_;
	int nameBufferLen_;
	char* contentBuffer_;
	int contentBufferLen_;
	char separator_;
};

inline int CSVRecord::GetFieldCount()
{
	return (int)csvFields_.size();
}

inline const char * CSVRecord::GetFieldName(int nIndex)
{
	return csvFields_[nIndex-1].FieldName;
}

inline const char * CSVRecord::GetFieldContent(int nIndex)
{
	return csvFields_[nIndex-1].FieldContent;
}
}
