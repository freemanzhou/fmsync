#include "CallMemberFunction.h"
#include "WriteJFile5.h"
#include "CDatabase.h"
#include "charset.h"
#include "Utilities.h"
#include "ConverterErrors.h"
#include "CDatabaseInfo.h"
#include "DebugOutput.h"
#include "JFile5.h"

#include <numeric>

using FMAE::FieldID;

namespace JFile5 {

vector<string> SplitRepeatingFields(const string& v, int repeatCount)
{
	static string groupSeparatorString(1, groupSeparator);
	vector<string> res(SplitString(v, groupSeparatorString));
	if (res.size() < repeatCount) {
		res.insert(res.end(), repeatCount - res.size(), "");
	}
	return res;
}

string JoinRepeatingFields(const vector<string>& v, int repeatCount)
{
	static string groupSeparatorString(1, groupSeparator);
	vector<string> toJoin(v);
	if (toJoin.size() < repeatCount) {
		toJoin.insert(toJoin.end(), repeatCount - toJoin.size(), "");
	}
	return JoinString(toJoin, groupSeparatorString);
}


CWriteJFile5::CWriteJFile5(CDataSource* sourceData, Boolean translateText)
	: CWritePDB(sourceData, translateText, kType, kCreator)
{
}

CWriteJFile5::~CWriteJFile5()
{
}

const int kMaxPerField = 75;

#ifdef CONDUIT
#define LOCK_FIELDS_IN_JFILE_PRO 1
#endif

struct JFile5FieldInfo {
	JFile5FieldInfo(const CDatabaseInfo& info, const FieldID& fieldID, int nameIndex, bool translateText);
	
	static string MakeFieldName(const CDatabaseInfo& info, const FieldID& fieldID, int nameIndex, bool translateText);
	static int MakeFieldType(const CDatabaseInfo& info, const FieldID& fieldID);
	static int MakeFieldColumnWidth(const CDatabaseInfo& info, const FieldID& fieldID);
	static int MakeFieldExtraData(const CDatabaseInfo& info, const FieldID& fieldID);
	static int MakeFieldExtraData2(const CDatabaseInfo& info, const FieldID& fieldID);
	
	string fFieldName;
	short fFieldType;
	short fShowDBColumnWidths;
	long fFieldExtraData;
	long fFieldExtraData2;
};

string JFile5FieldInfo::MakeFieldName(const CDatabaseInfo& info, const FieldID& fieldID, int nameIndex, bool translateText)
{
	string fieldName;
	info.FieldName(fieldID, fieldName);
	fieldName = CWriteJFile5::MakeFieldName(fieldName, nameIndex);
 	if (translateText)
		ConvertToPilotText(fieldName);

	return fieldName;
}

int JFile5FieldInfo::MakeFieldType(const CDatabaseInfo& info, const FieldID& fieldID)
{
	int fieldType;
	info.FieldType(fieldID, fieldType);
	int access;
	info.FieldAccess(fieldID, access);
	if (access) {
		fieldType |= FLDOPT_PROTECTED;
	}
	return fieldType;
}

int JFile5FieldInfo::MakeFieldColumnWidth(const CDatabaseInfo& info, const FieldID& fieldID)
{
	int cw = kMaxPerField;
	info.ColumnWidth(fieldID, cw);
	return cw;
}

int JFile5FieldInfo::MakeFieldExtraData(const CDatabaseInfo& info, const FieldID& fieldID)
{
	int extra = 0;
	info.FieldExtraData(fieldID, extra);
	return extra;
}

int JFile5FieldInfo::MakeFieldExtraData2(const CDatabaseInfo& info, const FieldID& fieldID)
{
	int extra = 0;
	info.FieldExtraData2(fieldID, extra);
	return extra;
}

JFile5FieldInfo::JFile5FieldInfo(const CDatabaseInfo& info, const FieldID& fieldID, int nameIndex, bool translateText)
	: fFieldName(MakeFieldName(info, fieldID, nameIndex, translateText)),
	fFieldType(MakeFieldType(info, fieldID)),
	fShowDBColumnWidths(MakeFieldColumnWidth(info, fieldID)),
	fFieldExtraData(MakeFieldExtraData(info, fieldID)),
	fFieldExtraData2(MakeFieldExtraData2(info, fieldID))
{
}


class MakeFields {
public:
	MakeFields(const CFieldIDList& fieldIDs, const CDatabaseInfo& info, const vector<int>& repeats, Boolean translateText);
	
	vector<JFile5FieldInfo> MakeFieldInfo();

private:	
	void HandleOneFieldID(const FieldID&);
	void HandleSingleField(const FieldID&);
	void HandleRepeatingField(const FieldID& fieldID, int repeatCount);
	
	
	CFieldIDList fFieldIDs;
	CDatabaseInfo fInfo;
	map<FieldID, int> fRepeats;
	bool fTranslateText;
	vector<JFile5FieldInfo> fConvertedInfo;
};

MakeFields::MakeFields(const CFieldIDList& fieldIDs, const CDatabaseInfo& info, const vector<int>& repeats, Boolean translateText)
	: fFieldIDs(fieldIDs), fInfo(info), fRepeats(MakeMap(fieldIDs, repeats)), fTranslateText(translateText)
{
}

void MakeFields::HandleSingleField(const FieldID& fieldID)
{
	JFile5FieldInfo theFieldInfo(fInfo, fieldID, 0, fTranslateText);
	fConvertedInfo.push_back(theFieldInfo);
}

void MakeFields::HandleRepeatingField(const FieldID& fieldID, int repeatCount)
{
	for(int i = 0; i < repeatCount; ++i) {
		JFile5FieldInfo theFieldInfo(fInfo, fieldID, i+1, fTranslateText);
		fConvertedInfo.push_back(theFieldInfo);
	}
}

void MakeFields::HandleOneFieldID(const FieldID& fieldID)
{
	int repeatCount = fRepeats[fieldID];
	if (repeatCount == 1)
		HandleSingleField(fieldID);
	else
		HandleRepeatingField(fieldID, repeatCount);
}

vector<JFile5FieldInfo> MakeFields::MakeFieldInfo()
{
	fConvertedInfo.clear();
	CallMemberFunction<MakeFields,FieldID> f(this, &MakeFields::HandleOneFieldID);
	for_each(fFieldIDs.begin(), fFieldIDs.end(), f);
	return fConvertedInfo; 
}

static void CopyIntoAppInfo(const JFile5FieldInfo& finfo, JFileAppInfoType& appInfo, int i)
{
	strncpy(appInfo.fieldNames[i], finfo.fFieldName.c_str(), MAX_FIELD_NAME_LENGTH);
	appInfo.fieldTypes[i] = finfo.fFieldType;
	appInfo.showDBColumnWidths[i] = finfo.fShowDBColumnWidths;
	appInfo.fieldExtraData[i] = finfo.fFieldExtraData;
	appInfo.fieldExtraData2[i] = finfo.fFieldExtraData2;
}

void
CWriteJFile5::WriteAppInfoBlock(LStream* targetStream, const CFieldIDList& fieldIDs, 
const CDatabaseInfo& info, const vector<int>& repeats, Boolean translateText, Boolean needsSortOrCalc)
{
	JFileAppInfoType appInfo = {};
	
	MakeFields mf(fieldIDs, info, repeats, translateText);
	vector<JFile5FieldInfo> theFields(mf.MakeFieldInfo());
	
	int fieldCount = theFields.size();
	if (fieldCount > MAX_FIELDS)
		Throw_(kTooManyFields);

	int i;
	for (i = 0; i < fieldCount; i += 1) {
		CopyIntoAppInfo(theFields.at(i), appInfo, i);
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
	appInfo.flags_defunct = info.fFlags;
	if (needsSortOrCalc)
		appInfo.flags_defunct |= INFOFLAGS_SYNC_ALTERED_DB_DEFUNCT;

	appInfo.firstColumnToShow = info.fFirstColumnToShow;
	strncpy(appInfo.findString, info.fFindString.c_str(), MAX_FIND_STRING);
	strncpy(appInfo.filterString, info.fFilterString.c_str(), MAX_FIND_STRING);
	appInfo.marker = CURRENT_MARKER;

	targetStream->WriteBlock(&appInfo, sizeof(JFileAppInfoType));
	LHandleStream hStream;
	CWriteJFile5::WritePopupChoices(&hStream, fieldCount, fieldIDs, info, translateText);
	CWriteJFile5::WriteCalcExtra1(&hStream, fieldCount, fieldIDs, info, translateText);
	CWriteJFile5::WriteCalcExtra2(&hStream, fieldCount, fieldIDs, info, translateText);
	CWriteJFile5::WriteOtherExtra(&hStream, info);
	char endNull = 0;
	hStream.WriteBlock(&endNull, sizeof(endNull));
	Handle h = hStream.GetDataHandle();
	::HLock(h);
	targetStream->WriteBlock(*h, GetHandleSize(h));
	const char kEndTag[] = "EndJFileData";
	targetStream->WriteBlock(&kEndTag[0], strlen(kEndTag)+1);
}


static vector<string> MakeExpandedFields(const vector<string>& fields, const vector<int>& repeats)
{
	vector<string> expandedFields;
	ThrowIf_(fields.size() != repeats.size());
	for(int i = 0; i < fields.size(); ++i) {
		if (repeats[i] == 1)
			expandedFields.push_back(fields[i]);
		else {
			vector<string> parts(SplitString(fields[i], string(1, groupSeparator)));
			if (parts.size() < repeats[i])
				parts.resize(repeats[i]);
			copy(parts.begin(), parts.end(), back_inserter(expandedFields));
		}
	}
	return expandedFields;
}

void CWriteJFile5::WriteRecord(LStream* targetStream, const vector<string>& tf, Boolean translateText, vector<int>& repeats)
{
	vector<string> expandedFields(MakeExpandedFields(tf, repeats));
	int fieldCount = expandedFields.size();
	vector<short> fieldSizes(fieldCount);
	for(int j = 0; j< fieldCount; j++) {
		int fieldSize = expandedFields[j].length() + 1;
		fieldSizes[j] = fieldSize;
	 	if (fieldSize > MAX_DATA_LENGTH)
	 		Throw_(kFieldDataTooLong);
 	}
 	targetStream->WriteBlock(&fieldSizes[0], fieldCount*sizeof(fieldSizes[0]));

	for(int j = 0; j< fieldCount; j++) {
		string fieldData = expandedFields[j];
	 	if (translateText)
			ConvertToPilotText(fieldData);
		ConvertToPilotReturns(fieldData);
		int fieldSize = fieldData.length() + 1;
		const char *p = fieldData.c_str();
 	 	targetStream->WriteBlock(p, fieldSize);
 	}
}

void
CWriteJFile5::WritePopupChoices(LStream* targetStream, int fieldCount, const CFieldIDList& fieldIDs, const CDatabaseInfo& info, bool translateText)
{
	if (info.fPopupValues.size() != 0) {
		for (int i = 0; i < fieldCount; i += 1) {
			FieldID fieldID = fieldIDs[i];
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
						popupValueString.append(1, kJFileBlankSignal);
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
	}
}

void
CWriteJFile5::WriteCalcExtra(LStream* targetStream, int fieldCount, const CFieldIDList& fieldIDs, 
	const CDatabaseInfo& info, bool translateText, const map<FMAE::FieldID,string>& theMap, const string& calcLabelBase)
{
	if (theMap.size() != 0) {
		for (int i = 0; i < fieldCount; i += 1) {
			FieldID fieldID = fieldIDs[i];
			string calcValue;
			map<FMAE::FieldID,string>::const_iterator f = theMap.find(fieldID); 
			if (f != theMap.end())
				calcValue = f->second;
			if (calcValue.length() == 0)
				continue;
		 	if (translateText)
				ConvertToPilotText(calcValue);
			string calcLabel(calcLabelBase);
			char c = IndexToChar(i);
			calcLabel += c;
			targetStream->WriteBlock(calcLabel.c_str(), calcLabel.length()+1);
			targetStream->WriteBlock(calcValue.c_str(), calcValue.length()+1);
		}
	}
}

void CWriteJFile5::WriteCalcExtra1(LStream* targetStream, int fieldCount, const CFieldIDList& fieldIDs, const CDatabaseInfo& info, bool translateText)
{
	WriteCalcExtra(targetStream, fieldCount, fieldIDs, info, translateText, info.fFieldCalcValue1, CALC_VALUE1_SEPERATOR_STRING);
}

void CWriteJFile5::WriteCalcExtra2(LStream* targetStream, int fieldCount, const CFieldIDList& fieldIDs, const CDatabaseInfo& info, bool translateText)
{
	WriteCalcExtra(targetStream, fieldCount, fieldIDs, info, translateText, info.fFieldCalcValue2, CALC_VALUE2_SEPERATOR_STRING);
}

#if 0
UInt8
CWriteJFile5::WriteRecord(UInt32 recordIndex, LStream& outStream)
{
	vector<string> fields;
	fDatabase->GetRecord(recordIndex, fields);
	WriteRecord(&outStream, fields, fTranslateText);
	return 0;
}
#endif

void
CWriteJFile5::WriteApplicationInfo(LStream& outStream)
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
CWriteJFile5::WriteAppInfoBlock(LStream* targetStream, vector<string>& fieldNames, 
				const vector<int>& fieldWidths, const vector<int>& fieldTypes, const vector<string_vector>& popups, Boolean translateText)
{
	JFileAppInfoType appInfo = {};
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
	appInfo.marker = CURRENT_MARKER;

	targetStream->WriteBlock(&appInfo, sizeof(JFileAppInfoType));
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
						popupValueString.append(1, kJFileBlankSignal);
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
CWriteJFile5::IndexToChar(int i)
{
	if (i > 25)
		return 'A' + i - 26;
		
	return 'a' + i;
}

UInt32 CWriteJFile5::ExtraDataForCalculatedFields(UInt32 f1, UInt32 f2, UInt32 oper)
{
	if (f1 == 0xffffffff)
		f1 = 0;
	if (f2 == 0xffffffff)
		f2 = 0;
	return (f1 << 24| (f2 << 16) | oper);
}

struct WriteStringOp {
	LStream* fTargetStream;
	WriteStringOp(LStream* targetStream) : fTargetStream(targetStream) {}
	
	void operator()(const string& v) {
		fTargetStream->WriteBlock(v.c_str(), v.length() + 1);
	}
};

struct WriteExtrasOp {
	LStream* fTargetStream;
	WriteExtrasOp(LStream* targetStream) : fTargetStream(targetStream) {}
	
	void operator()(const vector<string>& v) {
		for_each(v.begin(), v.end(), WriteStringOp(fTargetStream));
	}
};

void CWriteJFile5::WriteOtherExtra(LStream* targetStream, const CDatabaseInfo& info)
{
	for_each(info.fJFileExtraValues.begin(), info.fJFileExtraValues.end(), WriteExtrasOp(targetStream));
}

string CWriteJFile5::MakeFieldName(const string& baseName, int repeatIndex)
{
	if (repeatIndex < 2)
		return baseName.substr(0, MAX_FIELD_NAME_LENGTH);
	
	return AsString(repeatIndex) + ":" + baseName.substr(0, MAX_FIELD_NAME_LENGTH - 4);
}

}