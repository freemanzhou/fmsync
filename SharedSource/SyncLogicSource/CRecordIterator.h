/*
	File:		CRecordIterator.h

	Contains:	xxx put contents here xxx

	Created by:	Chris LeCroy on Sun, Nov 16, 1997

	Copyright:	© 1997 by Now Software, Inc., all rights reserved.

	Change History (most recent first):

				00/00/00---	file created

	To Do:
*/

#pragma once

#ifndef __CRecordIterator__
#define __CRecordIterator__

#include "SyncMgr.h"
#include "HotSyncDefines.h"
#include "hslog.h"

#include <TArray.h>
#include <TArrayIterator.h>

/*
	CRecordIterator
	
	Abstract class used for "iterating" over palm or pc record collections.
	
	Derived classes override GetRecordByIndex.  
	Derived classes have a few options in terms of data access optimization.
	They can:
		do nothing special - just get/allocate and deallocate each record as called
		cache objects that are "released"
		load an entire (or partial) table of data the first time GetRecordByIndex is called
		etc...
*/
class CSyncRecord;

class CCategory
{
public:
	
	enum	{ 	kMaxCategoryNameLen		= 15,
				kMaxNumPilotCategories	= 16 };

	CCategory(){ catName[0] = '\0'; hasChanged = false; };

	UInt8		catID;
	char		catName[kMaxCategoryNameLen+1];
	Boolean		hasChanged;
	
};

#pragma warn_hidevirtual off
typedef TArray<CCategory>				CCategoryArray;
typedef TArrayIterator<CCategory>		CCategoryArrayIterator;
#pragma warn_hidevirtual reset


class CRecordIterator
{
public:
	CRecordIterator(const CSyncProperties& inSyncProperties, short inDBIndex);
	virtual ~CRecordIterator() {}
	
	//Record manipulation
	//virtual Boolean GetRecordByIndex(long inIndex, eSyncTypes inSyncType, CSyncRecord& ioSyncRecord) = 0;
	virtual Boolean GetRecordByID(long inRecID, eSyncTypes inSyncType, CSyncRecord& ioSyncRecord) = 0;
	
	virtual void UpdateRecordID(CSyncRecord& inRecord, UInt32 inNewRecordID);
	
	virtual UInt32 WriteRecord(const CSyncRecord& inRecord) = 0; /*returns the rec ID*/
	virtual UInt32 AddRecord(const CSyncRecord& inRecord); /*returns the rec ID*/
	virtual void DeleteRecord(const CSyncRecord& inRecord) = 0;
	
	virtual Boolean GetAppInfo(CDbGenInfo* ioAppInfo) {return false;}
	virtual void WriteAppInfo(const CDbGenInfo& inAppInfo){};
	
	virtual Boolean GetSortInfo(CDbGenInfo* ioSortInfo) {return false;}
	virtual void WriteSortInfo(const CDbGenInfo& inSortInfo){};
	
	void OBSOLETE_Reset();
	
	Boolean OBSOLETE_CurrentRecord(eSyncTypes inSyncType, CSyncRecord& ioSyncRecord);
	Boolean OBSOLETE_NextRecord(eSyncTypes inSyncType, CSyncRecord& ioSyncRecord);
	virtual Boolean NextRecord(eSyncTypes inSyncType, CSyncRecord& ioSyncRecord) = 0;

	virtual Boolean NextCategory( CCategory& ioCategory ) = 0 ;
	
	virtual void AddCategory( CCategory& inCategory ) = 0 ;
	virtual void SetCategoryName( CCategory& inCategory, const char* newName ) = 0 ;
	virtual void SetCategoryID( CCategory& inCategory, UInt8 newID ) = 0 ;
	
	virtual void PreSync();
	virtual void PostSync(bool wasCanceled);
	
	virtual Boolean UsesCategories() = 0;
	
	virtual void DebugStreamRecord(const CSyncRecord& inCSyncRecord);

protected:

	//Record conversion
	//virtual void ConvertToSyncRecord(const void* inRecordData, CSyncRecord* outCSyncRecord) = 0;
	//virtual void ConvertFromSyncRecord(const CSyncRecord& inCSyncRecord, void* outRecordData) = 0;
	
	//long mCurrentIndex;
	CSyncProperties mSyncProperties;
	//details
};


#endif	//__CRecordIterator__
