/*
	File:		CArchiveIterator.cp

	Contains:	xxx put contents here xxx

	Created by:	Chris LeCroy on Sun, Nov 16, 1997

	Copyright:	© 1997 by Now Software, Inc., all rights reserved.

	Change History (most recent first):

				 00/00/00	---	file 	created

	To Do:
*/


#include "CArchiveIterator.h"
#include "CSyncRecord.h"



CArchiveIterator::CArchiveIterator(const CSyncProperties& inSyncProperties, short inDBIndex) :
	mCurrentIndex(0),
	CRecordIterator(inSyncProperties,inDBIndex)
{
}


CArchiveIterator::~CArchiveIterator()
{
}


void CArchiveIterator::ConvertToSyncRecord(CRawRecordInfo* inRecordData, CSyncRecord* outCSyncRecord) 
{
	printf("CArchiveIterator::ConvertToSyncRecord called");
	outCSyncRecord = nil;
}



void CArchiveIterator::ConvertFromSyncRecord(const CSyncRecord& inCSyncRecord, void* outRecordData) 
{
	printf("CArchiveIterator::ConvertFromSyncRecord called");
	outRecordData = nil;
}



Boolean CArchiveIterator::GetRecordByIndex(long inIndex, eSyncTypes inSyncType, CSyncRecord& ioSyncRecord)
{
	printf("CArchiveIterator::GetRecordByIndex called");
	return false;
}



Boolean CArchiveIterator::GetRecordByID(long inRecID, eSyncTypes inSyncType, CSyncRecord& ioSyncRecord)
{
	printf("CArchiveIterator::GetRecordByID called");
	return false;
}



UInt32 CArchiveIterator::WriteRecord(const CSyncRecord& inRecord) 
{
	void* recordData = nil;
	this->ConvertFromSyncRecord (inRecord, recordData);
	printf("CArchiveIterator::WriteRecord called");
	
	return 0;
}



void CArchiveIterator::DeleteRecord(const CSyncRecord& inRecord) 
{
	void* recordData = nil;
	this->ConvertFromSyncRecord (inRecord, recordData);
	printf("CArchiveIterator::DeleteRecord called");
}

Boolean CArchiveIterator::UsesCategories()
{
	return( true );
}


#pragma segment Main
Boolean CArchiveIterator::NextRecord(eSyncTypes inSyncType, CSyncRecord& ioSyncRecord)//Override
{
	return this->GetRecordByIndex(++mCurrentIndex, inSyncType, ioSyncRecord);
}

Boolean CArchiveIterator::NextCategory( CCategory& ioCategory )//Override
{
	return( false );
}

void CArchiveIterator::AddCategory( CCategory& inCategory )
{
}

void CArchiveIterator::SetCategoryName( CCategory& inCategory, const char* newName )
{
}

void CArchiveIterator::SetCategoryID( CCategory& inCategory, UInt8 newID )
{
}


