/*
	File:		CRecordIterator.cp

	Contains:	xxx put contents here xxx

	Created by:	Chris LeCroy on Sun, Nov 16, 1997

	Copyright:	© 1997 by Now Software, Inc., all rights reserved.

	Change History (most recent first):

				00/00/00	---	file 	created

	To Do:
*/

#include "CRecordIterator.h"
#include "DebugOutput.h"

#include "CSyncRecord.h"


CRecordIterator::CRecordIterator(const CSyncProperties& inSyncProperties, short inDBIndex) :
	//mCurrentIndex(0),
	mSyncProperties(inSyncProperties)
{
	
}


Boolean CRecordIterator::OBSOLETE_CurrentRecord(eSyncTypes inSyncType, CSyncRecord& ioSyncRecord)
{
	//return this->GetRecordByIndex(mCurrentIndex, inSyncType, ioSyncRecord);
	return( false );
}

Boolean CRecordIterator::OBSOLETE_NextRecord(eSyncTypes inSyncType, CSyncRecord& ioSyncRecord)
{
	//return this->GetRecordByIndex(++mCurrentIndex, inSyncType, ioSyncRecord);
	return( false );
}


void CRecordIterator::OBSOLETE_Reset()
{
	//mCurrentIndex = 0;
}


void CRecordIterator::PreSync()
{
	
}



void CRecordIterator::PostSync(bool)
{
	
}

#pragma segment Main
void CRecordIterator::DebugStreamRecord(const CSyncRecord& inCSyncRecord)
{
}

#pragma segment Main
void CRecordIterator::UpdateRecordID(CSyncRecord& inRecord, UInt32 inNewRecordID)
{
	inRecord.SetRemoteRecordID(inNewRecordID);
}

UInt32
CRecordIterator::AddRecord(const CSyncRecord& inRecord)
{
	return WriteRecord(inRecord);
}
