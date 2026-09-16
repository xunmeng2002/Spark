#pragma once
#include <Spark/Serialization/SerializationExport.h>
#include <vector>
#include <map>
#include <cstring>
#include <stdint.h>


namespace Spark::Serialization
{
const int CsvRecordMaxHeadSize = 1024;
const int CsvRecordMaxContentSize = 64 * 1024;

class SERIALIZATION_EXPORTS CsvRecord
{
public:
	CsvRecord();
	virtual ~CsvRecord();
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
	struct CsvField
	{
		char *FieldName;
		char *FieldContent;
	};

	std::vector<CsvField> csvFields_;

	struct CsvFieldLess
	{
	  bool operator()(const char* s1, const char* s2) const
	  {
		return std::strcmp(s1, s2) < 0;
	  }
	};
	typedef std::map<const char*, const char *, CsvFieldLess> CsvFieldMap;
	CsvFieldMap csvFieldMap_;

	char* nameBuffer_;
	int nameBufferLen_;
	char* contentBuffer_;
	int contentBufferLen_;
	char separator_;
};

inline int CsvRecord::GetFieldCount()
{
	return (int)csvFields_.size();
}

inline const char * CsvRecord::GetFieldName(int nIndex)
{
	return csvFields_[nIndex-1].FieldName;
}

inline const char * CsvRecord::GetFieldContent(int nIndex)
{
	return csvFields_[nIndex-1].FieldContent;
}
}
