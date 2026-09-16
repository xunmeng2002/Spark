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
	inline const char *GetFieldName(int fieldIndex);
	inline const char *GetFieldContent(int fieldIndex);

	bool AnalysisFieldName(const char *fieldName);
	bool AnalysisFieldContent(const char *fieldContent);

	const char *GetFieldAsString(const char *fieldName);
	char GetFieldAsChar(const char* fieldName);
	int GetFieldAsInt(const char *fieldName);
	int64_t GetFieldAsInt64(const char* fieldName);
	double GetFieldAsDouble(const char *fieldName);
	void SetSeparator(char separatorChar);
private:
	char *AppendNameToken(const char *token);
	char *AppendContentToken(const char *token);
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

inline const char * CsvRecord::GetFieldName(int fieldIndex)
{
	return csvFields_[fieldIndex-1].FieldName;
}

inline const char * CsvRecord::GetFieldContent(int fieldIndex)
{
	return csvFields_[fieldIndex-1].FieldContent;
}
}
