#include "WriteJFilePro.h"
#include "WriteJFile2.h"
#include "CDatabase.h"
#include "charset.h"
#include "Utilities.h"
#include "ConverterErrors.h"
#include "CDatabaseInfo.h"
#include "DebugOutput.h"
#include "JFilePro.h"

CWriteJFilePro::CWriteJFilePro(CDataSource* sourceData, Boolean translateText)
	: CWritePDB(sourceData, translateText, kType, kCreator)
{
}

CWriteJFilePro::~CWriteJFilePro()
{
}

const int kMaxPerField = 75;

#ifdef CONDUIT
#define LOCK_FIELDS_IN_JFILE_PRO 1
#endif

void
CWriteJFilePro::WriteAppInfoBlock(LStream* targetStream, const CFieldIDList& fieldIDs, 
const CDatabaseInfo& info, Boolean translateText)
{
	JFileProAppInfoType appInfo = {};
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
		int fieldType;
		info.FieldType(fieldID, fieldType);
		appInfo.fieldTypes[i] = fieldType;
		#if LOCK_FIELDS_IN_JFILE_PRO
		int access;
		info.FieldAccess(fieldID, access);
		if (access) {
			appInfo.fieldTypes[i] |= FLDOPT_PROTECTED;
		}
		#endif

		int columnWidth;
		if (info.ColumnWidth(fieldID, columnWidth))
			appInfo.showDBColumnWidths[i] = columnWidth;
		else
			appInfo.showDBColumnWidths[i] = kMaxPerField;

		int extra;
		if (info.FieldExtraData(fieldID, extra))
			appInfo.fieldExtraData[i] = extra;
		else
			appInfo.fieldExtraData[i] = 0;

		if (info.FieldExtraData2(fieldID, extra))
			appInfo.fieldExtraData2[i] = extra;
		else
			appInfo.fieldExtraData2[i] = 0;
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
	strncpy(appInfo.findString, info.fFindString.c_str(), MAX_FIND_STRING);
	strncpy(appInfo.filterString, info.fFilterString.c_str(), MAX_FIND_STRING);
	appInfo.version = CURRENT_VERSION;

	targetStream->WriteBlock(&appInfo, sizeof(JFileProAppInfoType));
	LHandleStream hStream;
	CWriteJFilePro::WritePopupChoices(&hStream, fieldCount, fieldIDs, info, translateText);
	Handle h = hStream.GetDataHandle();
	::HLock(h);
	targetStream->WriteBlock(*h, GetHandleSize(h));
	const char kEndTag[] = "EndJFileData";
	targetStream->WriteBlock(&kEndTag[0], strlen(kEndTag)+1);
}

void
CWriteJFilePro::WriteRecord(LStream* targetStream, const vector<string>& fields, Boolean translateText)
{
	int fieldCount = fields.size();
	vector<short> fieldSizes(fieldCount);
	for(int j = 0; j< fieldCount; j++) {
		int fieldSize = fields[j].length() + 1;
		fieldSizes[j] = fieldSize;
	 	if (fieldSize > MAX_DATA_LENGTH)
	 		Throw_(kFieldDataTooLong);
 	}
 	targetStream->WriteBlock(&fieldSizes[0], fieldCount*sizeof(fieldSizes[0]));

	for(int j = 0; j< fieldCount; j++) {
		string fieldData = fields[j];
	 	if (translateText)
			ConvertToPilotText(fieldData);
		ConvertToPilotReturns(fieldData);
		int fieldSize = fieldData.length() + 1;
		const char *p = fieldData.c_str();
 	 	targetStream->WriteBlock(p, fieldSize);
 	}
}

void
CWriteJFilePro::WritePopupChoices(LStream* targetStream, int fieldCount, const CFieldIDList& fieldIDs, const CDatabaseInfo& info, bool translateText)
{
	if (info.fPopupValues.size() == 0) {
		long nullPopups = 0;
		targetStream->WriteBlock(&nullPopups, sizeof(nullPopups));
	} else {
		for (int i = 0; i < fieldCount; i += 1) {
			FMAE::FieldID fieldID = fieldIDs[i];
			int popupCount = 0;
			string_vector popupChoices;
			info.PopupValues(fieldID, popupChoices);
			int popupSize = 0;
			vector<string>::const_iterator pIter = popupChoices.begin();
			if (pIter != popupChoices.end() && popupCount < MAX_TOTAL_POPUP_PER_LIST_LENGTH) {
				string popupLabel("popup");
				char c = IndexToChar(i);
				popupLabel += c;
				targetStream->WriteBlock(popupLabel.c_str(), popupLabel.length()+1);
				while (pIter != popupChoices.end()) {
					string popupValueString(*pIter);
					if (popupValueString.length() == 0) {
						popupValueString.append(1, kJFileProBlankSignal);
					}
				 	if (translateText)
						ConvertToPilotText(popupValueString);
					int popupItemLength = popupValueString.length() + 1;
					if (popupItemLength > MAX_POPUP_ITEM_LENGTH)
						popupValueString = popupValueString.substr(0, MAX_POPUP_ITEM_LENGTH);
					popupSize += popupItemLength;
					targetStream->WriteBlock(popupValueString.c_str(), popupItemLength);
					++pIter;
					++popupCount;
				}
			}
			if (popupSize > MAX_TOTAL_POPUP_PER_LIST_LENGTH)
				Throw_(kPopupChoicesTooLong);
		}
		short endPopups = 0;
		targetStream->WriteBlock(&endPopups, sizeof(endPopups));
	}
}

UInt8
CWriteJFilePro::WriteRecord(UInt32 recordIndex, LStream& outStream)
{
	vector<string> fields;
	fDatabase->GetRecord(recordIndex, fields);
	WriteRecord(&outStream, fields, fTranslateText);
	return 0;
}

void
CWriteJFilePro::WriteApplicationInfo(LStream& outStream)
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

void
CWriteJFilePro::WriteAppInfoBlock(LStream* targetStream, vector<string>& fieldNames, 
				const vector<int>& fieldWidths, const vector<int>& fieldTypes, const vector<string_vector>& popups, Boolean translateText)
{
	JFileProAppInfoType appInfo = {};
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

	targetStream->WriteBlock(&appInfo, sizeof(JFileProAppInfoType));
	if (popups.size() == 0) {
		long nullPopups = 0;
		targetStream->WriteBlock(&nullPopups, sizeof(nullPopups));
	} else {
		int fieldIndex = 0;
		vector<string_vector>::const_iterator vIter = popups.begin();
		while (vIter != popups.end()) {
			vector<string>::const_iterator pIter = vIter->begin();
			if (pIter != vIter->end()) {
				string popupLabel("popup");
				char c = IndexToChar(fieldIndex);
				popupLabel += c;
				targetStream->WriteBlock(popupLabel.c_str(), popupLabel.length()+1);
				while (pIter != vIter->end()) {
					string popupValueString(*pIter);
					if (popupValueString.length() == 0) {
						popupValueString.append(1, kJFileProBlankSignal);
					}
				 	if (translateText)
						ConvertToPilotText(popupValueString);
					targetStream->WriteBlock(popupValueString.c_str(), popupValueString.length()+1);
					++pIter;
				}
			}
			fieldIndex += 1;
			++vIter;
		}
		short endPopups = 0;
		targetStream->WriteBlock(&endPopups, sizeof(endPopups));
	}
}

char
CWriteJFilePro::IndexToChar(int i)
{
	if (i > 25)
		return 'A' + i - 26;
		
	return 'a' + i;
}
