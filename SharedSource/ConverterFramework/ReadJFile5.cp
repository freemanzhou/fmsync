#include "JFile5.h"
#include "ReadJFile5.h"
#include "charset.h"
#include "ConverterErrors.h"
#include "CDatabaseInfo.h"

#include <numeric>

using namespace JFile5;

CReadJFile5::CReadJFile5(const FSSpec& sourceFile, Boolean translateText)
	: CReadPDB(sourceFile, translateText)
{
}

CReadJFile5::~CReadJFile5()
{
}

static vector<string> CollapseFields(const vector<string>& fields, const vector<int>& repeats)
{
	vector<string> newFields;
	
	vector<string>::const_iterator f = fields.begin();
	vector<int>::const_iterator r = repeats.begin();
	
	while(f != fields.end()) {
		string fieldData = *f++;
		int repeatCount = *r++;
	
		while(f != fields.end() && repeatCount > 1) {
			fieldData += string(1, groupSeparator);
			fieldData += *f++;
			repeatCount--;
		}
		newFields.push_back(fieldData);
	}
	
	return newFields;
}

void
CReadJFile5::ReadRecord(LStream *stream, vector<string>& fields, int fc, Boolean translateText, const vector<int>& repeats)
{	
	ThrowIf_(repeats.size() < fc);
	int jfileFieldCount = accumulate(repeats.begin(), repeats.begin() + fc, 0);
	vector<short> fieldLengths(jfileFieldCount);
	stream->ReadBlock(&fieldLengths[0], jfileFieldCount*sizeof(fieldLengths[0]));
 	for(int j=0; j < jfileFieldCount; j++)
	{
		int fieldLength = fieldLengths[j];
		vector<char> fieldData(fieldLength);
		stream->ReadBlock(&fieldData[0], fieldLength);
		string fieldDataString(string(&fieldData[0], fieldLength-1));
 		if (translateText)
			ConvertToDesktopText(fieldDataString);
		ConvertToDesktopReturns(fieldDataString);
		fields.push_back(fieldDataString);
	}
	fields = CollapseFields(fields, repeats);
}

void
CReadJFile5::ExtractFromAppInfo(const void* inAppInfoPtr, int dataLength, bool translateText, const CFieldIDList& fieldIDs, const vector<int>& repeats, CDatabaseInfo& info)
{
	DebugOutput::Output( "CReadJFile5::ExtractFromAppInfo()" );
	JFileAppInfoType *appInfoPtr = (JFileAppInfoType*)inAppInfoPtr;
	const void* limitP = (const char*)inAppInfoPtr + dataLength;
	int jfileFieldIndex = 0;
	bool usePassedInFieldCount = false;
	
	 for (int index = 0; index < fieldIDs.size(); index += 1) {
	 	string fieldName(appInfoPtr->fieldNames[jfileFieldIndex]);
	 	FMAE::FieldID fieldID = fieldIDs[index];
	 	if (translateText)
			ConvertToDesktopText(fieldName);
	 	info.fFieldNames[fieldID] = fieldName;
	 	info.fFieldTypes[fieldID] =  appInfoPtr->fieldTypes[jfileFieldIndex] & FLDOPT_CLEAR;
	 	info.fFieldAccess[fieldID] =  ((appInfoPtr->fieldTypes[jfileFieldIndex] & FLDOPT_PROTECTED) != 0);
	 	info.fColumnWidths[fieldID] = appInfoPtr->showDBColumnWidths[jfileFieldIndex];
	 	info.fFieldExtraData[fieldID] =  appInfoPtr->fieldExtraData[jfileFieldIndex];
	 	info.fFieldExtraData2[fieldID] =  appInfoPtr->fieldExtraData2[jfileFieldIndex];
	 	info.fFieldCalcValue1[fieldID] =  "";
	 	info.fFieldCalcValue2[fieldID] =  "";
	 	if (repeats.at(index) > 1)
	 		usePassedInFieldCount = true;
	 	jfileFieldIndex += repeats.at(index);
	 }

	if (usePassedInFieldCount)
		info.fFieldCount = fieldIDs.size();
	else
		info.fFieldCount = appInfoPtr->numFields;
		
	info.fShowDataWidth = appInfoPtr->showDataWidth;
	info.fSortFields[0] = appInfoPtr->sort1Field;
	info.fSortFields[1] = appInfoPtr->sort2Field;
	info.fSortFields[2] = appInfoPtr->sort3Field;
	info.fFindField = appInfoPtr->findField;
	info.fFilterField = appInfoPtr->filterField;
	info.fFindString = string(appInfoPtr->findString);
	info.fFilterString = string(appInfoPtr->filterString);
	info.fFlags = 0;
	info.fFirstColumnToShow = appInfoPtr->firstColumnToShow;
	
	ExtractExtraData(appInfoPtr, limitP, translateText, fieldIDs, info);
}


static const char *kExtraDataSeparators[] = {
	DEFAULT_SEPERATOR_STRING,
	POPUP_SEPERATOR_STRING,
	FILTER_SEPERATOR_STRING,
	SORT_SEPERATOR_STRING,
	CALC_VALUE1_SEPERATOR_STRING,
	CALC_VALUE2_SEPERATOR_STRING
};

const int kExtraDataSeparatorsCount = sizeof(kExtraDataSeparators)/sizeof(kExtraDataSeparators[0]);
const int kSeparatorLength = 5;

enum {kDefaultsKind, kPopupsKind, kFiltersKind, kSortsKind, kCalcValue1Kind, kCalcValue2Kind};

static int kExtraDataKinds[kExtraDataSeparatorsCount] = {
	kDefaultsKind,
	kPopupsKind,
	kFiltersKind,
	kSortsKind,
	kCalcValue1Kind,
	kCalcValue2Kind
};

static int LabelKind(const char *p)
{
	for (int i = 0; i < kExtraDataSeparatorsCount; ++i) {
		if (strncmp(p, kExtraDataSeparators[i], kSeparatorLength) == 0)
			return kExtraDataKinds[i];
	}
	return -1;
}

static const char* GetIndexFromSeparator(const char* p, int& index)
{
	p += kSeparatorLength;
	index = CReadJFile5::CharToIndex(*p);
	p += 2;
	return p;
}

const char* CReadJFile5::ExtractPopups(const char *p, const void* limitP, bool translateText, const CFieldIDList& fieldIDs, CDatabaseInfo& info)
{
	DebugOutput::Output( "CReadJFile5::ExtractPopups()" );
	int index;
	p = GetIndexFromSeparator(p, index);
	FMAE::FieldID fieldID(fieldIDs[index]);
	vector<string> v;
	while (*p && (LabelKind(p) == -1)) {
		string item(p);
		if (item != "-") {
		 	if (translateText)
				ConvertToDesktopText(item);
			if (item.length() == 1 && item[0] == kJFileProBlankSignal)
				item.clear();
			v.push_back(item);
		}
		p += strlen(p);
		p += 1;
	}
	info.fPopupValues[fieldID] = v;
	return p;
}

static const char* ExtractCalcExtra(const char *p, const void* limitP, map<FMAE::FieldID,string>& theMap, const CFieldIDList& fieldIDs, CDatabaseInfo& info)
{
	int index;
	p = GetIndexFromSeparator(p, index);
	FMAE::FieldID fieldID(fieldIDs[index]);
	string item(p);
	theMap[fieldID] = item;
	p += item.length();
	p += 1;
	return p;
}

static const char* ExtractCalc1Extra(const char *p, const void* limitP, bool translateText, const CFieldIDList& fieldIDs, CDatabaseInfo& info)
{
	return ExtractCalcExtra(p, limitP, info.fFieldCalcValue1, fieldIDs, info);
}

static const char* ExtractCalc2Extra(const char *p, const void* limitP, bool translateText, const CFieldIDList& fieldIDs, CDatabaseInfo& info)
{
	return ExtractCalcExtra(p, limitP, info.fFieldCalcValue2, fieldIDs, info);
}

static const char* ExtractDefaults(const char *p, bool translateText, const CFieldIDList& fieldIDs, CDatabaseInfo& info)
{
	p += strlen(p);
	p += 1;
	return p;
}

static const char* ExtractGenericExtraData(const char *p, const void* limitP, bool translateText, const CFieldIDList& fieldIDs, CDatabaseInfo& info)
{
	DebugOutput::Output( "ExtractGenericExtraData()" );
	vector<string> extraD;
	
	do {
		extraD.push_back(p);
		p += strlen(p);
		p += 1;
	} while (LabelKind(p) == -1 && p < limitP);
	
	info.fJFileExtraValues.push_back(extraD);

	return p;
}

static const char* SkipUnknownExtraData(const char *p)
{
	p += strlen(p);
	p += 1;
	return p;
}

void CReadJFile5::ExtractExtraData(const void* inAppInfoPtr, const void* limitP, bool translateText, const CFieldIDList& fieldIDs, CDatabaseInfo& info)
{
	DebugOutput::Output( "CReadJFile5::ExtractExtraData()" );
	const char *p = (const char *)(inAppInfoPtr) + sizeof(JFileAppInfoType);
	
	for(;;) {
		if (*p == 0 || p >= limitP)
			break;
		else {
			int labelKind = LabelKind(p);
			switch(labelKind) {
			case kPopupsKind:
				p = ExtractPopups(p, limitP, translateText, fieldIDs, info);
				break;
			case kDefaultsKind:
			case kFiltersKind:
			case kSortsKind:
				p = ExtractGenericExtraData(p, limitP, translateText, fieldIDs, info);
				break;
			case kCalcValue1Kind:
				p = ExtractCalc1Extra(p, limitP, translateText, fieldIDs, info);
				break;
			case kCalcValue2Kind:
				p = ExtractCalc2Extra(p, limitP, translateText, fieldIDs, info);
				break;
			default:
				p = SkipUnknownExtraData(p);
				break;
			}
		}
	}
}

void
CReadJFile5::ValidateHeader(const PilotDatabaseHeader &header)
{
	if (header.wVersion != 0 && header.wVersion != 1)
		Throw_(kWrongVersion);
}

void
CReadJFile5::HandleApplicationInfo(const Ptr applicationInfo, UInt32)
{
	JFileAppInfoType *appInfoPtr = (JFileAppInfoType*)applicationInfo;

	 for (int fieldIndex = 0; fieldIndex < appInfoPtr->numFields; fieldIndex += 1) {
	 	string fieldName(appInfoPtr->fieldNames[fieldIndex]);
	 	if (fTranslateText)
			ConvertToDesktopText(fieldName);
	 	fDatabase.AppendFieldName(fieldName);
	 	fDatabase.AppendFieldType(FLDTYPE_STRING);
	 }
	 fDatabase.SetFieldNamesKnown(true);
}

#if 0
void
CReadJFile5::HandleRecord(const PilotRecordEntry& entry, const Ptr recordData, UInt32 recordLength)
{
	if (!(entry.uniqueID & kDeletedFlag)) {
		int fieldCount = fDatabase.FieldCount();
		LDataStream inStream(recordData, recordLength);
		vector<string> fields;
		CReadJFile5::ReadRecord(&inStream, fields, fieldCount, fTranslateText);
		fDatabase.AppendFields(fields);
	}
}
#endif

Boolean
CReadJFile5::IsVersion5JFile(const FSSpec& inMacFSSpec)
{
	try {
		LFileStream file(inMacFSSpec);
		
		file.OpenDataFork(fsRdPerm);
		PilotDatabaseHeader header;
		file.ReadBlock(&header, 78);
		file.SetMarker(header.ofsAttributes, streamFrom_Start);
		JFileAppInfoType appInfo;

		file.ReadBlock(&appInfo, sizeof(appInfo) - 4);	
		
		return (appInfo.marker == CURRENT_MARKER);
	} catch (...) {
	}
	
	return false;
}

int
CReadJFile5::CharToIndex(unsigned char c)
{
	if (c >= 'a')
		return c - 'a';
	
	return c - 'A' + 26;
}
