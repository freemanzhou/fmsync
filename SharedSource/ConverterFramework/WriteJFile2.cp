#include "WriteJFile2.h"
#include "CDatabase.h"
#include "charset.h"
#include "Utilities.h"
#include "ConverterErrors.h"
#include "CDatabaseInfo.h"
#include "JFile.h"

const int kMaxPerField = 75;

CWriteJFile2::CWriteJFile2(CDataSource* sourceData, Boolean translateText)
	: CWritePDB(sourceData, translateText, kType, kCreator)
{
	if (fDatabase->RecordCount() > MAX_RECORDS)
		Throw_(kTooManyRecords);
}

CWriteJFile2::~CWriteJFile2()
{
}

void
CWriteJFile2::WriteAppInfoBlock(LStream* targetStream, vector<string>& fieldNames, Boolean translateText)
{
	vector<int> fieldWidths;
	
	fieldWidths.push_back(160);
	for (int i = 1; i < fieldNames.size(); i += 1) {
		fieldWidths.push_back(0);
	}
	WriteAppInfoBlock(targetStream, fieldNames, fieldWidths, translateText);
}

void
CWriteJFile2::WriteAppInfoBlock(LStream* targetStream, vector<string>& fieldNames, 
		const vector<int>& fieldWidths, Boolean translateText)
{
	vector<int> fieldTypes;
	vector<string> choices;
	vector<string_vector> noChoices;
	
	
	fieldTypes.push_back(160);
	for (int i = 1; i < fieldNames.size(); i += 1) {
		fieldTypes.push_back(FLDTYPE_STRING);
		noChoices.push_back(choices);
	}
	WriteAppInfoBlock(targetStream, fieldNames, fieldWidths, fieldTypes, noChoices, translateText);
}

void
CWriteJFile2::WriteAppInfoBlock(LStream* targetStream, const CFieldIDList& fieldIDs, const CDatabaseInfo& info, Boolean translateText)
{
	JFile2AppInfoType appInfo = {};
	int fieldCount = fieldIDs.size();
	if (fieldCount > MAX_FIELDS)
		Throw_(kTooManyFields);
	
	int i;
	for (i = 0; i < fieldCount; i += 1) {
		FMAE::FieldID fieldID = fieldIDs[i];
		string fieldName;
		info.FieldName(fieldID, fieldName);
	 	if (translateText)
			ConvertToPilotText(fieldName);
#ifndef CONDUIT
		if (fieldName.length() > MAX_FIELD_NAME_LENGTH)
			Throw_(kFieldNameTooLongError);
#endif
		const char *p = fieldName.c_str();
		strncpy(appInfo.fieldNames[i], p, MAX_FIELD_NAME_LENGTH);
		string_vector popupChoices;
		info.PopupValues(fieldID, popupChoices);
		if (popupChoices.size() > 0)
			appInfo.fieldTypes[i] = FLDTYPE_LIST;
		else {
			int fieldType;
			info.FieldType(fieldID, fieldType);
			appInfo.fieldTypes[i] = fieldType;
		}
		#if LOCK_FIELDS_IN_JFILE
		int access;
		info.FieldAccess(fieldID, access);
		if ((access & kFileMakerWriteAccessMask) == 0) {
			appInfo.fieldTypes[i] |= FLDFLAG_READONLY;
		}
		#endif

		int columnWidth;
		if (info.ColumnWidth(fieldID, columnWidth))
			appInfo.showDBColumnWidths[i] = columnWidth;
		else
			appInfo.showDBColumnWidths[i] = kMaxPerField;
	}
	
	for (;i < MAX_FIELDS; i++) {
		appInfo.showDBColumnWidths[i] = 75;
		appInfo.fieldTypes[i] = FLDTYPE_STRING;
	}

	appInfo.numFields = fieldCount;
	appInfo.showDataWidth = info.fShowDataWidth;
	appInfo.sort1Field = info.fSortFields[0];
	appInfo.sort2Field = info.fSortFields[1];
	appInfo.sort3Field = info.fSortFields[2];
	appInfo.findField = info.fFindField;
	appInfo.filterField = info.fFilterField;
	appInfo.flags = info.fFlags;
	appInfo.firstColumnToShow = info.fFirstColumnToShow;
	strncpy(appInfo.password, info.fPassword.c_str(), MAX_PASSWORD_LENGTH);
	strncpy(appInfo.findString, info.fFindString.c_str(), MAX_FIND_STRING);
	strncpy(appInfo.filterString, info.fFilterString.c_str(), MAX_FIND_STRING);
	appInfo.version = CURRENT_VERSION;

	targetStream->WriteBlock(&appInfo, sizeof(JFile2AppInfoType));
	LHandleStream hStream;
	WritePopupChoices(&hStream, fieldCount, fieldIDs, info, translateText);
	Handle h = hStream.GetDataHandle();
	int choicesLength = ::GetHandleSize(h);
	if (choicesLength > MAX_TOTAL_POPUP_LENGTH)
		Throw_(kPopupChoicesTooLong);
	::HLock(h);
	targetStream->WriteBlock(*h, choicesLength);
}

void
CWriteJFile2::WritePopupChoices(LStream* targetStream, int fieldCount, const CFieldIDList& fieldIDs, const CDatabaseInfo& info, bool translateText)
{
	if (info.fPopupValues.size() == 0) {
		long nullPopups = 0;
		targetStream->WriteBlock(&nullPopups, sizeof(nullPopups));
	} else {
		char c = 'a';
		for (int i = 0; i < fieldCount; i += 1) {
			FMAE::FieldID fieldID = fieldIDs[i];
			int popupCount = 0;
			string_vector popupChoices;
			info.PopupValues(fieldID, popupChoices);
			vector<string>::const_iterator pIter = popupChoices.begin();
			if (pIter != popupChoices.end() && popupCount < MAX_POPUP_LENGTH) {
				string popupLabel("popup");
				popupLabel += c;
				targetStream->WriteBlock(popupLabel.c_str(), popupLabel.length()+1);
				while (pIter != popupChoices.end()) {
					string popupValueString(*pIter);
				 	if (translateText)
						ConvertToPilotText(popupValueString);
					targetStream->WriteBlock(popupValueString.c_str(), popupValueString.length()+1);
					++pIter;
					++popupCount;
				}
			}
			c += 1;
		}
		short endPopups = 0;
		targetStream->WriteBlock(&endPopups, sizeof(endPopups));
	}
}

void
CWriteJFile2::WriteAppInfoBlock(LStream* targetStream, vector<string>& fieldNames, 
				const vector<int>& fieldWidths, const vector<int>& fieldTypes, const vector<string_vector>& popups, Boolean translateText)
{
	JFile2AppInfoType appInfo = {};
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
		if (popups[i].size() > 0)
			appInfo.fieldTypes[i] = FLDTYPE_LIST;
		else
			appInfo.fieldTypes[i] = fieldTypes[i];
	}

	appInfo.numFields = fieldCount;
	for(int i = 0; i < fieldCount; i += 1) {
		int fieldWidth = fieldWidths[i];
		appInfo.showDBColumnWidths[i] = fieldWidth;
	}
	appInfo.showDataWidth = 110;
	appInfo.firstColumnToShow = 1;
	appInfo.version = CURRENT_VERSION;

	targetStream->WriteBlock(&appInfo, sizeof(JFile2AppInfoType));
	if (popups.size() == 0) {
		long nullPopups = 0;
		targetStream->WriteBlock(&nullPopups, sizeof(nullPopups));
	} else {
		char c = 'a';
		vector<string_vector>::const_iterator vIter = popups.begin();
		while (vIter != popups.end()) {
			vector<string>::const_iterator pIter = vIter->begin();
			if (pIter != vIter->end()) {
				string popupLabel("popup");
				popupLabel += c;
				targetStream->WriteBlock(popupLabel.c_str(), popupLabel.length()+1);
				while (pIter != vIter->end()) {
					string popupValue(*pIter);
				 	if (translateText)
						ConvertToPilotText(popupValue);
					targetStream->WriteBlock(popupValue.c_str(), popupValue.length()+1);
					++pIter;
				}
			}
			c += 1;
			++vIter;
		}
		short endPopups = 0;
		targetStream->WriteBlock(&endPopups, sizeof(endPopups));
	}
}

void
CWriteJFile2::WriteRecord(LStream* targetStream, const vector<string>& fields, Boolean translateText)
{
	int recordSize = 0;
	int fieldCount = fields.size();
	for(int j = 0; j< fieldCount; j++) {
		string fieldData = fields[j];
	 	if (translateText)
			ConvertToPilotText(fieldData);
		ConvertToPilotReturns(fieldData);
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
CWriteJFile2::WriteApplicationInfo(LStream& outStream)
{
	int fieldCount = fDatabase->FieldCount();
	vector<int> fieldWidths;
	vector<int> fieldTypes;
	vector<string_vector> fieldChoices;
	fDatabase->GetFieldTypes(fieldTypes);
	fDatabase->GetFieldChoices(fieldChoices);
	int recordCount = fDatabase->RecordCount();
	int defaultWidth = 40;
	for (int i = 0; i < fieldCount; i += 1) {
		fieldWidths.push_back(defaultWidth);
	}
	
	int totalWidth = 1600;
	const int kPixelsPerCharacter = 5;
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
	vector<string> fieldNames;
	fDatabase->GetFieldNames(fieldNames);
	WriteAppInfoBlock(&outStream, fieldNames, fieldWidths, fieldTypes, fieldChoices, fTranslateText);
}

UInt8
CWriteJFile2::WriteRecord(UInt32 recordIndex, LStream& outStream)
{
	vector<string> fields;
	fDatabase->GetRecord(recordIndex, fields);
	WriteRecord(&outStream, fields, fTranslateText);
	return 0;
}
