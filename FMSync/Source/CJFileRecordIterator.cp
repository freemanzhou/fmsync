#include "CJFileRecordIterator.h"

#include <StdLib.h>
#include <stddef.h>

#include <UMemoryMgr.h>

#include "CSynchronizer.h"
#include "ConverterErrors.h"
#include "ErrorCodes.h"
#include "SyncMgr.h"
#include "CSyncRecord.h"
#include "ToDoCommonHeader.h"
#include "StSharedBuffer.h"
#include "DebugOutput.h"
#include "CMyStr255.h"
#include "WriteJFile5.h"
#include "WriteJFilePro.h"
#include "ReadJFilePro.h"
#include "ReadJFile5.h"
#include "Utilities.h"
#include "Stringiness.h"
#include "CDatabaseInfo.h"
#include "FieldIDFactory.h"
#include "JFile5.h"

using JFile5::CWriteJFile5;

CJFileRecordIterator::CJFileRecordIterator(const CSyncProperties& inSyncProperties, int index, bool translateText, bool jFilePro) :
	CRecordIterator(inSyncProperties, index),
	mCurrentIndex(0),
	mLastIDRetrieved(0),
	mLastIndexRetrieved(0),
	mPilotDBHandle(0),
	fCreated(false),
	fJFilePro(jFilePro),
	fIgnorePalmRecords(false),
	fDeleteAfterSync(false),
	fDeleteBeforeSync(false),
	fNeedsSortOrSync(false),
	fTranslateText(translateText)
{
	if (jFilePro) {
		mType = CWriteJFilePro::kType;
		mCreator = CWriteJFilePro::kCreator;
	} else {
		mType = CWriteJFile5::kType;
		mCreator = CWriteJFile5::kCreator;
	}
}


CJFileRecordIterator::~CJFileRecordIterator()
{
	if (mPilotDBHandle) {
		int saveHandle = mPilotDBHandle;
		mPilotDBHandle = 0;
		ThrowIfErrorC_(::SyncCloseDB(saveHandle));
	}
}


Boolean CJFileRecordIterator::GetRecordByID(long inRecID, eSyncTypes inSyncType, CSyncRecord& ioSyncRecord)
{
	if (fIgnorePalmRecords)
		return false;
		
	if (inRecID == 0)
		return false;
		
	Boolean recordFound = false;
	long err = 0;
	CRawRecordInfo rawRecInfo = {};
	
	StUseSharedBuffer grabSharedBuffer(mSharedBuffer);

	rawRecInfo.m_FileHandle = mPilotDBHandle;
	rawRecInfo.m_TotalBytes = mSharedBuffer.GetBufferLen();
	
	rawRecInfo.m_pBytes = (Byte*)mSharedBuffer;	//ALLOC

	rawRecInfo.m_RecId = inRecID;
	err = ::SyncReadRecordById(rawRecInfo);
		
	if (err != SYNCERR_FILE_NOT_FOUND)
	{
		ThrowIfErrorC_(err);
		recordFound = true;
		//fIndexMap[inRecID] = rawRecInfo.m_RecIndex;
		this->ConvertToSyncRecord(&rawRecInfo, &ioSyncRecord);
	}
	
	return recordFound;
}


UInt32 CJFileRecordIterator::WriteRecord(const CSyncRecord& inRecord)
{	
	StUseSharedBuffer grabSharedBuffer(mSharedBuffer);

	ValidateData(inRecord);
	
	CRawRecordInfo rawRecord = {};
	
	rawRecord.m_TotalBytes = mSharedBuffer.GetBufferLen();
	rawRecord.m_pBytes = (Byte*)mSharedBuffer;	//ALLOC
	
	this->ConvertFromSyncRecord(inRecord, &rawRecord);
#if 0
	int recIndex = fIndexMap[inRecord.GetRemoteRecordID()];
	if (recIndex != 0) {
		rawRecord.m_RecIndex = recIndex;
	}
#endif
	
	//clear out any temporary attributes before writing the record
	rawRecord.m_Attribs &= ~kPilotAttrTempAttrsMask;

	int err = ::SyncWriteRec(rawRecord);
	ThrowIfErrorC_(err);
	
	fNeedsSortOrSync = true;
	
	return rawRecord.m_RecId;
}



void CJFileRecordIterator::DeleteRecord(const CSyncRecord& inRecord)
{
	StUseSharedBuffer grabSharedBuffer(mSharedBuffer);

	CRawRecordInfo rawRecord = {};
	
	rawRecord.m_TotalBytes = mSharedBuffer.GetBufferLen();
	rawRecord.m_pBytes = (Byte*)mSharedBuffer;	//ALLOC
	
	this->ConvertFromSyncRecord(inRecord, &rawRecord);
	int errorCode = ::SyncDeleteRec(rawRecord);
	if (errorCode != SYNCERR_NOT_FOUND)
		ThrowIfErrorC_(errorCode);
}



void CJFileRecordIterator::ConvertFromSyncRecord(const CSyncRecord& inCSyncRecord, void* outRecordData)
{
	CRawRecordInfo* rawRecordInfoPtr = (CRawRecordInfo*)outRecordData;
	rawRecordInfoPtr->m_Attribs = 0;

	vector<string> fields;
	int fieldCount = fDesiredFieldIDs.size();
	for (int fieldIndex = 0; fieldIndex < fieldCount; fieldIndex += 1) {
		fields.push_back(inCSyncRecord.GetValue( FieldIDFactory::GetID(fDesiredFieldIDs[fieldIndex])));
	}
	LHandleStream stream;
	if (fJFilePro)
		CWriteJFilePro::WriteRecord(&stream, fields, fTranslateText);
	else
		CWriteJFile5::WriteRecord(&stream, fields, fTranslateText, fDesiredRepeats);
	Handle h = stream.GetDataHandle();
	int recordLength = stream.GetLength();
	rawRecordInfoPtr->m_TotalBytes = recordLength;
	rawRecordInfoPtr->m_RecSize = recordLength;
	::BlockMoveData(*h, rawRecordInfoPtr->m_pBytes, recordLength);
	
	if (inCSyncRecord.IsModified())
		rawRecordInfoPtr->m_Attribs |= kPilotAttrModified;
		
	if (inCSyncRecord.IsPrivate())
		rawRecordInfoPtr->m_Attribs |= kPilotAttrPrivate;
		
	if (inCSyncRecord.IsArchived())
		rawRecordInfoPtr->m_Attribs |= kPilotAttrArchived;
		
	if (inCSyncRecord.IsDeleted())
		rawRecordInfoPtr->m_Attribs |= kPilotAttrDeleted;
		
	rawRecordInfoPtr->m_FileHandle = mPilotDBHandle;
	
	rawRecordInfoPtr->m_RecId = inCSyncRecord.GetRemoteRecordID();
	
	rawRecordInfoPtr->m_CatId = 0;
	
}




void CJFileRecordIterator::ConvertToSyncRecord(CRawRecordInfo* rawRecord, CSyncRecord* outCSyncRecord)
{
	if (rawRecord->m_RecSize > 0) {
		char* buffer = (char*)rawRecord->m_pBytes;
		LDataStream stream(buffer, rawRecord->m_RecSize);
		vector<string> fields;
		int fieldCount = fExistingFieldIDs.size();
		if (fJFilePro)
			CReadJFilePro::ReadRecord(&stream, fields, fieldCount, fTranslateText);
		else
			CReadJFile5::ReadRecord(&stream, fields, fieldCount, fTranslateText, fExistingRepeats);
		
		for (int fieldIndex = 0; fieldIndex < fieldCount; fieldIndex += 1) {
			string field = fields[fieldIndex];
			FMAE::FieldID fieldID(fExistingFieldIDs[fieldIndex]);
			UInt32 constructedID(FieldIDFactory::GetID(fieldID));
			outCSyncRecord->SetValue(constructedID, field);
		}
	}
		
	outCSyncRecord->SetAttributes(0);
	if (rawRecord->m_Attribs & kPilotAttrModified)
		outCSyncRecord->SetModified(true);
		
	if (rawRecord->m_Attribs & kPilotAttrPrivate)
		outCSyncRecord->SetPrivate(true);
		
	if (rawRecord->m_Attribs & kPilotAttrArchived)
		outCSyncRecord->SetArchived(true);
		
	if (rawRecord->m_Attribs & kPilotAttrDeleted)
		outCSyncRecord->SetDeleted(true);
		
	outCSyncRecord->SetRemoteRecordID(rawRecord->m_RecId);
	outCSyncRecord->SetLocalRecordID(0);
	
	outCSyncRecord->SetCategoryID( 0);
}


Boolean CJFileRecordIterator::GetAppInfo(CDbGenInfo*)
{
	Throw_(paramErr);
	return false;
}



Boolean CJFileRecordIterator::GetSortInfo(CDbGenInfo*)
{
	Throw_(paramErr);
	return false;
}



void CJFileRecordIterator::WriteAppInfo(const CDbGenInfo& inAppInfo)
{
	Assert_(inAppInfo.m_pBytes != nil);
	Assert_(inAppInfo.m_BytesRead == inAppInfo.m_TotalBytes);
	
	long err = ::SyncWriteDBAppInfoBlock(mPilotDBHandle, const_cast<CDbGenInfo&>(inAppInfo));
    ThrowIfErrorC_(err);
}



void CJFileRecordIterator::WriteSortInfo(const CDbGenInfo& inSortInfo)
{
}


void CJFileRecordIterator::PreSync()
{
	ThrowIf_(mFileName.length() == 0);
	
	long err = ::SyncOpenDB(mFileName.c_str(), 0, mPilotDBHandle);
	Boolean dbExists = (err == SYNCERR_NONE);
	
	if (err != SYNCERR_FILE_NOT_FOUND)
		ThrowIfErrorC_(err);

	if (!dbExists) {
		CDbCreateDB createDB = {};
		createDB.m_Creator = mCreator;
		createDB.m_Flags = (eDbFlags)0;
		strcpy(createDB.m_Name, mFileName.c_str());
		createDB.m_Type = mType;
		if (!fJFilePro)
			createDB.m_Version = DB_VERSION_50_FORMATTED | DB_VERSION_STRUCTLOCK;
		
		ThrowIfErrorC_(::SyncCreateDB(createDB));
		
		mPilotDBHandle = createDB.m_FileHandle;
		fCreated = true;
	}
#ifdef DEMO
	WORD recordCount;
	ThrowIfErrorC_(SyncGetDBRecordCount(mPilotDBHandle, recordCount));
	if (recordCount > DEMO_RECORD_LIMIT)
		Throw_(kDemoLimitExceeded);
#endif

	if (fDeleteBeforeSync)
		ThrowIfErrorC_(::SyncPurgeAllRecs(mPilotDBHandle));
}


bool
CJFileRecordIterator::CreatedDatabase() const
{
	return fCreated;
}


void CJFileRecordIterator::PostSync(bool wasCanceled)
{
	if (mPilotDBHandle) {
		if (!wasCanceled) {
			if (fDeleteAfterSync)
				ThrowIfErrorC_(::SyncPurgeAllRecs(mPilotDBHandle));
			else
				ThrowIfErrorC_(::SyncPurgeDeletedRecs(mPilotDBHandle));
			ThrowIfErrorC_(::SyncResetSyncFlags(mPilotDBHandle));
		}
		int saveHandle = mPilotDBHandle;
		mPilotDBHandle = 0;
		ThrowIfErrorC_(::SyncCloseDB(saveHandle));
	}
	
	if (wasCanceled && fCreated && mFileName.length() > 0) {
		fCreated = false;
		::SyncDeleteDB(mFileName.c_str(), 0);
	}
}


void CJFileRecordIterator::DebugStreamRecord(const CSyncRecord& inCSyncRecord)//Override
{
	//DebugStreamRecord(fExistingFieldIDs, inCSyncRecord);
}


void CJFileRecordIterator::DebugStreamRecord(const CFieldIDList& fieldIDs, const CSyncRecord& inCSyncRecord)
{
}

Handle CJFileRecordIterator::AllocateHandle()
{
	OSErr err = noErr;
	
	Handle h = TempNewHandle(0, &err);
	ThrowIfOSErr_(err);
	
	return h;
}

void CJFileRecordIterator::ReadPilotRecords(eSyncTypes inSyncType)
{
	long syncErr = 0;
	long pilotRecIndex = 0;
	CRawRecordInfo rawRecInfo = {};
	StUseSharedBuffer grabSharedBuffer(mSharedBuffer);
	
	rawRecInfo.m_FileHandle = mPilotDBHandle;
	rawRecInfo.m_TotalBytes = mSharedBuffer.GetBufferLen();
	rawRecInfo.m_pBytes = (Byte*)mSharedBuffer;

	do
	{
		if (inSyncType == eFast)
		{			
			syncErr = ::SyncReadNextModifiedRec(rawRecInfo);
		}
		else
		{
			rawRecInfo.m_RecIndex = pilotRecIndex;
			syncErr = ::SyncReadRecordByIndex(rawRecInfo);
			++pilotRecIndex;
		}
			
		if (syncErr != SYNCERR_FILE_NOT_FOUND && ( (rawRecInfo.m_Attribs & kPilotAttrDeleted) || rawRecInfo.m_RecSize > 0 ) )
		{
			CSyncRecord syncRecord;
			
			this->ConvertToSyncRecord(&rawRecInfo, &syncRecord);
			
			mPilotRecords.push_back(syncRecord);
			
		}
	} while (syncErr != SYNCERR_FILE_NOT_FOUND && ( (rawRecInfo.m_Attribs & kPilotAttrDeleted) || rawRecInfo.m_RecSize > 0 ) );
}

Boolean CJFileRecordIterator::GetRecordByIndex(long inIndex, eSyncTypes inSyncType, CSyncRecord& ioSyncRecord) 
{
	if (fIgnorePalmRecords)
		return false;
		
	Boolean recordFound = false;

	if (mPilotRecords.size() == 0)
	{
		this->ReadPilotRecords(inSyncType);
	}

	if (inIndex >= 1 && inIndex <= mPilotRecords.size()) {
		ioSyncRecord = mPilotRecords[inIndex-1];
		recordFound = true;
	}
		
#ifdef _DEBUG
	if (recordFound)
	{
		this->DebugStreamRecord(fExistingFieldIDs, ioSyncRecord);
	}
#endif	//_DEBUG
	
	return recordFound;
}

Boolean CJFileRecordIterator::NextRecord(eSyncTypes inSyncType, CSyncRecord& ioSyncRecord)//Override
{
	return this->GetRecordByIndex(++mCurrentIndex, inSyncType, ioSyncRecord);
}

Boolean CJFileRecordIterator::NextCategory( CCategory& ioCategory )//Override
{
	return( false );
}

void CJFileRecordIterator::AddCategory( CCategory& inCategory )
{
}

void CJFileRecordIterator::SetCategoryName( CCategory& inCategory, const char* newName )
{
}

void CJFileRecordIterator::SetCategoryID( CCategory& inCategory, UInt8 newID )
{
}

Boolean CJFileRecordIterator::UsesCategories()
{
	return false;
}

void
CJFileRecordIterator::GetDatabaseInfo(CDatabaseInfo& outInfo)
{
	DebugOutput::Output( "CJFileRecordIterator::GetDatabaseInfo()" );
	const int kMaxAppInfoBuffer = 8192;
	ThrowIf_(fExistingFieldIDs.size() == 0);
	CDatabaseInfo info;
	StPointerBlock appInfoBuffer(kMaxAppInfoBuffer);
	CDbGenInfo appInfo = {};
	appInfo.m_pBytes = (BYTE*)appInfoBuffer.Get();
	appInfo.m_TotalBytes = kMaxAppInfoBuffer;
	long err = ::SyncReadDBAppInfoBlock(mPilotDBHandle, appInfo);
	if (err != SYNCERR_NOT_FOUND) {
		ThrowIfErrorC_(err);
		if (fJFilePro)
			CReadJFilePro::ExtractFromAppInfo(appInfoBuffer.Get(), fTranslateText, fExistingFieldIDs, info);
		else
			CReadJFile5::ExtractFromAppInfo(appInfoBuffer.Get(), appInfo.m_BytesRead, fTranslateText, fExistingFieldIDs, fExistingRepeats, info);
	}
	outInfo = info;
}

void
CJFileRecordIterator::SetDatabaseInfo(const CDatabaseInfo& info, const vector<int>& repeats)
{
	LHandleStream stream;
	if(fJFilePro)
		CWriteJFilePro::WriteAppInfoBlock(&stream, fDesiredFieldIDs, info, fTranslateText);
	else
		CWriteJFile5::WriteAppInfoBlock(&stream, fDesiredFieldIDs, info, repeats, fTranslateText, fNeedsSortOrSync);
	Handle h = stream.GetDataHandle();
	HLock(h);
	int aiLength = stream.GetLength();
	CDbGenInfo appInfo = {};
	appInfo.m_pBytes = (BYTE*)*h;
	appInfo.m_TotalBytes = aiLength;
	appInfo.m_BytesRead = aiLength;
	
	WriteAppInfo(appInfo);
}

void
CJFileRecordIterator::SetExistingFieldInfo(const CFieldIDList& fids, const vector<int>& v)
{
	fExistingFieldIDs = fids;
	fExistingRepeats = v;
}

void
CJFileRecordIterator::SetDesiredFieldInfo(const CFieldIDList& fids, const vector<int>& v)
{
	fDesiredFieldIDs = fids;
	fDesiredRepeats = v;
}

void
CJFileRecordIterator::ValidateData(const CSyncRecord& localData)
{
	CRawRecordInfo rawRecord = {};
	
	rawRecord.m_TotalBytes = mSharedBuffer.GetBufferLen();
	rawRecord.m_pBytes = (Byte*)mSharedBuffer;	//ALLOC
	
	this->ConvertFromSyncRecord(localData, &rawRecord);
}
