#include "WriteJFile1.h"
#include "CDatabase.h"
#include "charset.h"
#include "Utilities.h"
#include "ConverterErrors.h"
#include "JFile1.h"

CWriteJFile1::CWriteJFile1(CDataSource* sourceData, Boolean translateText)
	: CWritePDB(sourceData, translateText, kType, kCreator)
{
	if (fDatabase->RecordCount() > MAX_RECORDS)
		Throw_(kTooManyRecords);
}

CWriteJFile1::~CWriteJFile1()
{
}

void
CWriteJFile1::WriteAppInfoBlock(LStream* targetStream, vector<string>& fieldNames, Boolean translateText)
{
	vector<int> fieldWidths;
	
	fieldWidths.push_back(160);
	for (int i = 1; i < fieldNames.size(); i += 1) {
		fieldWidths.push_back(0);
	}
	WriteAppInfoBlock(targetStream, fieldNames, fieldWidths, translateText);
}

void
CWriteJFile1::WriteAppInfoBlock(LStream* targetStream, vector<string>& fieldNames, 
		const vector<int>& fieldWidths, Boolean translateText)
{
	JFile1AppInfoType appInfo;
	Clear(appInfo);
	int fieldCount = fieldNames.size();
	if (fieldCount > MAX_FIELDS)
		Throw_(kTooManyFields);
	
	for (int i = 0; i < fieldCount; i += 1) {
		string fieldName = fieldNames[i];
	 	if (translateText)
			ConvertToPilotText(fieldName);
		if (fieldName.length() > MAX_FIELD_NAME_LENGTH)
			Throw_(kFieldNameTooLongError);
		const char *p = fieldName.c_str();
		strncpy(appInfo.fieldNames[i], p, MAX_FIELD_NAME_LENGTH-1);
	}

	appInfo.numFields = fieldCount;
	appInfo.showDataWidth = 80;
	for(int i = 0; i < fieldCount; i += 1)
		appInfo.showDBColumnWidths[i] = fieldWidths[i];
	appInfo.showDBColumns = fieldCount;

	targetStream->WriteBlock(&appInfo, sizeof(JFile1AppInfoType));	
}

void
CWriteJFile1::WriteRecord(LStream* targetStream, const vector<string>& fields, Boolean translateText)
{
	int recordSize = 0;
	int fieldCount = fields.size();
	for(int j = 0; j< fieldCount; j++) {
		string fieldData = fields[j];
	 	if (translateText)
			ConvertToPilotText(fieldData);
		ConvertVerticalTabToPilotReturn(fieldData);
		int fieldSize = fieldData.length() + 1;
		if (fieldSize > MAX_DATA_LENGTH)
			Throw_(kFieldDataTooLong);
		const char *p = fieldData.c_str();
 	 	targetStream->WriteBlock(p, fieldSize);
 	 	recordSize += fieldSize;
 	}
 	if (recordSize > MAX_RECORD_LENGTH)
 		Throw_(kRecordDataTooLong);
}

void
CWriteJFile1::WriteApplicationInfo(LStream& outStream)
{
	vector<int> fieldWidths;
	int fieldCount = fDatabase->FieldCount();
	for (int i = 0; i < fieldCount; i += 1) {
		fieldWidths.push_back(0);
	}
	
	int totalWidth = 160;
	const int kPixelsPerCharacter = 5;
	const int kMaxPerField = 80;
	int recordCount = fDatabase->RecordCount();
	for (int i = 0; i < fieldCount && totalWidth > 0; i += 1) {
		for (int j = 0; j < recordCount; j += 1) {
			vector<string> record;
			fDatabase->GetRecord(j, record);
			int fieldDataLength = record[i].size() - 1;
			int fieldPixels = fieldDataLength * kPixelsPerCharacter;
			if (fieldPixels > kMaxPerField)
				fieldPixels = kMaxPerField;
			if (fieldPixels > totalWidth)
				fieldPixels = totalWidth;
			if (fieldPixels > fieldWidths[i])
				fieldWidths[i] = fieldPixels;
		}
		totalWidth -= fieldWidths[i];
	}
	if (totalWidth > 0) {
		fieldWidths[fieldCount - 1] += totalWidth;
	}
	vector<string> fieldNames;
	fDatabase->GetFieldNames(fieldNames);
	WriteAppInfoBlock(&outStream, fieldNames, fieldWidths, fTranslateText);
}

UInt8
CWriteJFile1::WriteRecord(UInt32 recordIndex, LStream& outStream)
{
	vector<string> fields;
	fDatabase->GetRecord(recordIndex, fields);
	WriteRecord(&outStream, fields, fTranslateText);
	return 0;
}
