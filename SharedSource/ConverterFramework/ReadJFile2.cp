#include "JFile.h"
#include "ReadJFile1.h"
#include "ReadJFile2.h"
#include "charset.h"
#include "ConverterErrors.h"
#include "CDatabase.h"
#include "CDatabaseInfo.h"

CReadJFile2::CReadJFile2(const FSSpec& sourceFile, Boolean translateText)
	: CReadPDB(sourceFile, translateText)
{
}

CReadJFile2::~CReadJFile2()
{
}

void
CReadJFile2::ValidateHeader(const PilotDatabaseHeader &header)
{
	if (header.wVersion != 0 && header.wVersion != 1)
		Throw_(kWrongVersion);
}

void
CReadJFile2::HandleApplicationInfo(const Ptr applicationInfo, UInt32)
{
	JFile2AppInfoType *appInfoPtr = (JFile2AppInfoType*)applicationInfo;

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
CReadJFile2::HandleRecord(const PilotRecordEntry& entry, const Ptr recordData, UInt32 recordLength)
{
	if (!(entry.uniqueID & kDeletedFlag)) {
		int fieldCount = fDatabase.FieldCount();
		LDataStream inStream(recordData, recordLength);
		vector<string> fields;
		CReadJFile1::ReadRecord(&inStream, fields, fieldCount, fTranslateText);
		fDatabase.AppendFields(fields);
	}
}

Boolean
CReadJFile2::IsVersion2JFile(const FSSpec& inMacFSSpec)
{
	try {
		LFileStream file(inMacFSSpec);
		
		file.OpenDataFork(fsRdPerm);
		PilotDatabaseHeader header;
		file.ReadBlock(&header, 78);
		file.SetMarker(header.ofsAttributes, streamFrom_Start);
		JFile2AppInfoType appInfo;

		file.ReadBlock(&appInfo, sizeof(appInfo) - 4);	
		
		return (appInfo.version == CURRENT_VERSION);
	} catch (...) {
	}
	
	return false;
}

void
CReadJFile2::ExtractFromAppInfo(const void* inAppInfoPtr, bool translateText, const CFieldIDList& fieldIDs, CDatabaseInfo& info)
{
	JFile2AppInfoType *appInfoPtr = (JFile2AppInfoType*)inAppInfoPtr;

	 for (int fieldIndex = 0; fieldIndex < appInfoPtr->numFields; fieldIndex += 1) {
	 	string fieldName(appInfoPtr->fieldNames[fieldIndex]);
	 	FMAE::FieldID fieldID = fieldIDs[fieldIndex];
	 	if (translateText)
			ConvertToDesktopText(fieldName);
	 	info.fFieldNames[fieldID] = fieldName;
	 	info.fFieldTypes[fieldID] = appInfoPtr->fieldTypes[fieldIndex];
	 	info.fColumnWidths[fieldID] = appInfoPtr->showDBColumnWidths[fieldIndex];
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
	info.fPassword = string(appInfoPtr->password);
}

