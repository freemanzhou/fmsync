/*
	File:		CPilotRecordIterator.cp

	Contains:	xxx put contents here xxx

	Created by:	Chris LeCroy on Thu, Nov 6, 1997

	Copyright:	© 1997 by Now Software, Inc., all rights reserved.

	Change History (most recent first):

				 00/00/00	---	file 	created

	To Do:
*/


#include "CPilotRecordIterator.h"

#include <StdLib.h>
#include <stddef.h>

#include <UMemoryMgr.h>

#include "SyncMgr.h"
#include "CSyncRecord.h"
#include "ToDoCommonHeader.h"
#include "StSharedBuffer.h"
#include "UDebugOut.h"
#include "CMyStr255.h"


CPilotRecordIterator::CPilotRecordIterator(const CSyncProperties& inSyncProperties, short inDBIndex) :
	CRecordIterator(inSyncProperties,inDBIndex),
	mCurrentIndex(0),
	mCurrentCategory(0),
	mCategoryInfo(nil),
	mPilotRecords(CPilotRecordIterator::AllocateHandle(), nil, false),
	mLastIDRetrieved(0),
	mLastIndexRetrieved(0)
{
	CDbGenInfo emptyInfoRec = {};
	
	mAppInfo = emptyInfoRec;
	mSortInfo = emptyInfoRec;
	
	mCDbList = *(inSyncProperties.m_RemoteDbList[inDBIndex]);
}


CPilotRecordIterator::~CPilotRecordIterator()
{
//	delete mPilotRecords contents;
	CSyncRecordArrayIterator iter(mPilotRecords);
	CSyncRecord* syncRecord;
	
	while (iter.Next(syncRecord))
	{
		mPilotRecords.Remove(syncRecord);
		delete syncRecord;
	}
}


Boolean CPilotRecordIterator::OBSOLETE_GetRecordByIndex(long inIndex, eSyncTypes inSyncType, CSyncRecord& ioSyncRecord)
{
	Boolean recordFound = false;
	long err = 0;
	CRawRecordInfo rawRecInfo = {};
	
	StUseSharedBuffer grabSharedBuffer(mSharedBuffer);

	--inIndex;	//pilot record indexes are zero-based, our API is 1-based

	rawRecInfo.m_FileHandle = mPilotDBHandle;
	rawRecInfo.m_TotalBytes = mSharedBuffer.GetBufferLen();
	
	rawRecInfo.m_pBytes = (Byte*)mSharedBuffer;

	if (inSyncType == eFast)
	{
		rawRecInfo.m_RecId = mLastIDRetrieved;
		rawRecInfo.m_RecIndex = mLastIndexRetrieved;
		
		err = ::SyncReadNextModifiedRec(rawRecInfo);
		
		mLastIDRetrieved = rawRecInfo.m_RecId;
		mLastIndexRetrieved = rawRecInfo.m_RecIndex;
		
		DEBUG_OUT_DEF("PILOT::GetRecordByIndex(eFast): ");
	}
	else
	{
		rawRecInfo.m_RecIndex = inIndex;
		err = ::SyncReadRecordByIndex(rawRecInfo);
		DEBUG_OUT_DEF("PILOT::GetRecordByIndex(eSlow): ");
	}
		
	if (err != SYNCERR_FILE_NOT_FOUND)
	{
		ThrowIfError_(err);
		recordFound = true;
		this->ConvertToSyncRecord(&rawRecInfo, &ioSyncRecord);
	}
	
#ifdef _DEBUG
	if (recordFound)
	{
		DEBUG_OUT_NO_TS_DEF("ToDoItem = ");
		this->DebugStreamRecord(ioSyncRecord);
		DEBUG_OUT_NO_TS_DEF("\r");
	}
	else
	{
		DEBUG_OUT_NO_TS_DEF("ToDoItem = <not found>" );
		DEBUG_OUT_NO_TS_DEF("\r");
	}
#endif	//_DEBUG
	
	return recordFound;
}


Boolean CPilotRecordIterator::GetRecordByID(long inRecID, eSyncTypes inSyncType, CSyncRecord& ioSyncRecord)
{
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
		ThrowIfError_(err);
		recordFound = true;
		this->ConvertToSyncRecord(&rawRecInfo, &ioSyncRecord);
	}
	
#ifdef _DEBUG
	if (recordFound)
	{
		DEBUG_OUT_DEF("PILOT::GetRecordByID: ToDoItem = ");
		this->DebugStreamRecord(ioSyncRecord);
		DEBUG_OUT_DEF("\r");
	
		if (&ioSyncRecord == nil)
		{
			this->TimeOperations (rawRecInfo, ioSyncRecord);
		}
	}
	else
	{
		DEBUG_OUT_DEF("PILOT::GetRecordByID: ToDoItem = <not found>");
		DEBUG_OUT_DEF(",");
	}
#endif	//_DEBUG
	
	return recordFound;
}


UInt32 CPilotRecordIterator::WriteRecord(const CSyncRecord& inRecord)
{	
#ifdef _DEBUG
	DataType outDataType;
	short outDataSize;
	DEBUG_OUT_DEF( "PILOT::WriteRecord toDoItem = " );
	DEBUG_OUT_DEF( inRecord.GetRecordID() );
	DEBUG_OUT_DEF( "-" );
	DEBUG_OUT_DEF( (char*)inRecord.GetValuePtr(kDescriptionFieldID, &outDataType, &outDataSize) );
	DEBUG_OUT_DEF( ", " );
	DEBUG_OUT_DEF( (char*)inRecord.GetValuePtr(kNoteFieldID, &outDataType, &outDataSize) );
	DEBUG_OUT_DEF( "\r" );
#endif

	StUseSharedBuffer grabSharedBuffer(mSharedBuffer);

	CRawRecordInfo rawRecord = {};
	
	rawRecord.m_TotalBytes = mSharedBuffer.GetBufferLen();
	rawRecord.m_pBytes = (Byte*)mSharedBuffer;	//ALLOC
	
	this->ConvertFromSyncRecord(inRecord, &rawRecord);
	
	//clear out any temporary attributes before writing the record
	rawRecord.m_Attribs &= ~kPilotAttrTempAttrsMask;

	ThrowIfError_(::SyncWriteRec(rawRecord));
	
	return rawRecord.m_RecId;
}



void CPilotRecordIterator::DeleteRecord(const CSyncRecord& inRecord)
{
#ifdef _DEBUG
	DataType outDataType;
	short outDataSize;
	DEBUG_OUT_DEF( "PILOT::DeleteRecord toDoItem = " );
	DEBUG_OUT_DEF( inRecord.GetRecordID() );
	DEBUG_OUT_DEF( "-" );
	DEBUG_OUT_DEF( (char*)inRecord.GetValuePtr(kDescriptionFieldID, &outDataType, &outDataSize) );
	DEBUG_OUT_DEF( ", " );
	DEBUG_OUT_DEF( (char*)inRecord.GetValuePtr(kNoteFieldID, &outDataType, &outDataSize) );
	DEBUG_OUT_DEF( "\r" );
#endif

	StUseSharedBuffer grabSharedBuffer(mSharedBuffer);

	CRawRecordInfo rawRecord = {};
	
	rawRecord.m_TotalBytes = mSharedBuffer.GetBufferLen();
	rawRecord.m_pBytes = (Byte*)mSharedBuffer;	//ALLOC
	
	this->ConvertFromSyncRecord(inRecord, &rawRecord);

    // determine if res or resource
    if (true /*!this->IsResourceDB()*/)
	{
      	ThrowIfError_(::SyncDeleteRec(rawRecord));
    }
	else
	{
        ThrowIfError_(::SyncDeleteResourceRec(rawRecord));
    }

}



void CPilotRecordIterator::ConvertFromSyncRecord(const CSyncRecord& inCSyncRecord, void* outRecordData)
{
	char* buffer = nil;
	DataType dataType = kWildcardData;
	short dataSize = 0;
	CRawRecordInfo* rawRecordInfoPtr = (CRawRecordInfo*)outRecordData;
	rawRecordInfoPtr->m_Attribs = 0;
	rawRecordInfoPtr->m_RecSize = 0;
	
	//verify that we got passed a good raw record ptr
	Assert_(rawRecordInfoPtr->m_pBytes != nil);
	Assert_(rawRecordInfoPtr->m_TotalBytes != 0);
	Assert_(rawRecordInfoPtr->m_RecIndex == 0);

	UInt16 fieldIndex=0;
	{	//add the due date to the raw record
		LongDateTime dueDate;
		Boolean gotValue = inCSyncRecord.GetValue( kDueDateFieldID, &dataType, &dueDate, sizeof( dueDate ), &dataSize );
		Assert_( gotValue );
		Assert_( dataType == kInteger64Data );

		LongDateRec dueDateRec;
		LongSecondsToDate( &dueDate, &dueDateRec);

		short rawDate = 0x0000;
		rawDate |= ( dueDateRec.ld.year - kPilotBaseYear ) << kYearShiftFactor;
		rawDate |= dueDateRec.ld.month << kMonthShiftFactor;
		rawDate |= dueDateRec.ld.day;
		this->AddFieldToPilotRecord( fieldIndex++, (Ptr)&rawDate, sizeof( rawDate ), rawRecordInfoPtr );
	}
	
	{	//priority and completed fields
		UInt8 priority;
		Boolean gotValue = inCSyncRecord.GetValue( kPriorityFieldID, &dataType, &priority, sizeof( priority ), &dataSize );
		Assert_( gotValue );
		Assert_( dataType == kInteger8Data );

		Boolean completed;
		gotValue = inCSyncRecord.GetValue( kCompletedFieldID, &dataType, &completed, sizeof( completed ), &dataSize );
		Assert_( gotValue );
		Assert_( dataType == kBooleanData );

		//completed is high bit of priority field
		priority |= completed << kCompletedShiftFactor;
		this->AddFieldToPilotRecord( fieldIndex++, (Ptr)&priority, sizeof( priority ), rawRecordInfoPtr );
	}
	
	{	//description field
		char* description = inCSyncRecord.GetCStringPtr( kDescriptionFieldID );
		this->AddStringFieldToPilotRecord( fieldIndex++, description, (CRawRecordInfo*)outRecordData);
	}
	
	{	//note field
		char* description = inCSyncRecord.GetCStringPtr( kNoteFieldID );
		this->AddStringFieldToPilotRecord( fieldIndex++, description, (CRawRecordInfo*)outRecordData);
	}
	
	
	if (inCSyncRecord.IsModified())
		rawRecordInfoPtr->m_Attribs |= kPilotAttrModified;
		
	if (inCSyncRecord.IsPrivate())
		rawRecordInfoPtr->m_Attribs |= kPilotAttrPrivate;
		
	if (inCSyncRecord.IsArchived())
		rawRecordInfoPtr->m_Attribs |= kPilotAttrArchived;
		
	if (inCSyncRecord.IsDeleted())
		rawRecordInfoPtr->m_Attribs |= kPilotAttrDeleted;
		
	rawRecordInfoPtr->m_FileHandle = mPilotDBHandle;
	
	rawRecordInfoPtr->m_RecId = inCSyncRecord.GetRecordID();
	
	rawRecordInfoPtr->m_CatId = this->ConvertCategoryIDToIndex( inCSyncRecord.GetCategoryID() );
	
}




void CPilotRecordIterator::ConvertToSyncRecord(const void* inRecordData, CSyncRecord* outCSyncRecord)
{
	CRawRecordInfo* rawRecord = (CRawRecordInfo*)inRecordData;

	char* buffer = (char*)rawRecord->m_pBytes;
	{	//due date
		UInt16 rawDate = *(UInt16*)buffer;
		LongDateRec dueDateRec={};
		dueDateRec.ld.year = ( ( rawDate & kRawDateYearMask ) >> kYearShiftFactor ) + kPilotBaseYear;
		dueDateRec.ld.month = ( rawDate & kRawDateMonthMask ) >> kMonthShiftFactor;
		dueDateRec.ld.day = rawDate & kRawDateDayMask;
		
		LongDateTime dueDate;
		LongDateToSeconds( &dueDateRec, &dueDate );
		
		outCSyncRecord->SetValue( kDueDateFieldID, kInteger64Data, &dueDate, sizeof(dueDate) );

		buffer += sizeof( UInt16 );
	}
	
	{	//priority and completed fields
		UInt8 priority = *(UInt8*)buffer;

		//get completed bit out and then strip it out of the priority
		//variable so we don't add 2^7 to it by mistake!
		Boolean completed = (Boolean)( ( priority & kCompletedBitMask ) >> kCompletedShiftFactor );
		priority &= ~kCompletedBitMask;

		outCSyncRecord->SetValue( kPriorityFieldID, kInteger8Data, &priority, sizeof(priority) );

		outCSyncRecord->SetValue( kCompletedFieldID, kBooleanData, &completed, sizeof(completed) );

		buffer += sizeof( UInt8 );
	}
	
	{	//description field
		char* description = buffer;
		UInt32 len = strlen( description );
		
		outCSyncRecord->SetValue( kDescriptionFieldID, description );
		
		buffer += len+1;
	}
		
	{	//description field
		char* note = buffer;
		UInt32 len = strlen( note );
		
		outCSyncRecord->SetValue( kNoteFieldID, note );
		
		buffer += len+1;
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
		
	outCSyncRecord->SetRecordID(rawRecord->m_RecId);
	
	outCSyncRecord->SetCategoryID( this->ConvertCategoryIndexToID( rawRecord->m_CatId ) );
}


void CPilotRecordIterator::AddFieldToPilotRecord(UInt32 fieldNum, Ptr fieldData, short fieldLen, CRawRecordInfo* rawRecordInfo)
{
	if (fieldLen != 0)
	{
		/*ToDoPackedDBRecord*	todoHeader = (ToDoPackedDBRecord*)rawRecordInfo->m_pBytes;
		Ptr pilotRecordBuffer = &todoHeader->firstField + (rawRecordInfo->m_RecSize - offsetof(ToDoPackedDBRecord, firstField));*/
	
		//todoHeader->flags.allBits += fieldNum;
		//SetBitMacro(todoHeader->flags.allBits, fieldNum);
	
		(void)memcpy(rawRecordInfo->m_pBytes+rawRecordInfo->m_RecSize, fieldData, fieldLen);
	
		rawRecordInfo->m_RecSize += fieldLen;
		
		Assert_(rawRecordInfo->m_RecSize <= rawRecordInfo->m_TotalBytes);
	}
}



void CPilotRecordIterator::AddStringFieldToPilotRecord(UInt32 fieldNum,  char* inString,  CRawRecordInfo* rawRecordInfo)
{
	short len = strlen(inString);
	
	if (len > 0)
	{
		this->AddFieldToPilotRecord(fieldNum,  inString, len+1, rawRecordInfo);
	}
}


Boolean CPilotRecordIterator::GetAppInfo(CDbGenInfo* ioAppInfo)
{
	if (mAppInfo.m_pBytes == nil)
	{
		StUseSharedBuffer grabSharedBuffer(mSharedBuffer);
	
	    // Allocate storage for app/sort info blocks
		mAppInfo.m_pBytes = (BYTE*)mSharedBuffer;
		mAppInfo.m_TotalBytes = mSharedBuffer.GetBufferLen();
	
	    // Read the AppInfo block
		long err = ::SyncReadDBAppInfoBlock(mPilotDBHandle, mAppInfo);
		if (err == SYNCERR_NOT_FOUND)
		{
			return false;
		}
		else
		{
	    	ThrowIfError_(err);
		}
		
		ThrowIfNil_(mAppInfo.m_pBytes = (Byte*)malloc(mAppInfo.m_BytesRead));
		(void)memmove(mAppInfo.m_pBytes, mSharedBuffer, mAppInfo.m_BytesRead);
		
		mAppInfo.m_TotalBytes = mAppInfo.m_BytesRead;
	}
	
	*ioAppInfo = mAppInfo;
	
	mCategoryInfo = (CPilotCategoryInfo*)mAppInfo.m_pBytes;
	
	return true;
}



Boolean CPilotRecordIterator::GetSortInfo(CDbGenInfo* ioSortInfo)
{
	if (mSortInfo.m_pBytes == nil)
	{
		StUseSharedBuffer grabSharedBuffer(mSharedBuffer);
	
	    // Allocate storage for app/sort info blocks
		mSortInfo.m_pBytes = (BYTE*)mSharedBuffer;
		mSortInfo.m_TotalBytes = mSharedBuffer.GetBufferLen();
	
	    // Read the SortInfo block
		long err = ::SyncReadDBSortInfoBlock(mPilotDBHandle, mSortInfo);
		if (err == SYNCERR_NOT_FOUND)
		{
			return false;
		}
		else
		{
	    	ThrowIfError_(err);
		}
			
		ThrowIfNil_(mSortInfo.m_pBytes = (Byte*)malloc(mSortInfo.m_BytesRead));
		(void)memmove(mSortInfo.m_pBytes, mSharedBuffer, mSortInfo.m_BytesRead);
		
		mSortInfo.m_TotalBytes = mSortInfo.m_BytesRead;
	}
	
	*ioSortInfo = mSortInfo;
	
	return true;
}



void CPilotRecordIterator::WriteAppInfo(const CDbGenInfo& inAppInfo)
{
	Assert_(inAppInfo.m_pBytes != nil);
	
	//clear out the names changed bits in the category info
	Assert_( mCategoryInfo != nil );
	mCategoryInfo->namesChangedBits = 0x0000;

    ThrowIfError_(::SyncWriteDBAppInfoBlock(mPilotDBHandle, (CDbGenInfo&)inAppInfo));
	
	mAppInfo = inAppInfo;
}



void CPilotRecordIterator::WriteSortInfo(const CDbGenInfo& inSortInfo)
{
	Assert_(inSortInfo.m_pBytes != nil);
	
	if (inSortInfo.m_pBytes == nil)
	{
	    ThrowIfError_(::SyncWriteDBSortInfoBlock(mPilotDBHandle, (CDbGenInfo&)inSortInfo));
	}
	
	mSortInfo = inSortInfo;
}


void CPilotRecordIterator::PreSync()
{
	//due to limitations in some of the older Palm OS versions, it will
	//probably be necessary to read all records at the beginning of the
	//HotSync.

	long err = ::SyncOpenDB(mCDbList.m_Name, 0, mPilotDBHandle);
	
	if (err != SYNCERR_FILE_NOT_FOUND)
		ThrowIfError_(err);
	else
	{
		CDbCreateDB createDB = {};
		createDB.m_Creator = mCDbList.m_Creator;
		createDB.m_Flags = (eDbFlags)mCDbList.m_DbFlags;
		strcpy(createDB.m_Name, mCDbList.m_Name);
		createDB.m_Type = mCDbList.m_DbType;
		
		ThrowIfError_(::SyncCreateDB(createDB));
		
		mPilotDBHandle = createDB.m_FileHandle;
	}

}




void CPilotRecordIterator::PostSync()
{
	//write the app info to the pilot so category changes make sense
	this->WriteAppInfo( mAppInfo );	

	ThrowIfError_(::SyncPurgeDeletedRecs(mPilotDBHandle));
	ThrowIfError_(::SyncResetSyncFlags(mPilotDBHandle));
	ThrowIfError_(::SyncCloseDB(mPilotDBHandle));
}


void CPilotRecordIterator::TimeOperations( CRawRecordInfo& inRawRecord, const CSyncRecord& inCSyncRecord)
{		

	CRawRecordInfo workRawRecord = {};
	CSyncRecord workSyncRecord;

	UInt32 startTicks = 0;
	short i = 0;
	
	//copy CRawRecordInfos
	startTicks = ::TickCount();
	for (i = 0; i < 10000; i++)
	{
		workRawRecord = inRawRecord;
	}
	printf("Time to copy %d CRawRecordInfos is %d ticks", i, ::TickCount() - startTicks);
	
	//ConvertToSyncRecord
	startTicks = ::TickCount();
	for (i = 0; i < 10000; i++)
	{
		this->ConvertToSyncRecord(&inRawRecord, &workSyncRecord);
	}
	printf("Time to ConvertToSyncRecord %d records is %d ticks", i, ::TickCount() - startTicks);
	
	//ConvertFromSyncRecord
	startTicks = ::TickCount();
	for (i = 0; i < 10000; i++)
	{
		this->ConvertFromSyncRecord(inCSyncRecord, &inRawRecord);
	}
	printf("Time to ConvertFromSyncRecord %d records is %d ticks", i, ::TickCount() - startTicks);
	
	//stack
	startTicks = ::TickCount();
	for (i = 0; i < 10000; i++)
	{
		CSyncRecord syncRecord;
	}
	printf("Time to stack %d records is %d ticks", i, ::TickCount() - startTicks);
	
	//stack/convert
	startTicks = ::TickCount();
	for (i = 0; i < 10000; i++)
	{
		CSyncRecord syncRecord;
		this->ConvertToSyncRecord(&inRawRecord, &syncRecord);
	}
	printf("Time to stack/ConvertToSyncRecord %d records is %d ticks", i, ::TickCount() - startTicks);
	
	
	//allocate/delete
	startTicks = ::TickCount();
	for (i = 0; i < 10000; i++)
	{
		CSyncRecord* syncRecord = new CSyncRecord;
		delete syncRecord;
	}
	printf("Time to allocate/delete %d records is %d ticks", i, ::TickCount() - startTicks);
	
	//allocate/delete/convert
	startTicks = ::TickCount();
	for (i = 0; i < 10000; i++)
	{
		CSyncRecord* syncRecord = new CSyncRecord;
		this->ConvertToSyncRecord(&inRawRecord, &workSyncRecord);
		delete syncRecord;
	}
	printf("Time to allocate/delete/ConvertToSyncRecord %d records is %d ticks", i, ::TickCount() - startTicks);
	
	Debugger();
}

#pragma segment Main
void CPilotRecordIterator::DebugStreamRecord(const CSyncRecord& inCSyncRecord)//Override
{
	if (inCSyncRecord.FieldIDExists(kRecordIDField))
	{
		DEBUG_OUT_NO_TS_DEF( inCSyncRecord.GetRecordID() );
		DEBUG_OUT_NO_TS_DEF( "-" );
		if (inCSyncRecord.GetRecordID() != 0)
		{
			DataType outDataType;
			short outDataSize;

			DEBUG_OUT_NO_TS_DEF( (char*)inCSyncRecord.GetValuePtr(kDescriptionFieldID, &outDataType, &outDataSize) );
			DEBUG_OUT_NO_TS_DEF( "," );
			DEBUG_OUT_NO_TS_DEF( (char*)inCSyncRecord.GetValuePtr(kNoteFieldID, &outDataType, &outDataSize) );
		}
	}
	else
	{
		DEBUG_OUT_NO_TS_DEF( "<NO RECORD>" );
	}
}





#pragma segment Main
Handle CPilotRecordIterator::AllocateHandle()
{
	OSErr err = noErr;
	
	Handle h = TempNewHandle(0, &err);
	ThrowIfOSErr_(err);
	
	return h;
}


#pragma segment Main
void CPilotRecordIterator::ReadPilotRecords(eSyncTypes inSyncType)
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
			CSyncRecord* syncRecord = new CSyncRecord;	//move to stack-allocations at some point...
			
			ThrowIfError_(syncErr);
			this->ConvertToSyncRecord(&rawRecInfo, syncRecord);
			
			(void)mPilotRecords.InsertItemsAt (1, LArray::index_Last, syncRecord);
			
		}
	} while (syncErr != SYNCERR_FILE_NOT_FOUND && ( (rawRecInfo.m_Attribs & kPilotAttrDeleted) || rawRecInfo.m_RecSize > 0 ) );
}


#pragma segment Main
Boolean CPilotRecordIterator::GetRecordByIndex(long inIndex, eSyncTypes inSyncType, CSyncRecord& ioSyncRecord) 
{
	Boolean recordFound = false;

	if (mPilotRecords.GetCount() == 0)
	{
		this->ReadPilotRecords(inSyncType);
	}

	if (inSyncType == eFast)
	{
		DEBUG_OUT_DEF( "PILOT::GetRecordByIndex(eFast): ");
	}
	else
	{
		DEBUG_OUT_DEF( "PILOT::GetRecordByIndex(eSlow): ");
	}
	
	CSyncRecord* syncRecord = nil;
	recordFound = mPilotRecords.FetchItemAt(inIndex, syncRecord);
	if (recordFound)
	{
		ThrowIfNil_(syncRecord);
		ioSyncRecord = *syncRecord;	//return a copy
	}
	
#ifdef _DEBUG
	if (recordFound)
	{
		DEBUG_OUT_NO_TS_DEF("ToDoItem = ");
		this->DebugStreamRecord(ioSyncRecord);
		DEBUG_OUT_NO_TS_DEF( "\r" );
	}
	else
	{
		DEBUG_OUT_NO_TS_DEF("ToDoItem = <not found>" );
		DEBUG_OUT_NO_TS_DEF( "\r" );
	}
#endif	//_DEBUG
	
	return recordFound;
}

UInt8 CPilotRecordIterator::ConvertCategoryIndexToID( UInt16 inIndex )
{
	//make sure this category exists
	Assert_( mCategoryInfo != nil );
	if( mCategoryInfo->catNames[inIndex][0] == '\0' )
	{
		CMyStr255 errorString( kSTRx_ConduitErrors, kstr_CorruptedPilotDB );
		char buffer[256];
		::LogAddEntry( errorString.ToCStr( buffer, sizeof( buffer ) ), slText, false);
		Throw_( SYNCERR_UNKNOWN );
	}

	return( mCategoryInfo->catIDs[inIndex] );
}

UInt16 CPilotRecordIterator::ConvertCategoryIDToIndex( UInt8 inID )
{
	Assert_( mCategoryInfo != nil );
	
	UInt16 i;
	Boolean foundID = false;
	for( i=0; !foundID && mCategoryInfo->catNames[i][0] != '\0'; i++ )
		foundID = mCategoryInfo->catIDs[i] == inID;
	i--;
	
	Assert_( foundID );
	return( i );
}


#pragma segment Main
Boolean CPilotRecordIterator::NextRecord(eSyncTypes inSyncType, CSyncRecord& ioSyncRecord)//Override
{
	return this->GetRecordByIndex(++mCurrentIndex, inSyncType, ioSyncRecord);
}

Boolean CPilotRecordIterator::NextCategory( CCategory& ioCategory )//Override
{
	//if the category info isn't filled in yet, then call GetAppInfo to fill it in
	if( !mCategoryInfo )
	{
		CDbGenInfo dummy;
		if( !this->GetAppInfo( &dummy ) )
			return( false );
	}
	
	//if we're at the end of the array, return the counter to 0 and return false
	if( mCurrentCategory >= CCategory::kMaxNumPilotCategories || mCategoryInfo->catNames[mCurrentCategory][0] == '\0' )
	{
		mCurrentCategory = 0;
		return( false );
	}
		
	//copy category stuff into the io parameter
	::strcpy( ioCategory.catName, mCategoryInfo->catNames[mCurrentCategory] );
	ioCategory.catID = mCategoryInfo->catIDs[mCurrentCategory];
	ioCategory.hasChanged = ( mCategoryInfo->namesChangedBits & ( 1 << mCurrentCategory ) ) != 0;

	//increment current category
	mCurrentCategory++;
	
	return( true );
}

void CPilotRecordIterator::AddCategory( CCategory& inCategory )
{
	Assert_( mCategoryInfo != nil );

	short newIndex;
	Boolean foundOpen=false;
	for( newIndex=0; newIndex<CCategory::kMaxNumPilotCategories && !foundOpen; newIndex++ )
		foundOpen = mCategoryInfo->catNames[newIndex][0] == '\0';
	Assert_( foundOpen );
	newIndex--;
	
	::strcpy( mCategoryInfo->catNames[newIndex], inCategory.catName );
	mCategoryInfo->catIDs[newIndex] = inCategory.catID;

	//mark the name not changed in the bit mask
	mCategoryInfo->namesChangedBits &= ~( 1 << newIndex );
}

void CPilotRecordIterator::SetCategoryName( CCategory& inCategory, const char* newName )
{
	Assert_( mCategoryInfo != nil );

	short i;
	Boolean foundCat=false;
	for( i=0; i<CCategory::kMaxNumPilotCategories && !foundCat; i++ )
		foundCat = mCategoryInfo->catIDs[i] == inCategory.catID;
	i--;
	Assert_( foundCat );

	::strcpy( mCategoryInfo->catNames[i], newName );
}

void CPilotRecordIterator::SetCategoryID( CCategory& inCategory, UInt8 newID )
{
	Assert_( mCategoryInfo != nil );

	short i;
	Boolean foundCat=false;
	for( i=0; i<CCategory::kMaxNumPilotCategories && !foundCat; i++ )
		foundCat = mCategoryInfo->catIDs[i] == inCategory.catID;
	i--;
	Assert_( foundCat );

	mCategoryInfo->catIDs[i] = newID;
}

Boolean CPilotRecordIterator::UsesCategories()
{
	//if the category info isn't filled in yet, then call GetAppInfo to fill it in
	if( !mCategoryInfo )
	{
		CDbGenInfo dummy;
		if( !this->GetAppInfo( &dummy ) )
			return( false );
	}
	return( mCategoryInfo->catNames[0][0] != '\0' );
}

