#include "CRecord.h"
#include "FMDatabase.h"
#include "CFileMaker.h"
#include "CFrontProcess.h"
#include "Reader.h"
#include "CAEDescriptor.h"
#include "CDatabaseInfo.h"
#include "StAppleEvent.h"
#include "FMAE.h"
#include "FolderUtilities.h"
#include "Utilities.h"
#include "Stringiness.h"
#include "Str255.h"
#include "WriteText.h"
#include "Descriptors.h"

using Folders::GetFileCreator;
using FMAE::FieldID;

namespace FileMaker {

class StRecordLocker {
public:
	StRecordLocker(Database* database, UInt32);
	~StRecordLocker();
private:
	Database* fDatabase;
	UInt32 fRecordID;
};

StRecordLocker::StRecordLocker(Database* database, UInt32 recordID)
	: fDatabase(database), fRecordID(recordID)
{
	fDatabase->LockRecord(fRecordID);
}

StRecordLocker::~StRecordLocker()
{
	fDatabase->UnlockRecord(fRecordID);
}


const int kMaxFieldDataSize = 32*1024;
static char fieldDataBuffer[kMaxFieldDataSize];

const unsigned long kChoicesProperty = 'pCHS';

const int kFileMakerReadAccessMask = 1;

Database::Database(const FSSpec& databaseSpec, bool useFoundSet, CReaderProgress *progress)
	: fProgress(progress), fHasNonWritableFields(true), fUseFoundSet(useFoundSet), fCheckedForRelated(false), 
	  fLastFieldIDWritten(0), fCheckedForContainers(false), fLayoutID(-1), 
	  fFieldChoicesDirty(true), fRecordIDsDirty(true), fAllRecordIDsDirty(true), fCloseOnDelete(true), fRecordLocked(false)
{
	SetupDatabaseName(databaseSpec);
	CFileMaker& fm = CFileMaker::Get(fName.GetCreator());
	
	fm.Open(databaseSpec, fDatabaseDesc);
	CreateDocumentDescriptor(AsStr255(fName.GetName()));
	CheckForJapaneseSystem();
}

Database::Database(const CDatabaseName& databaseName, bool useFoundSet, CReaderProgress *progress)
	: fName(databaseName), fProgress(progress), fHasNonWritableFields(true), fUseFoundSet(useFoundSet), fCheckedForRelated(false), 
	  fLastFieldIDWritten(0), fCheckedForContainers(false), fLayoutID(-1), 
	  fFieldChoicesDirty(true), fRecordIDsDirty(true), fAllRecordIDsDirty(true), fCloseOnDelete(false), fRecordLocked(false)
{
	CFileMaker& fm = CFileMaker::Get(fName.GetCreator());
	
	fm.GetDatabaseDescriptor(fName.GetName(), fDatabaseDesc);
	CreateDocumentDescriptor(AsStr255(fName.GetName()));
	CheckForJapaneseSystem();
}


Database::~Database()
{
	if (fCloseOnDelete) {
		try {
			if (fDatabaseDesc.HasData())
				CFileMaker::Get(fName.GetCreator()).Close(fDatabaseDesc);
		} catch (...) {
		}
	}
}

void
Database::SetupDatabaseName(const FSSpec& inSpec)
{
	fName = CDatabaseName(AsString(inSpec.name), Folders::GetFileCreator(inSpec));
}

void
Database::CreateDocumentDescriptor(ConstStringPtr databaseName)
{
	CAEDescriptor docName(databaseName);
	CAEDescriptor nullDesc;
	ThrowIfOSErr_(CreateObjSpecifier(cDocument, nullDesc, formName, docName, false, fDocumentDesciptor));
}

void
Database::CheckForJapaneseSystem()
{
	long systemScript = GetScriptManagerVariable(smSysScript);
	fJapanese = (systemScript == smJapanese);
}

void
Database::UseLayout(int layoutID)
{
	if (layoutID == fLayoutID)
		return;
	
	fLayoutID = layoutID;
	FMAE::GetDefaultLayoutDescriptor(fLayout0Desc, fDatabaseDesc);
	if (layoutID == 0) {
		FMAE::GetDefaultLayoutDescriptor(fDocumentLayoutDesc, fDocumentDesciptor);
		FMAE::GetDefaultLayoutDescriptor(fDatabaseLayoutDesc, fDatabaseDesc);
	} else {
		FMAE::GetLayoutIDDescriptor(layoutID, fDocumentLayoutDesc, fDocumentDesciptor);
		FMAE::GetLayoutIDDescriptor(layoutID, fDatabaseLayoutDesc, fDatabaseDesc);
	}
	if (fUseFoundSet)
		fDefaultLayoutDesc = fDocumentLayoutDesc;
	else
		fDefaultLayoutDesc = fDatabaseLayoutDesc;
	
	ShowLayout(fLayoutID);
	Setup();
}

bool
Database::IsMultiUser()
{
	CFileMaker& fm = CFileMaker::Get(fName.GetCreator());
	if (fm.MajorVersion() < 4)
		return false;
		
	DoProgress("Checking multi user status");
	CAEDescriptor isMultiUser;
	FMAE::GetPropertyDescriptor('pMUr', isMultiUser, fDatabaseDesc);
	StAppleEvent theEvent;
	StAppleEvent theReply;
	FMAE::NewGetDataAppleEvent(fName.GetCreator(), theEvent, isMultiUser, kAnyTransactionID);
	theEvent.Send(theReply, kAEFlagsForConduit);
	DescType typeCode;
	long actualSize;
	Boolean isMulti;
	ThrowIfOSErr_(::AEGetParamPtr(theReply, keyAEResult, typeBoolean, 
		&typeCode, &isMulti, sizeof(isMulti), &actualSize));
	return isMulti;
}

map<FieldID, string>
Database::GetAllFieldsNameMap()
{
	map<FieldID, string> theMap;
	
	for (CFieldIDList::const_iterator fidIter = fAllFieldIDs.begin();
			fidIter != fAllFieldIDs.end(); ++fidIter) {
		theMap[*fidIter] = fFieldNames[*fidIter];
	}
	return theMap;
}

map<FieldID, string>
Database::GetFieldNameMap()
{
	map<FieldID, string> theMap;
	
	for (CFieldIDList::const_iterator fidIter = fFieldIDs.begin();
			fidIter != fFieldIDs.end(); ++fidIter) {
		theMap[*fidIter] = fFieldNames[*fidIter];
	}
	return theMap;
}

void
Database::DoScript(FMAE::ScriptID scriptID, bool waitAfter)
{
	DoProgress("Executing a script");
	CAEDescriptor scriptDesc;
	FMAE::GetScriptDescriptor(scriptID, scriptDesc, fDatabaseDesc);
	StAppleEvent theEvent;
	StAppleEvent theReply;
	FMAE::NewDoScriptAppleEvent(fName.GetCreator(), theEvent, scriptDesc);
	try {
		theEvent.Send(theReply, kAEFlagsForConduit);
	} catch (const LException& inErr) {
		if (inErr.GetErrorCode() != connectionInvalid)
			throw;
		waitAfter = false;
	}
	if (waitAfter) {
		try {
			string version(CFileMaker::Get(fName.GetCreator()).GetVersionString());
		} catch (const LException& inErr) {
			if (inErr.GetErrorCode() != connectionInvalid)
				throw;
		}
	}
}

vector<string>
Database::GetLayoutNames()
{
	vector<string> fieldNames;

	DoProgress("Getting layout names");
	CAEDescriptor allLayouts;
	CAEDescriptor allLayoutsNames;
	FMAE::GetAllLayoutsDescriptor(allLayouts, fDatabaseDesc);
	FMAE::GetPropertyDescriptor(pName, allLayoutsNames, allLayouts);
	StAppleEvent theEvent;
	StAppleEvent theReply;
	FMAE::NewGetDataAppleEvent(fName.GetCreator(), theEvent, allLayoutsNames, kAnyTransactionID);
	theEvent.Send(theReply, kAEFlagsForConduit);
	CAEDescriptor result;
	ThrowIfOSErr_(::AEGetParamDesc(theReply, keyDirectObject, typeAEList, result));
	return ExtractStringList(result);
}

vector<string>
Database::GetScriptNames()
{
	vector<string> fieldNames;

	DoProgress("Getting script names");
	CAEDescriptor allScripts;
	CAEDescriptor allScriptsNames;
	FMAE::GetAllScriptsDescriptor(allScripts, fDatabaseDesc);
	FMAE::GetPropertyDescriptor(pName, allScriptsNames, allScripts);
	StAppleEvent theEvent;
	StAppleEvent theReply;
	FMAE::NewGetDataAppleEvent(fName.GetCreator(), theEvent, allScriptsNames, kAnyTransactionID);
	theEvent.Send(theReply, kAEFlagsForConduit);
	CAEDescriptor result;
	ThrowIfOSErr_(::AEGetParamDesc(theReply, keyDirectObject, typeAEList, result));
	return ExtractStringList(result);
}

vector<string_vector>
Database::ExtractStringVectorList(CAEDescriptor& theDesc)
{
	vector<string_vector> list;
	Descriptors::ExtractList(theDesc, list);
	return list;
}

vector<string>
Database::ExtractStringList(CAEDescriptor& theDesc)
{
	vector<string> list;
	Descriptors::ExtractList(theDesc, list);
	return list;
}

bool
Database::HasAEListItems(CAEDescriptor& theDesc)
{
	vector<int> listInts;

	long listCount = 0;
	OSStatus status = ::AECountItems(theDesc, &listCount);
	if (status != noErr)
		return false;
		
	for (int i = 1; i <= listCount; i += 1) {
		AEKeyword keyWord;
		CAEDescriptor listItem;
		ThrowIfOSErr_(AEGetNthDesc(theDesc, i, typeWildCard, &keyWord, listItem));
		long itemListCount = listItem.Count();
		if (itemListCount > 1)
			return true;
	}
	return false;
}

vector<int>
Database::ExtractIntList(CAEDescriptor& theDesc)
{
	vector<int> list;
	Descriptors::ExtractList(theDesc, list);
	return list;
}

vector<int>
Database::ExtractEnumList(CAEDescriptor& theDesc)
{
	vector<int> list;
	Descriptors::ExtractList(theDesc, list);
	return list;
}

vector<int>
Database::GetFieldTypes()
{
	return MakeOrderedList(fFieldTypes, fFieldIDs);
}

vector<int>
Database::GetRecordIDs()
{
	if (fRecordIDsDirty) {
		LoadRecordIDs();
	}
	return fRecordIDs;
}

vector<int>
Database::GetAllRecordIDs()
{
	if (fAllRecordIDsDirty) {
		LoadAllRecordIDs();
	}
	return fAllRecordIDs;
}

CFieldIDList
Database::GetFieldIDs()
{
	return fFieldIDs;
}

void
Database::LoadRecordIDs()
{
	DoProgress("Getting record IDs");
	CAEDescriptor allRecords;
	CAEDescriptor allRecordsIDs;
	FMAE::GetAllRecordsDescriptor(allRecords, fDefaultLayoutDesc);
	FMAE::GetPropertyDescriptor(pID, allRecordsIDs, allRecords);
	StAppleEvent theEvent;
	StAppleEvent theReply;
	FMAE::NewGetDataAppleEvent(fName.GetCreator(), theEvent, allRecordsIDs, kAnyTransactionID);
	theEvent.Send(theReply, kAEFlagsForConduit);
	CAEDescriptor result;
	ThrowIfOSErr_(::AEGetParamDesc(theReply, keyDirectObject, typeAEList, result));
	Descriptors::ExtractList(result, fRecordIDs);
	fRecordIDsDirty = false;
}

void
Database::LoadAllRecordIDs()
{
	DoProgress("Getting all record IDs");
	CAEDescriptor allRecords;
	CAEDescriptor allRecordsIDs;
	FMAE::GetAllRecordsDescriptor(allRecords, fDatabaseLayoutDesc);
	FMAE::GetPropertyDescriptor(pID, allRecordsIDs, allRecords);
	StAppleEvent theEvent;
	StAppleEvent theReply;
	FMAE::NewGetDataAppleEvent(fName.GetCreator(), theEvent, allRecordsIDs, kAnyTransactionID);
	theEvent.Send(theReply, kAEFlagsForConduit);
	CAEDescriptor result;
	ThrowIfOSErr_(::AEGetParamDesc(theReply, keyDirectObject, typeAEList, result));
	Descriptors::ExtractList(result, fAllRecordIDs);
	fAllRecordIDsDirty = false;
}

void
Database::LoadFieldIDs()
{
	DoProgress("Getting field IDs");
	CAEDescriptor allFields;
	CAEDescriptor allFieldsIDs;
	FMAE::GetAllFieldsDescriptor(allFields, fDefaultLayoutDesc);
	FMAE::GetPropertyDescriptor(pID, allFieldsIDs, allFields);
	StAppleEvent theEvent;
	StAppleEvent theReply;
	FMAE::NewGetDataAppleEvent(fName.GetCreator(), theEvent, allFieldsIDs, kAnyTransactionID);
	theEvent.Send(theReply, kAEFlagsForConduit);
	CAEDescriptor result;
	ThrowIfOSErr_(::AEGetParamDesc(theReply, keyDirectObject, typeAEList, result));
	fFieldIDs = FieldID::ExtractFromDescriptor(result);
}

void
Database::LoadLayout0FieldIDs()
{
	DoProgress("Getting field IDs");
	CAEDescriptor allFields;
	CAEDescriptor allFieldsIDs;
	FMAE::GetAllFieldsDescriptor(allFields, fLayout0Desc);
	FMAE::GetPropertyDescriptor(pID, allFieldsIDs, allFields);
	StAppleEvent theEvent;
	StAppleEvent theReply;
	FMAE::NewGetDataAppleEvent(fName.GetCreator(), theEvent, allFieldsIDs, kAnyTransactionID);
	theEvent.Send(theReply, kAEFlagsForConduit);
	CAEDescriptor result;
	ThrowIfOSErr_(::AEGetParamDesc(theReply, keyDirectObject, typeAEList, result));
	fLayout0FieldIDs = FieldID::ExtractFromDescriptor(result);
}

bool
Database::HasRelatedFields()
{
	if (fCheckedForRelated)
		return fHasRelated;
		
	DoProgress("Checking for related fields");
	CAEDescriptor allFields;
	CAEDescriptor allFieldsIDs;
	FMAE::GetAllFieldsDescriptor(allFields, fDefaultLayoutDesc);
	FMAE::GetPropertyDescriptor(pID, allFieldsIDs, allFields);
	StAppleEvent theEvent;
	StAppleEvent theReply;
	FMAE::NewGetDataAppleEvent(fName.GetCreator(), theEvent, allFieldsIDs, kAnyTransactionID);
	theEvent.Send(theReply, kAEFlagsForConduit);
	CAEDescriptor result;
	ThrowIfOSErr_(::AEGetParamDesc(theReply, keyDirectObject, typeAEList, result));
	fHasRelated = HasAEListItems(result);
	fCheckedForRelated = true;
	return fHasRelated;
}

bool
Database::HasContainerFields()
{
	if (fCheckedForContainers)
		return fHasContainers;
		
	fHasContainers = false;
		
	DoProgress("Checking for container fields");
	vector<int> fieldTypes(GetFieldTypes());
	for (vector<int>::const_iterator i = fieldTypes.begin();
		i != fieldTypes.end() && !fHasContainers; ++i) {
		if (*i == 'PICT')
			fHasContainers = true;
	}
	fCheckedForContainers = true;
	return fHasContainers;
}

CFieldIDList
Database::GetAllFieldIDs()
{
	return fAllFieldIDs;
}

vector<string_vector>
Database::GetFieldChoices()
{
	if (fFieldChoicesDirty) {
		SetupChoices();
	}
	return MakeOrderedList(fFieldChoices, fFieldIDs);
}

vector<string_vector>
Database::GetAllFieldChoices()
{
	if (fFieldChoicesDirty) {
		SetupChoices();
	}
	return MakeOrderedList(fFieldChoices, fAllFieldIDs);
}

vector<int>
Database::GetFieldAccess()
{
	return MakeOrderedList(fAccess, fFieldIDs);
}

vector<int>
Database::GetFieldReadOnly()
{
	return MakeOrderedList(fFieldReadOnly, fFieldIDs);
}

vector<int>
Database::GetAllFieldReadOnly()
{
	return MakeOrderedList(fFieldReadOnly, fAllFieldIDs);
}

vector<int>
Database::GetAllFieldAccess()
{
	return MakeOrderedList(fAccess, fAllFieldIDs);
}

vector<int>
Database::GetAllFieldTypes()
{
	return MakeOrderedList(fFieldTypes, fAllFieldIDs);
}

vector<string>
Database::GetFieldNames()
{
	return MakeOrderedList(fFieldNames, fFieldIDs);
}

vector<string>
Database::GetAllFieldNames()
{
	return MakeOrderedList(fFieldNames, fAllFieldIDs);
}

vector<int>
Database::GetFieldRepeats()
{
	return MakeOrderedList(fRepeats, fFieldIDs);
}

vector<int>
Database::GetAllFieldRepeats()
{
	return MakeOrderedList(fRepeats, fAllFieldIDs);
}

vector<int>
Database::GetLayoutIDs()
{
	vector<int> fieldIDs;

	DoProgress("Getting layout IDs");
	CAEDescriptor allLayouts;
	CAEDescriptor allLayoutsIDs;
	FMAE::GetAllLayoutsDescriptor(allLayouts, fDatabaseDesc);
	FMAE::GetPropertyDescriptor(pID, allLayoutsIDs, allLayouts);
	StAppleEvent theEvent;
	StAppleEvent theReply;
	FMAE::NewGetDataAppleEvent(fName.GetCreator(), theEvent, allLayoutsIDs, kAnyTransactionID);
	theEvent.Send(theReply, kAEFlagsForConduit);
	CAEDescriptor result;
	ThrowIfOSErr_(::AEGetParamDesc(theReply, keyDirectObject, typeAEList, result));
	return ExtractIntList(result);
}

vector<FMAE::ScriptID>
Database::GetScriptIDs()
{
	vector<int> fieldIDs;

	DoProgress("Getting script IDs");
	CAEDescriptor allScripts;
	CAEDescriptor allScriptsIDs;
	FMAE::GetAllScriptsDescriptor(allScripts, fDatabaseDesc);
	FMAE::GetPropertyDescriptor(pID, allScriptsIDs, allScripts);
	StAppleEvent theEvent;
	StAppleEvent theReply;
	FMAE::NewGetDataAppleEvent(fName.GetCreator(), theEvent, allScriptsIDs, kAnyTransactionID);
	theEvent.Send(theReply, kAEFlagsForConduit);
	CAEDescriptor result;
	ThrowIfOSErr_(::AEGetParamDesc(theReply, keyDirectObject, typeAEList, result));
	vector<FMAE::ScriptID> scriptIDs;
	Descriptors::ExtractList(result, scriptIDs);
	return scriptIDs;
}

static void
ListToString(CAEDescriptor& field, string& outString)
{
	AEKeyword keyWord;
	long fieldCount = field.Count();
	int fieldIndex;
	for (fieldIndex = 1; fieldIndex <= fieldCount; fieldIndex += 1) {
		DescType typeCode;
		long actualSize;
		ThrowIfOSErr_(AEGetNthPtr(field, fieldIndex, typeChar, &keyWord, &typeCode, fieldDataBuffer, kMaxFieldDataSize, &actualSize));
		if (fieldIndex > 1)
			outString += groupSeparator;
		if (actualSize > 0)
			outString += string(fieldDataBuffer, actualSize);
	}
}

void
Database::ShowRecord(int recordID)
{
	CAEDescriptor recordDesc;
	FMAE::GetRecordIDDescriptor(recordID, recordDesc, fDatabaseLayoutDesc);
	StAppleEvent theEvent;
	StAppleEvent theReply;
	FMAE::NewShowAppleEvent(fName.GetCreator(), theEvent, recordDesc);
	theEvent.Send(theReply, kAEFlagsForConduit);
}

void
Database::ShowLayout(FMAE::LayoutID layoutID)
{
	CAEDescriptor layoutDesc;
	FMAE::GetLayoutIDDescriptor(layoutID, layoutDesc, fDocumentDesciptor);
	StAppleEvent theEvent;
	StAppleEvent theReply;
	FMAE::NewShowAppleEvent(fName.GetCreator(), theEvent, layoutDesc);
	theEvent.Send(theReply, kAEFlagsForConduit);
}

vector<string>
Database::GetRecord(int recordID)
{
	return GetRecord(recordID, fDatabaseLayoutDesc, fFieldIDs);
}

vector<string>
Database::GetRecordAllFields(int recordID)
{
	return MakeOrderedList(GetRecordAllFieldsMap(recordID), fAllFieldIDs);
}

map<FieldID, string>
Database::GetRecordAllFieldsMap(int recordID)
{
	vector<string> layout0Fields(GetRecord(recordID, fLayout0Desc, fLayout0FieldIDs));
	map<FieldID, string> dataInMap(MakeMap(fLayout0FieldIDs, layout0Fields));

	vector<string> selectedLayoutFields(GetRecord(recordID, fDatabaseLayoutDesc, fFieldIDs));
	AddToMap(fFieldIDs, selectedLayoutFields, dataInMap);
	return dataInMap;
}

vector<string>
Database::GetRecord(int recordID, const AEDesc *layout, const vector<FieldID>& fieldIDs)
{
	DoProgress("Getting record");
	vector<string> fields;
	CAEDescriptor recordDesc;
	FMAE::GetRecordIDDescriptor(recordID, recordDesc, layout);
	StAppleEvent theEvent;
	StAppleEvent theReply;
	FMAE::NewGetDataAppleEvent(fName.GetCreator(), theEvent, recordDesc, kAnyTransactionID);
	theEvent.Send(theReply, kAEFlagsForConduit);
	CAEDescriptor record;
	ThrowIfOSErr_(::AEGetParamDesc(theReply, keyDirectObject, typeAEList, record));
	AEKeyword keyWord;
	long fieldCount = record.Count();
	int fieldIndex;
	fields.reserve(fieldCount);
	for (fieldIndex = 1; fieldIndex <= fieldCount; fieldIndex += 1) {
		int fieldIndex0 = fieldIndex - 1;
		FieldID fieldID(fieldIDs.at(fieldIndex0));
		string fieldString;
		int fieldType = fFieldTypes[fieldID];
		if (fieldType != 'PICT') {
			CAEDescriptor field;
			ThrowIfOSErr_(AEGetNthDesc(record, fieldIndex, typeAEList, &keyWord, field));
			ListToString(field, fieldString);
		}
		fields.push_back(fieldString);
	}
	fieldDataBuffer[0] = 0;
	for (; fieldIndex <= fieldCount; fieldIndex += 1) {
		fields.push_back(kEmptyString);
	}

	return fields;
}

vector<string>
Database::GetFields()
{
	vector<string> fields;
	GetFieldTypes();
	GetFieldRepeats();
	CAEDescriptor allRecords;
	int recordIndex = 1;
	int allRecordCount = FoundRecordsCount();
	int recordChunk = 2000/fFieldIDs.size();
	fields.reserve(allRecordCount * fFieldIDs.size());
	while (allRecordCount > 0) {
		CAEDescriptor recordRangeDesc;
		if (recordChunk > allRecordCount)
			recordChunk = allRecordCount;
		int recordEndIndex = recordIndex + recordChunk - 1;
		string progStr("Getting records ");
		progStr.append(AsString(recordIndex));
		progStr.append(" to ");
		progStr.append(AsString(recordEndIndex));
		DoProgress(progStr.c_str());
		FMAE::GetRecordRangeDescriptor(recordIndex, recordEndIndex, recordRangeDesc, fDefaultLayoutDesc);
		StAppleEvent theEvent;
		StAppleEvent theReply;
		FMAE::NewGetDataAppleEvent(fName.GetCreator(), theEvent, recordRangeDesc, kAnyTransactionID);
		theEvent.Send(theReply, kAEFlagsForConduit);
		allRecordCount -= recordChunk;
		recordIndex += recordChunk;
		DoProgress("Processing fields");
		CAEDescriptor result;
		ThrowIfOSErr_(::AEGetParamDesc(theReply, keyDirectObject, typeAEList, result));
		for (int i = 1; i <= recordChunk; i += 1) {
			CAEDescriptor record;
			if (recordChunk == 1) {
				record = result;
			} else {
				AEKeyword keyWord;
				ThrowIfOSErr_(::AEGetNthDesc(result, i, typeAEList, &keyWord, record));
			}
			long fieldCount = record.Count();
			int fieldIndex;
			fields.reserve(fieldCount);
			for (fieldIndex = 1; fieldIndex <= fieldCount; fieldIndex += 1) {
				int fieldIndex0 = fieldIndex - 1;
				FieldID fieldID(fFieldIDs.at(fieldIndex0));
				int fieldType = fFieldTypes[fieldID];
				string fieldString;
				if (fieldType != 'PICT') {
					CAEDescriptor field;
					AEKeyword keyWord;
					ThrowIfOSErr_(AEGetNthDesc(record, fieldIndex, typeAEList, &keyWord, field));
					ListToString(field, fieldString);
				}
				fields.push_back(fieldString);
			}
			fieldDataBuffer[0] = 0;
			for (; fieldIndex <= fieldCount; fieldIndex += 1) {
				fields.push_back(kEmptyString);
			}
		}
	}

	return fields;
}

void
Database::DoProgress(const string& progressString)
{
	if (fProgress)
		fProgress->DoProgress(progressString);
}

int
Database::GetRecordID(const CAEDescriptor& recordReference)
{
	StAppleEvent theGetIDEvent;
	StAppleEvent theGetIDReply;
	CAEDescriptor recordID;
	FMAE::GetPropertyDescriptor(pID, recordID, recordReference);
	FMAE::NewGetDataAppleEvent(fName.GetCreator(), theGetIDEvent, recordID);
	theGetIDEvent.Send(theGetIDReply, kAEFlagsForConduit);
	DescType typeCode;
	long actualSize;
	UInt32 resultID;
	ThrowIfOSErr_(::AEGetKeyPtr(theGetIDReply, keyDirectObject, typeLongInteger, 
		&typeCode, &resultID, sizeof(resultID), &actualSize));
	return resultID;
}

int
Database::AddRecordSimple(const vector<string>& fieldData)
{
	StAppleEvent theEvent;
	StAppleEvent theReply;
	DoProgress("Adding record");
	OSType recordTypeCode = cRow;
	CAEDescriptor recordType(typeType, &recordTypeCode, sizeof(recordTypeCode));
	CAEDescriptor recordDataDesc;
	FieldListToDescriptor(fieldData, recordDataDesc, fJapanese);
	FMAE::NewCreateNewAppleEvent(fName.GetCreator(), theEvent, recordType, recordDataDesc);
	theEvent.Send(theReply, kAEFlagsForConduit);
	
	CAEDescriptor recordReference;
	ThrowIfOSErr_(::AEGetParamDesc(theReply, keyDirectObject, typeObjectSpecifier, recordReference));
	return GetRecordID(recordReference);
}

int
Database::AddRecord(const CFieldIDList& fieldIDs, const vector<string>& fieldData)
{
	if (fieldIDs == fFieldIDs)
		return AddRecordSimple(fieldData);

	return AddRecordComplex(fieldIDs, fieldData);
}

int
Database::AddRecordComplex(const CFieldIDList& fieldIDs, const vector<string>& fieldData)
{
	long resultID = 0;
	StAppleEvent theEvent;
	StAppleEvent theReply;
	DoProgress("Adding record");
	OSType recordTypeCode = cRow;
	CAEDescriptor recordType(typeType, &recordTypeCode, sizeof(recordTypeCode));
	FMAE::NewCreateNewAppleEvent(fName.GetCreator(), theEvent, recordType);
	theEvent.Send(theReply, kAEFlagsForConduit);
	
	StAppleEvent theGetIDEvent;
	StAppleEvent theGetIDReply;
	CAEDescriptor recordReference;
	CAEDescriptor recordID;
	ThrowIfOSErr_(::AEGetParamDesc(theReply, keyDirectObject, typeObjectSpecifier, recordReference));
	FMAE::GetPropertyDescriptor(pID, recordID, recordReference);
	FMAE::NewGetDataAppleEvent(fName.GetCreator(), theGetIDEvent, recordID);
	theGetIDEvent.Send(theGetIDReply, kAEFlagsForConduit);
	DescType typeCode;
	long actualSize;
	ThrowIfOSErr_(::AEGetKeyPtr(theGetIDReply, keyDirectObject, typeLongInteger, 
		&typeCode, &resultID, sizeof(resultID), &actualSize));
		
	try {
		//ShowRecord(resultID);
		map<FieldID, string> recordMap(GetRecordAllFieldsMap(resultID));
		vector<string> record(fieldData);
		int fieldCount = fieldIDs.size();
		for (int i = 0; i < fieldCount; i += 1) {
			if (record.at(i).length() == 0) {
				record.at(i) = recordMap[fieldIDs.at(i)];
			}
		}
		WriteRecord(resultID, fieldIDs, record);
	} catch (const LException& inErr) {
		DeleteRecord(resultID, false);
		throw;
	}
		
	return resultID;
}

static vector<string>
StringToList(const string& inData)
{
	const string kSeparatorString(1, groupSeparator);
	return SplitString(inData, kSeparatorString);
}

void  Database::FieldListToDescriptor(const vector<string>& fieldData, CAEDescriptor& fieldListDescriptor, bool inJapanese)
{
	ThrowIfOSErr_(AECreateList(0, 0, false, fieldListDescriptor));
	for (int i = 0; i < fieldData.size(); ++i) {
		CAEDescriptor fieldDataDesc;
		FieldToDescriptor(fieldData.at(i), fRepeats[fFieldIDs.at(i)], fieldDataDesc, fJapanese);
		ThrowIfOSErr_(AEPutDesc(fieldListDescriptor, i+1, fieldDataDesc));
	}
}

void
Database::FieldToDescriptor(string theField, int repeatCount, CAEDescriptor& fieldData, bool inJapanese)
{
	if (repeatCount > 1) {
		vector<string> fieldLines = StringToList(theField);
		int lineCount = fieldLines.size();
		ThrowIfOSErr_(AECreateList(0, 0, false, fieldData));
		for (int rIndex = 0; rIndex < repeatCount; rIndex += 1) {
			if (rIndex >= lineCount) {
				ThrowIfOSErr_(AEPutPtr(fieldData, rIndex+1, typeChar, 
					kEmptyString.data(), kEmptyString.length()));
			} else {
				ThrowIfOSErr_(AEPutPtr(fieldData, rIndex+1, typeChar, 
					fieldLines.at(rIndex).data(), fieldLines.at(rIndex).length()));
			}
		}
	} else {
		if (inJapanese) {
			int totalLength = theField.length() + 4;
			StPointerBlock dataBlock(totalLength);
			short *scriptids = (short*)Ptr(dataBlock);
			*scriptids++ = smJapanese;
			*scriptids++ = langJapanese;
			SysBeep(0);
			::BlockMoveData(theField.data(), scriptids, theField.length());
			fieldData = CAEDescriptor(cIntlText, Ptr(dataBlock), totalLength);
		} else {
			fieldData = CAEDescriptor(typeChar, theField.data(), theField.length());
		}
	}
}

bool
Database::HasNonWritableFields()
{
	return fHasNonWritableFields;
}

void
Database::WriteRecord(Uint32 recordID, const CFieldIDList& fieldIDs, const vector<string>& fieldData)
{
	StRecordLocker locker(this, recordID);
	if (fieldIDs == fFieldIDs)
		WriteRecordSimple(recordID, fieldData);
	else
		WriteRecordComplex(recordID, fieldIDs, fieldData);
}

void Database::WriteRecordLow(Uint32 recordID, const vector<string>& fieldData)
{
	fLastFieldIDWritten = FieldID();
	CAEDescriptor recordIDDesc;
	FMAE::GetRecordIDDescriptor(recordID, recordIDDesc, fDatabaseLayoutDesc);
	CAEDescriptor recordDataDesc;
	FieldListToDescriptor(fieldData, recordDataDesc, fJapanese);
	StAppleEvent theEvent;
	StAppleEvent theReply;
	FMAE::NewSetDataAppleEvent(fName.GetCreator(), theEvent, recordIDDesc, recordDataDesc);
	theEvent.Send(theReply, kAEFlagsForConduit);
	UpdateFieldChoices();
}

void
Database::WriteRecordSimple(Uint32 recordID, const vector<string>& fieldData)
{
	vector<string> oldData(GetRecord(recordID));
	
	int fieldIndex = 0;
	vector<string>::const_iterator new_iter = fieldData.begin();
	vector<string>::iterator old_iter = oldData.begin();
	while (old_iter != oldData.end()) {
		if (fFieldReadOnly[fFieldIDs.at(fieldIndex)]) {
			*old_iter = "";
		} else if (new_iter->length() != 0) {
			*old_iter = *new_iter;
		}
		++old_iter;
		++new_iter;
		++fieldIndex;
	}
	WriteRecordLow(recordID, oldData);
}

void
Database::WriteRecordComplex(Uint32 recordID, const CFieldIDList& fieldIDs, const vector<string>& fieldData)
{
	vector<string> oldData(GetRecordAllFields(recordID));
	map<FieldID, string> dataMap(MakeMap(fAllFieldIDs, oldData));
	
	CFieldIDList::const_iterator i = fieldIDs.begin();
	vector<string>::const_iterator j = fieldData.begin();
	while (i != fieldIDs.end() &&  j != fieldData.end()) {
		if (FieldExists(*i)) {
			string oldData(dataMap[*i]);
			if (*j != oldData)
				WriteFieldByID(recordID, *i, *j);
		}
		++i;
		++j;
	}
}

string
Database::GetFieldByID(Uint32 recordID, FieldID fieldID)
{
	DoProgress("Getting field");
	string fieldString;
	if (fFieldTypes[fieldID] != 'PICT') {
		CAEDescriptor fieldIDDesc;
		FMAE::GetFieldIDDescriptor(fieldID, recordID, fieldIDDesc, fDatabaseLayoutDesc);
		StAppleEvent theEvent;
		StAppleEvent theReply;
		FMAE::NewGetDataAppleEvent(fName.GetCreator(), theEvent, fieldIDDesc);
		theEvent.Send(theReply, kAEFlagsForConduit);
		int repeatCount = fRepeats[fieldID];
		CAEDescriptor field;
		ThrowIfOSErr_(::AEGetParamDesc(theReply, keyDirectObject, typeAEList, field));
		ListToString(field, fieldString);
	}
	return fieldString;
}

void
Database::WriteFieldByID(Uint32 recordID, FieldID fieldID, const string& fieldData)
{
	DoProgress("Writing field");
	if (FieldExists(fieldID)) {
		if (!fFieldReadOnly[fieldID]) {
			CAEDescriptor fieldIDDesc;
			fLastFieldIDWritten = fieldID;
			int field, relationship;
			if (fieldID.GetValue(field, relationship)) {
				FMAE::GetFieldIDDescriptor(fieldID, recordID, fieldIDDesc, fDatabaseLayoutDesc);
			} else {
				FMAE::GetFieldIDDescriptor(fieldID, recordID, fieldIDDesc, fLayout0Desc);
			}
			CAEDescriptor fieldDataDesc;
			int repeatCount = fRepeats[fieldID];
			FieldToDescriptor(fieldData, repeatCount, fieldDataDesc, fJapanese);
			StAppleEvent theEvent;
			StAppleEvent theReply;
			FMAE::NewSetDataAppleEvent(fName.GetCreator(), theEvent, fieldIDDesc, fieldDataDesc);
			theEvent.Send(theReply, kAEFlagsForConduit);
			UpdateFieldChoices();
			fLastFieldIDWritten = FieldID();
		}
	}
}

void
Database::WriteField(Uint32 recordID, int fieldIndex, const string& fieldData)
{
	WriteFieldByID(recordID, fFieldIDs.at(fieldIndex), fieldData);
}

void
Database::DeleteRecord(Uint32 recordID, bool withProgress)
{
	if (withProgress)
		DoProgress("Deleting record");
	CAEDescriptor record;
	FMAE::GetRecordIDDescriptor(recordID, record, fDatabaseLayoutDesc);
	StAppleEvent theEvent;
	StAppleEvent theReply;
	FMAE::NewDeleteAppleEvent(fName.GetCreator(), theEvent, record);
	theEvent.Send(theReply, kAEFlagsForConduit);
	UpdateFieldChoices();
}

bool
Database::DoesExist(AEDesc* thisItem)
{
	StAppleEvent theEvent;
	StAppleEvent theReply;
	FMAE::NewDoesExistAppleEvent(fName.GetCreator(), theEvent, thisItem);
	theEvent.Send(theReply, kAEFlagsForConduit);
	DescType typeCode;
	long actualSize;
	Boolean wasFound;
	ThrowIfOSErr_(::AEGetParamPtr(theReply, keyAEResult, typeBoolean, 
		&typeCode, &wasFound, sizeof(wasFound), &actualSize));
	return wasFound;
}

long
Database::Count(AEDesc* thisItem)
{
	StAppleEvent theEvent;
	StAppleEvent theReply;
	FMAE::NewCountAppleEvent(fName.GetCreator(), theEvent, fDefaultLayoutDesc, thisItem);
	theEvent.Send(theReply, kAEFlagsForConduit);
	DescType typeCode;
	long actualSize;
	long theCount;
	ThrowIfOSErr_(::AEGetParamPtr(theReply, keyAEResult, typeLongInteger, 
		&typeCode, &theCount, sizeof(theCount), &actualSize));
	return theCount;
}

void
Database::EnterBrowseMode()
{
	DoProgress("Entering browse mode");
	try {
		CAEDescriptor menuIDDesc;
		FMAE::GetMenuIDDescriptor(4, menuIDDesc);
		if (!DoesExist(menuIDDesc)) {
			FMAE::GetMenuIDDescriptor(22, menuIDDesc);
		}
		CAEDescriptor menuItemDesc;
		FMAE::GetMenuItemDescriptor(1, menuItemDesc, menuIDDesc);

		StAppleEvent theEvent;
		StAppleEvent theReply;
		FMAE::NewDoMenuAppleEvent(fName.GetCreator(), theEvent, menuItemDesc);
		theEvent.Send(theReply, kAEFlagsForConduit);
	} catch (...) {
	}
}

void
Database::FindAll()
{
	DoProgress("Finding all records");
	CAEDescriptor menuIDDesc;
	FMAE::GetMenuIDDescriptor(5, menuIDDesc);
	CAEDescriptor menuItemDesc;
	FMAE::GetMenuItemDescriptor(1, menuItemDesc, menuIDDesc);
	CAEDescriptor propertyDesc;
	FMAE::GetPropertyDescriptor(pEnabled, propertyDesc, menuItemDesc);

	StAppleEvent thePropEvent;
	StAppleEvent thePropReply;
	FMAE::NewGetDataAppleEvent(fName.GetCreator(), thePropEvent, propertyDesc);
	thePropEvent.Send(thePropReply, kAEFlagsForConduit);
	DescType typeCode;
	long actualSize;
	Boolean isEnabled;
	ThrowIfOSErr_(::AEGetParamPtr(thePropReply, keyAEResult, typeBoolean, 
		&typeCode, &isEnabled, sizeof(isEnabled), &actualSize));

	if (isEnabled) {
		StAppleEvent theEvent;
		StAppleEvent theReply;
		FMAE::NewDoMenuAppleEvent(fName.GetCreator(), theEvent, menuItemDesc);
		theEvent.Send(theReply, kAEFlagsForConduit);
	}
}

int
Database::FieldCount()
{
	if (fFieldIDs.size() > 0)
		return fFieldIDs.size();
	return GetFieldIDs().size();
}

bool
Database::FieldExists(FieldID fieldID)
{
	return fFieldExists[fieldID];
}

bool
Database::FieldReadOnly(FieldID fieldID)
{
	return fFieldReadOnly[fieldID];
}

bool
Database::RecordExists(int recordID)
{
	CAEDescriptor recordDesc;
	FMAE::GetRecordIDDescriptor(recordID, recordDesc, fDatabaseLayoutDesc);
	return DoesExist(recordDesc);
}

bool
Database::ScriptExists(FMAE::ScriptID scriptID)
{
	CAEDescriptor scriptDesc;
	FMAE::GetScriptDescriptor(scriptID, scriptDesc, fDatabaseDesc);
	return DoesExist(scriptDesc);
}

bool
Database::LayoutExists(int layoutID)
{
	CAEDescriptor layoutDesc;
	FMAE::GetLayoutIDDescriptor(layoutID, layoutDesc, fDatabaseDesc);
	return DoesExist(layoutDesc);
}

FieldID
Database::LastFieldWritten()
{
	return fLastFieldIDWritten;
}

int
Database::FoundRecordsCount()
{
	vector<int> foundRecordIDs(GetRecordIDs());
#ifdef DEMO
	if (foundRecordIDs.size() > DEMO_RECORD_LIMIT)
		return DEMO_RECORD_LIMIT;
#endif		
	return foundRecordIDs.size();
}

int
Database::AllRecordsCount()
{
	if (fAllRecordIDs.size() < 1)
		GetAllRecordIDs();
	return fAllRecordIDs.size();
}

void
Database::Setup()
{
	fCheckedForContainers = false;
	fCheckedForRelated = false;
	LoadFieldIDs();
	LoadLayout0FieldIDs();
	SetupAllFields();
	SetupRepeats();
	SetupFieldTypes();
	SetupNames();
	//SetupChoices();
	SetupAccess();
	SetupFieldExists();
	//LoadAllRecordIDs();
	//LoadRecordIDs();
}

void
Database::SetupAllFields()
{
	vector<FieldID> layoutFields(fFieldIDs);
	vector<FieldID> layout0Fields(fLayout0FieldIDs);
	fAllFieldIDs.clear();
	sort(layoutFields.begin(), layoutFields.end());
	sort(layout0Fields.begin(), layout0Fields.end());
	set_union(layoutFields.begin(), layoutFields.end(),
		layout0Fields.begin(), layout0Fields.end(),
		back_inserter(fAllFieldIDs));
}

void
Database::SetupRepeats()
{
	SetupPropertyMap('pRPS', fRepeats);
}

void
Database::GetAllFieldsProperty(int property, const AEDesc* layout, AEDesc* props, int fieldCount)
{
	CAEDescriptor allFields;
	CAEDescriptor allFieldsProperty;
	FMAE::GetAllFieldsDescriptor(allFields, layout);
	FMAE::GetPropertyDescriptor(property, allFieldsProperty, allFields);
	StAppleEvent theEvent;
	StAppleEvent theReply;
	FMAE::NewGetDataAppleEvent(fName.GetCreator(), theEvent, allFieldsProperty, kAnyTransactionID);
	theEvent.Send(theReply, kAEFlagsForConduit);
	ThrowIfOSErr_(::AEGetParamDesc(theReply, keyDirectObject, typeAEList, props));
	if (property == kChoicesProperty && fieldCount == 1) {
		CAEDescriptor singleResult;
		ThrowIfOSErr_(::AEGetParamDesc(theReply, keyDirectObject, typeAEList, singleResult));
		ThrowIfOSErr_(AECreateList(0, 0, false, props));
		ThrowIfOSErr_(AEPutDesc(props, 1, singleResult));
	} else {
		ThrowIfOSErr_(::AEGetParamDesc(theReply, keyDirectObject, typeAEList, props));
	}
}

void
Database::SetupAccess()
{
	SetupPropertyMap('pACS', fAccess);
	fHasNonWritableFields = false;
	for (map<FieldID, int>::const_iterator i = fAccess.begin(); 
			i != fAccess.end(); ++i) {
		int access = i->second;
		bool readOnly = ((access & kFileMakerWriteAccessMask) == 0);
		fFieldReadOnly[i->first] = readOnly;
		if(readOnly)
			fHasNonWritableFields = readOnly;
	}
}

void
Database::SetupFieldExists()
{
	CFieldIDList::const_iterator i;
	for (i = fAllFieldIDs.begin(); i != fAllFieldIDs.end(); ++i) {
		fFieldExists[*i] = true;
	}
}

void
Database::SetupFieldTypes()
{
	SetupPropertyMap(keyAEDefaultType, fFieldTypes);
}

void
Database::SetupNames()
{
	SetupPropertyMap(pName, fFieldNames);
	for (map<FieldID,string>::iterator i = fFieldNames.begin(); i != fFieldNames.end(); ++i) {
		if (i->first.IsRelated())
			i->second = FMAE::SimpleFieldName(i->second);
	}
}

void
Database::SetupChoices()
{
	SetupPropertyMap(kChoicesProperty, fFieldChoices);
	fFieldChoicesDirty = false;
}

void
Database::UpdateFieldChoices()
{
	fFieldChoicesDirty = true;
}

int
Database::MajorVersion()
{
	CFileMaker& fm = CFileMaker::Get(fName.GetCreator());
	return fm.MajorVersion();
}

void
Database::LockRecord(UInt32 recordID)
{
	ThrowIf_(fRecordLocked);
	fRecordLocked = true;
	fLockedRecordID = recordID;
}

void
Database::UnlockRecord(UInt32 recordID)
{
	ThrowIf_(!fRecordLocked);
	ThrowIf_(fLockedRecordID != recordID);
	fRecordLocked = false;
	fRecordLocked = 0;
}



}