#include "CFMFieldTyper.h"
#include "FMDatabase.h"
#include "Utilities.h"
#include "JFilePro.h"

CFMFieldTyper::CFMFieldTyper(FileMaker::DatabasePtr sourceDatabase)
	: fDatabase(sourceDatabase)
{
	SetupDefaultFieldMaps();
}
				
string
CFMFieldTyper::FindField(const FMAE::FieldID& fieldID)
{
	if (!fFieldCached[fieldID]) {
		const int kMaxRecordsToSearch = 32;
		int recordsAvailable = fAllRecordIDs.size();
		int recordCount = min(recordsAvailable, kMaxRecordsToSearch);
		
		
		if (recordCount == 0)
			return kEmptyString;

		for (int i = 0; i < recordCount && !fFieldCached[fieldID]; i++) {
			vector<string> record(fDatabase->GetRecord(fAllRecordIDs[i]));
			for (int j = 0; j < fFieldIDs.size(); j++) {
				FMAE::FieldID cachedFieldID(fFieldIDs[j]);
				if (record[j].length() > 0) {
					fCachedFields[cachedFieldID] = record[j];
				}
				fFieldCached[cachedFieldID] = true;
			}
		}
		fFieldCached[fieldID] = true;
	}
	return fCachedFields[fieldID];
}

void
CFMFieldTyper::SetupDefaultFieldTypes()
{
	vector<int> fmFieldTypes(fDatabase->GetFieldTypes());
	
	int fieldCount = fFieldIDs.size();
	for (int i = 0; i < fieldCount; i+=1) {
		FMAE::FieldID fieldID(fFieldIDs[i]);
		string fieldData;
		if (TypeDeterminationNeedsData(fmFieldTypes[i])) {
			fieldData = FindField(fieldID);
		}
		fJFileFieldTypes[fieldID] = 
			JFileFromFMType(fmFieldTypes[i], fieldData, fInfo.fPopupValues[fieldID].size() > 0);
	}
	
}

void
CFMFieldTyper::SetupDefaultFieldMaps()
{
	fAllRecordIDs = fDatabase->GetAllRecordIDs();
	fFieldIDs = fDatabase->GetFieldIDs();
	fInfo.fFieldNames = MakeMap(fFieldIDs, fDatabase->GetFieldNames());
	fInfo.fPopupValues = MakeMap(fFieldIDs, fDatabase->GetFieldChoices());
	fInfo.fRepeats = MakeMap(fFieldIDs, fDatabase->GetFieldRepeats());
	vector<int> fmFieldTypes(fDatabase->GetFieldTypes());
	vector<int> fmReadOnly(fDatabase->GetFieldReadOnly());
	
	SetupDefaultFieldTypes();

	int fieldCount = fFieldIDs.size();
	fInfo.fFieldCount = fieldCount;
	for (int i = 0; i < fieldCount; i+=1) {
		int extra, extra2;
		FMAE::FieldID fieldID(fFieldIDs[i]);
		fInfo.fColumnWidths[fieldID] = 80;
		fInfo.fFieldTypes[fieldID] = fJFileFieldTypes[fieldID];
		fInfo.fFieldAccess[fieldID] = fmReadOnly[i];
		ExtraFromType(fInfo.fFieldTypes[fieldID], extra, extra2);
		fInfo.fFieldExtraData[fieldID] = extra;
		fInfo.fFieldExtraData2[fieldID] = extra2;
		fInfo.fFieldCalcValue1[fieldID] = "";
		fInfo.fFieldCalcValue2[fieldID] = "";
	}
}

bool
CFMFieldTyper::TypeDeterminationNeedsData(int fieldType)
{
	return (fieldType == typeLongDateTime);
}

void
CFMFieldTyper::ExtraFromType(int , int& extra1, int& extra2)
{
	extra1 = 0;
	extra2 = 0;
}

bool CFMFieldTyper::StringIsTime(const string& inData)
{
	return inData.find(":") != string::npos;
}

int
CFMFieldTyper::JFileFromFMType(int fieldType, const string& sampleData, bool hasChoices)
{
	if (hasChoices)
		return FLDTYPE_LIST;

	int jFileFieldType = FLDTYPE_INT;
	
	switch(fieldType) {
	case typeSInt16:
	case typeSInt32:
	case typeUInt32:
	case typeSInt64:
		jFileFieldType = FLDTYPE_INT;
		break;

	case typeIEEE32BitFloatingPoint:
	case typeIEEE64BitFloatingPoint:
	case type128BitFloatingPoint:
		jFileFieldType = FLDTYPE_FLOAT;
		break;

	case typeBoolean:
		jFileFieldType = FLDTYPE_BOOLEAN;
		break;

	case typeLongDateTime:
		bool isTime = StringIsTime(sampleData);
		if (!isTime)
			jFileFieldType = FLDTYPE_AUTODATE;
		else
			jFileFieldType = FLDTYPE_AUTOTIME;
		break;

	default:
		jFileFieldType = FLDTYPE_STRING;
		break;
	}
	
	return jFileFieldType;
}

CDatabaseInfo
CFMFieldTyper::GetDatabaseInfo() const
{
	return fInfo;
}

