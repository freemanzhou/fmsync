#include <functional>

#include "CFileMakerIterator.h"
#include "FMDatabase.h"
#include "CFMFieldTyper.h"
#include "WriteJFile2.h"
#include "CDatabaseFile.h"
#include "CDatabaseInfo.h"
#include "CConduitSettings.h"
#include "StAppleEvent.h"
#include "ErrorCodes.h"

#include "CSyncRecord.h"
#include "Utilities.h"
#include "DebugOutput.h"

#include "UGladUtilities.h"
#include "UConduitUtils.h"
#include "HotSyncDefines.h"
#include "HotSyncDefines.h"
#include "JFileConduit.h"
#include "JFilePro.h"
#include "FieldIDFactory.h"
#include "CJFileSynchronizer.h"

#include "Stringiness.h"
#include "charset.h"

bool dateCacheInit = false;
DateCacheRecord dateCache;

CFileMakerRecordIterator::CFileMakerRecordIterator(const CSyncProperties& inSyncProperties, 
	short inDBIndex, CDatabaseFile::Ptr dFile, FileMaker::DatabasePtr database) :
	CRecordIterator(inSyncProperties,inDBIndex), fDatabaseFile(dFile), 
		fFileMakerDatabase(database), mCurrentIndex(0), fDoingDeleted(false),
		fIgnore(false)
{
	ThrowIfNil_(fDatabaseFile.get());
	ThrowIfNil_(fFileMakerDatabase.get());
	if (!dateCacheInit) {
		ThrowIfOSErr_(InitDateCache(&dateCache));
		dateCacheInit = true;
	}
}


CFileMakerRecordIterator::~CFileMakerRecordIterator()
{
}


void CFileMakerRecordIterator::DebugStreamRecord(const CSyncRecord& inCSyncRecord)//Override
{
}

Boolean CFileMakerRecordIterator::GetRecordByID(long inRemoteRecID, eSyncTypes inSyncType, CSyncRecord& ioSyncRecord)
{
#ifdef DEMO
	if (fAllRecordIDs.size() > DEMO_RECORD_LIMIT)
		Throw_(kDemoLimitExceeded);
#endif

	if (fIgnore)
		return false;

	if (fRemoteIDToIndex.find(inRemoteRecID) == fRemoteIDToIndex.end()) {
		if (fPilotToFM.find(inRemoteRecID) != fPilotToFM.end()) {
			int fmID = fPilotToFM[inRemoteRecID];
			if (fRecordsToDelete[fmID]) {
				ioSyncRecord.SetRemoteRecordID(inRemoteRecID);
				ioSyncRecord.SetLocalRecordID(fmID);
				ioSyncRecord.SetAttributes(0);
				ioSyncRecord.SetCategoryID(0);
				ioSyncRecord.SetDeleted(true);
				return true;
			}
		}
		return false;
	}
		
	int recordIndex = fRemoteIDToIndex[inRemoteRecID];
	int fmID = fRecordIDs.at(recordIndex);
	if (fRecordsToDelete[fmID])
		return false;
	
	return GetRecordByIndex(recordIndex+1, inSyncType, ioSyncRecord);
}

Boolean CFileMakerRecordIterator::GetRecordByIndex(long inIndex, eSyncTypes inSyncType, CSyncRecord& ioSyncRecord) 
{
#ifdef DEMO
	if (inIndex > DEMO_RECORD_LIMIT)
		return false;
#endif		

	if (fIgnore)
		return false;

	inIndex -= 1;
	if (inIndex >= fRecordIDs.size() || inIndex < 0)
		return false;
	
	vector<string> record;
	fDatabase.GetRecord(inIndex, record);
	for (int fieldIndex = 0; fieldIndex < fFieldIDs.size(); fieldIndex += 1) {
		string field(record.at(fieldIndex));
		ioSyncRecord.SetValue(FieldIDFactory::GetID(fFieldIDs.at(fieldIndex)), field);
	}
	int fmRecID = fRecordIDs.at(inIndex);
	
	if (fRecordsToDelete[fmRecID])
		return false;

	ioSyncRecord.SetLocalRecordID(fmRecID);
	ioSyncRecord.SetAttributes(0);
	ioSyncRecord.SetCategoryID(0);
	CRecordMap::const_iterator i = fFMIDToRecInfo.find(fmRecID);
	if (i == fFMIDToRecInfo.end()) {
		ioSyncRecord.SetRemoteRecordID(0);
	} else {
		ioSyncRecord.SetRemoteRecordID(i->second.fRecordID);
		if (!CDataSource::DigestFieldsMatch(record, i->second.fDigest)) {
			ioSyncRecord.SetModified(true); 
		}
	}
	return true;
}

UInt32 CFileMakerRecordIterator::WriteRecord(const CSyncRecord& inRecord) 
{
	Throw_(paramErr);
	return 0;
}

UInt32
CFileMakerRecordIterator::AddRecord(const CSyncRecord& inRecord)
{
	vector<string> fields;
	
	for (int fieldIndex = 0; fieldIndex < fFieldIDs.size(); fieldIndex += 1) {
		fields.push_back(inRecord.GetValue( FieldIDFactory::GetID(fFieldIDs.at(fieldIndex))));
	}
	
	UInt32 pilotRecordID = inRecord.GetRemoteRecordID();
	UInt32 fmID = fFileMakerDatabase->AddRecord(fFieldIDs, fields);
	LogDatabaseError();
	fields = fFileMakerDatabase->GetRecord(fmID);
	fDatabase.AppendFields(fields);
	fRecordIDs.push_back(fmID);
	
	CRecord record(pilotRecordID, fields);
	fFMIDToRecInfo[fmID] = CRecordInfo(record);
	fRemoteIDToIndex[pilotRecordID] = fDatabase.RecordCount() - 1;
	fLocalIDToIndex[fmID] = fDatabase.RecordCount() - 1;
	return fmID;
}


void
CFileMakerRecordIterator::DoDeleteRecords() 
{
}

void
CFileMakerRecordIterator::DoAddRecords() 
{
}

void
CFileMakerRecordIterator::DoChangeRecords() 
{
}

void CFileMakerRecordIterator::DeleteRecord(const CSyncRecord& inRecord) 
{
	UInt32 recordID = inRecord.GetLocalRecordID();
	ThrowIf_(recordID == 0);
	fFileMakerDatabase->DeleteRecord(recordID);
	LogDatabaseError();
}


void CFileMakerRecordIterator::PreSync()
{
	ConfirmFreeStackSpace(); 
	FSSpec spec;
	int fmRecordID;
	fDatabaseFile->GetFSSpec(&spec);
	CallConduitProgress("Getting field names");
	fFieldNames = fFileMakerDatabase->GetFieldNames();
	fFieldIDs = fFileMakerDatabase->GetFieldIDs();
	fRecordIDs = fFileMakerDatabase->GetRecordIDs();
	fAllRecordIDs = fFileMakerDatabase->GetAllRecordIDs();
#ifdef DEMO
	if (fAllRecordIDs.size() > DEMO_RECORD_LIMIT)
		Throw_(kDemoLimitExceeded);
#endif
	fDatabase.SetFieldNames(fFieldNames);
	fFieldCount = fFieldNames.size();
	CallConduitProgress("Getting fields");
	vector<string> fields(fFileMakerDatabase->GetFields());
	CallConduitProgress("Getting fields");
	fDatabase.AppendFields(fields);
	
	CRecordMap fmIDToRecInfo;
	fFMIDToRecInfo = fDatabaseFile->GetRecordMap();
	int recordCount = fDatabase.RecordCount();
	for (long index = 0; index < recordCount; index += 1) {
		if ((index % 10) == 0)
			CallConduitProgress("Processing stored record information");
		fmRecordID = fRecordIDs.at(index);
		CRecordMap::iterator i(fFMIDToRecInfo.find(fmRecordID));
		if (i == fFMIDToRecInfo.end()) {
			// new record, never synced, add blank record to map
			CRecordInfo info;
			fmIDToRecInfo[fmRecordID] = info;
		} else {
			// existing record, cary info record over
			UInt32 recordID = (*i).second.fRecordID;
			fRemoteIDToIndex[recordID] = index;
			fLocalIDToIndex[fmRecordID] = index;
			fIndexToRecID[index] = recordID;
			fmIDToRecInfo[fmRecordID] = (*i).second;
			fPilotToFM[recordID] = fmRecordID;
		}
	}
	
	int index = 0;
	// look for mapped record ids that are no longer in database
	CRecordMap::const_iterator i = fFMIDToRecInfo.begin();
	while (i != fFMIDToRecInfo.end()) {
		if ((index % 10) == 0)
			CallConduitProgress("Looking for deleted FileMaker records");
		fmRecordID = (*i).first;
		CRecordMap::const_iterator f = fmIDToRecInfo.find(fmRecordID);
		if (f == fmIDToRecInfo.end()) {
			fPilotRecordsToDelete.push_back((*i).second);
			fRecordIDsToDelete.push_back(fmRecordID);
			fPilotToFM[(*i).second.fRecordID] = fmRecordID;
			fRecordsToDelete[fmRecordID] = true;
		}
		++i;
		index += 1;
	}
	fFMIDToRecInfo = fmIDToRecInfo;
}

void CFileMakerRecordIterator::PostSync(bool wasCanceled)
{
	if (fDatabaseFile.get() && fFileMakerDatabase.get() && !wasCanceled) {
		if (fDatabaseFile->GetSyncMode() == CDatabaseFile::SyncUploadAndClear)
			fDatabaseFile->ClearRecordMap();
		else
			fDatabaseFile->SetRecordMap(fFMIDToRecInfo);
		FMAE::ScriptID scriptID(fDatabaseFile->GetPostSyncScriptID());
		if (scriptID.IsValid()) {
			fFileMakerDatabase->DoScript(scriptID, false);
		}
	}
}

void CFileMakerRecordIterator::UpdateRecordID(CSyncRecord& inRecord, UInt32 inNewRecordID)//Override
{
	Throw_(paramErr);
	long fmID = inRecord.GetRemoteRecordID();
	if (fmID != 0) {
		CRecordInfo newInfo(fFMIDToRecInfo[fmID]);
		UInt32 oldRecID = newInfo.fRecordID;
		newInfo.fRecordID = inNewRecordID;
		fFMIDToRecInfo[fmID] = newInfo;
		if (oldRecID != 0) {
			UInt32 index = fRemoteIDToIndex[oldRecID];
			fRemoteIDToIndex[inNewRecordID] = index;
			fIndexToRecID[index] = inNewRecordID;
		}
	}
	CRecordIterator::UpdateRecordID(inRecord, inNewRecordID);
}

Boolean CFileMakerRecordIterator::NextRecord(eSyncTypes inSyncType, CSyncRecord& ioSyncRecord)//Override
{
#ifdef DEMO
	if (fAllRecordIDs.size() > DEMO_RECORD_LIMIT)
		Throw_(kDemoLimitExceeded);
#endif
	if (fIgnore)
		return false;

	if (fDoingDeleted) {
		if (mCurrentIndex >= fPilotRecordsToDelete.size())
			return false;
		CRecordInfo info(fPilotRecordsToDelete.at(mCurrentIndex));
	
		int recordID = fRecordIDsToDelete.at(mCurrentIndex);
		ioSyncRecord.SetRemoteRecordID(info.fRecordID);
		ioSyncRecord.SetLocalRecordID(recordID);
		ioSyncRecord.SetAttributes(0);
		ioSyncRecord.SetCategoryID(0);
		ioSyncRecord.SetDeleted(true); 
		vector<int>::const_iterator i(find(fAllRecordIDs.begin(), fAllRecordIDs.end(), recordID));
		if (i != fAllRecordIDs.end()) {
			vector<string> fields(fFileMakerDatabase->GetRecord(recordID));
			if (!CDataSource::DigestFieldsMatch(fields, info.fDigest))
				ioSyncRecord.SetModified(true); 
			for (int fieldIndex = 0; fieldIndex < fFieldIDs.size(); fieldIndex += 1) {
				string field = fields.at(fieldIndex);
				ioSyncRecord.SetValue(FieldIDFactory::GetID(fFieldIDs.at(fieldIndex)), field);
			}
		}
		
		mCurrentIndex+=1;
	} else {
		int totalLength;
		do {
			++mCurrentIndex;
#ifdef DEMO
			if (mCurrentIndex > DEMO_RECORD_LIMIT)
				return false;
#endif		
			while (IsDeletedByIndex(mCurrentIndex-1))
				++mCurrentIndex;

			if (!this->GetRecordByIndex(mCurrentIndex, inSyncType, ioSyncRecord)) {
				fDoingDeleted = true;
				mCurrentIndex = 0;
				return NextRecord(inSyncType, ioSyncRecord);
			}
			totalLength = 0;
			for (int fieldIndex = 0; fieldIndex < fFieldIDs.size(); fieldIndex += 1) {
				string s(ioSyncRecord.GetValue(FieldIDFactory::GetID(fFieldIDs.at(fieldIndex))));
				totalLength += s.length();
			}
		} while (totalLength == 0 || (inSyncType == eFast && !ioSyncRecord.IsModified() && ioSyncRecord.GetRemoteRecordID()));
	}
	return true;
}

Boolean CFileMakerRecordIterator::NextCategory( CCategory& ioCategory )//Override
{
	return false;
}

void CFileMakerRecordIterator::AddCategory( CCategory& )
{
}

void CFileMakerRecordIterator::SetCategoryName( CCategory&, const char* )
{
}

void CFileMakerRecordIterator::SetCategoryID( CCategory&, UInt8 )
{
}

Boolean CFileMakerRecordIterator::UsesCategories()
{
	return false;
}

Boolean CFileMakerRecordIterator::GetAppInfo(CDbGenInfo* ioAppInfo)
{
	Throw_(paramErr);
	return false;
}

bool
CFileMakerRecordIterator::IsDeletedByIndex(long inIndex)
{
	if (inIndex >= fRecordIDs.size())
		return false;
		
	UInt32 fmID = fRecordIDs.at(inIndex);
	return fRecordsToDelete[fmID] || fDeletedByPilot[fmID];
}
void
CFileMakerRecordIterator::LogDatabaseError()
{
	if (StAppleEvent::gLastErrorString.length() > 0) {
		::LogAddEntry(StAppleEvent::gLastErrorString.c_str(), slWarning, false);
		StAppleEvent::gLastErrorString.clear();
	}
}

void
CFileMakerRecordIterator::AssociateLocalWithRemote(int localID, int remoteID)
{
	CRecordMap::iterator i(fFMIDToRecInfo.find(localID));
	if (i == fFMIDToRecInfo.end()) {
		CRecordInfo info;
		info.fRecordID = remoteID;
		fFMIDToRecInfo[localID] = info;
	} else {
		i->second.fRecordID = remoteID;
	}
}

void
CFileMakerRecordIterator::UpdateLocalData(int localID, const vector<string>& fieldData)
{
	CRecordMap::iterator i(fFMIDToRecInfo.find(localID));
	if (i != fFMIDToRecInfo.end()) {
		i->second.fDigest = CDataSource::DigestFields(fieldData);
	}
}


void
CFileMakerRecordIterator::ForgetLocal(int localID)
{
	CRecordMap::iterator i(fFMIDToRecInfo.find(localID));
	if (i != fFMIDToRecInfo.end()) {
		(*i).second.fDeleted = true;
	}
}

void
CFileMakerRecordIterator::GetDatabaseInfo(CDatabaseInfo& outInfo)
{
	CFMFieldTyper typer(fFileMakerDatabase);
	outInfo = typer.GetDatabaseInfo();
}

bool
CFileMakerRecordIterator::IsReallyDeleted(int localID)
{
	return find(fAllRecordIDs.begin(), fAllRecordIDs.end(), localID) == fAllRecordIDs.end();
}
