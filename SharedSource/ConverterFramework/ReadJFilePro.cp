#include "JFilePro.h"
#include "ReadJFilePro.h"
#include "charset.h"
#include "ConverterErrors.h"
#include "CDatabaseInfo.h"

CReadJFilePro::CReadJFilePro(const FSSpec& sourceFile, Boolean translateText)
	: CReadPDB(sourceFile, translateText)
{
}

CReadJFilePro::~CReadJFilePro()
{
}

void
CReadJFilePro::ReadRecord(LStream *stream, vector<string>& fields, int fieldCount, Boolean translateText)
{
	vector<short> fieldLengths(fieldCount);
	stream->ReadBlock(&fieldLengths[0], fieldCount*sizeof(fieldLengths[0]));
 	for(int j=0; j < fieldCount; j++)
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
}

void
CReadJFilePro::ExtractFromAppInfo(const void* inAppInfoPtr, bool translateText, const CFieldIDList& fieldIDs, CDatabaseInfo& info)
{
	DebugOutput::Output( "CReadJFilePro::ExtractFromAppInfo()" );
	JFileProAppInfoType *appInfoPtr = (JFileProAppInfoType*)inAppInfoPtr;

	 for (int fieldIndex = 0; fieldIndex < appInfoPtr->numFields; fieldIndex += 1) {
	 	string fieldName(appInfoPtr->fieldNames[fieldIndex]);
	 	FMAE::FieldID fieldID = fieldIDs[fieldIndex];
	 	if (translateText)
			ConvertToDesktopText(fieldName);
	 	info.fFieldNames[fieldID] = fieldName;
	 	info.fFieldTypes[fieldID] =  appInfoPtr->fieldTypes[fieldIndex] & FLDOPT_CLEAR;
	 	info.fFieldAccess[fieldID] =  ((appInfoPtr->fieldTypes[fieldIndex] & FLDOPT_PROTECTED) != 0);
	 	info.fColumnWidths[fieldID] = appInfoPtr->showDBColumnWidths[fieldIndex];
	 	info.fFieldExtraData[fieldID] =  appInfoPtr->fieldExtraData[fieldIndex];
	 	info.fFieldExtraData2[fieldID] =  appInfoPtr->fieldExtraData2[fieldIndex];
	 }

	info.fFieldCount = appInfoPtr->numFields;
	info.fShowDataWidth = appInfoPtr->showDataWidth;
	info.fSortFields[0] = appInfoPtr->sort1Field;
	info.fSortFields[1] = appInfoPtr->sort2Field;
	info.fSortFields[2] = appInfoPtr->sort3Field;
	info.fFindField = appInfoPtr->findField;
	info.fFilterField = appInfoPtr->filterField;
	info.fFindString = string(appInfoPtr->findString);
	info.fFilterString = string(appInfoPtr->filterString);
	info.fFlags = appInfoPtr->flags;
	info.fFirstColumnToShow = appInfoPtr->firstColumnToShow;
	
	ExtractPopups(appInfoPtr, translateText, fieldIDs, info);
}

void
CReadJFilePro::ExtractPopups(const void* inAppInfoPtr, bool translateText, const CFieldIDList& fieldIDs, CDatabaseInfo& info)
{
	DebugOutput::Output( "CReadJFilePro::ExtractPopups()" );
	const char *p = (const char *)(inAppInfoPtr) + sizeof(JFileProAppInfoType);
	
	const char *kLabel = "popup";
	const int kLabelSize = 5;
	
	for(;;) {
		if (*p == 0)
			break;
		else {
			if (strncmp(p, kLabel, kLabelSize) == 0) {
				p += kLabelSize;
				int index = CharToIndex(*p);
				FMAE::FieldID fieldID(fieldIDs[index]);
				p += 2;
				vector<string> v;
				while (*p && strncmp(p, kLabel, kLabelSize) != 0) {
					string item(p);
				 	if (translateText)
						ConvertToDesktopText(item);
					if (item.length() == 1 && item[0] == kJFileProBlankSignal)
						item.clear();
					v.push_back(item);
					p += strlen(p);
					p += 1;
				}
				info.fPopupValues[fieldID] = v;
			} else {
				p += strlen(p);
				p += 1;
			}
		}
	}
}

void
CReadJFilePro::ValidateHeader(const PilotDatabaseHeader &header)
{
	if (header.wVersion != 0 && header.wVersion != 1)
		Throw_(kWrongVersion);
}

void
CReadJFilePro::HandleApplicationInfo(const Ptr applicationInfo, UInt32)
{
	JFileProAppInfoType *appInfoPtr = (JFileProAppInfoType*)applicationInfo;

	 for (int fieldIndex = 0; fieldIndex < appInfoPtr->numFields; fieldIndex += 1) {
	 	string fieldName(appInfoPtr->fieldNames[fieldIndex]);
	 	if (fTranslateText)
			ConvertToDesktopText(fieldName);
	 	fDatabase.AppendFieldName(fieldName);
	 	fDatabase.AppendFieldType(FLDTYPE_STRING);
	 }
	 fDatabase.SetFieldNamesKnown(true);
}

void
CReadJFilePro::HandleRecord(const PilotRecordEntry& entry, const Ptr recordData, UInt32 recordLength)
{
	if (!(entry.uniqueID & kDeletedFlag)) {
		int fieldCount = fDatabase.FieldCount();
		LDataStream inStream(recordData, recordLength);
		vector<string> fields;
		CReadJFilePro::ReadRecord(&inStream, fields, fieldCount, fTranslateText);
		fDatabase.AppendFields(fields);
	}
}

Boolean
CReadJFilePro::IsVersionProJFile(const FSSpec& inMacFSSpec)
{
	try {
		LFileStream file(inMacFSSpec);
		
		file.OpenDataFork(fsRdPerm);
		PilotDatabaseHeader header;
		file.ReadBlock(&header, 78);
		file.SetMarker(header.ofsAttributes, streamFrom_Start);
		JFileProAppInfoType appInfo;

		file.ReadBlock(&appInfo, sizeof(appInfo) - 4);	
		
		return (appInfo.version == CURRENT_VERSION);
	} catch (...) {
	}
	
	return false;
}

int
CReadJFilePro::CharToIndex(unsigned char c)
{
	if (c >= 'a')
		return c - 'a';
	
	return c - 'A' + 26;
}
