/*
	File:		CArchiveIterator.h

	Contains:	xxx put contents here xxx

	Created by:	Chris LeCroy on Sun, Nov 16, 1997

	Copyright:	© 1997 by Now Software, Inc., all rights reserved.

	Change History (most recent first):

				 00/00/00	---	file created

	To Do:
*/

#pragma once

#ifndef __CArchiveIterator__
#define __CArchiveIterator__

#ifndef __CRecordIterator__
#include "CRecordIterator.h"
#endif


class CArchiveIterator : public CRecordIterator
{
public:
	CArchiveIterator(const CSyncProperties& inSyncProperties, short inDBIndex);
	virtual ~CArchiveIterator();
	virtual void ConvertToSyncRecord(CRawRecordInfo* inRecordData, CSyncRecord* outCSyncRecord) ;

	virtual void ConvertFromSyncRecord(const CSyncRecord& inCSyncRecord, void* outRecordData) ;

	virtual Boolean NextRecord(eSyncTypes inSyncType, CSyncRecord& ioSyncRecord);

	virtual Boolean NextCategory( CCategory& ioCategory );
	virtual void AddCategory( CCategory& inCategory ); //Override
	virtual void SetCategoryName( CCategory& inCategory, const char* newName ); //override
	virtual void SetCategoryID( CCategory& inCategory, UInt8 newID ); //override

	virtual Boolean GetRecordByID(long inRecID, eSyncTypes inSyncType, CSyncRecord& ioSyncRecord);

	virtual UInt32 WriteRecord(const CSyncRecord& inRecord);

	virtual void DeleteRecord(const CSyncRecord& inRecord);

	virtual Boolean UsesCategories();

protected:
	virtual Boolean GetRecordByIndex(long inIndex, eSyncTypes inSyncType, CSyncRecord& ioSyncRecord);

	long mCurrentIndex;
};

#endif	//__CArchiveIterator__
