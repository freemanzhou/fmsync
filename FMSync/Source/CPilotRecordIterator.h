/*
	File:		CPilotRecordIterator.h

	Contains:	xxx put contents here xxx

	Created by:	Chris LeCroy on Thu, Nov 6, 1997

	Copyright:	© 1997 by Now Software, Inc., all rights reserved.

	Change History (most recent first):

				 00/00/00	---	file created

	To Do:
*/

#pragma once

#ifndef __CPilotRecordIterator__
#define __CPilotRecordIterator__

#include "CRecordIterator.h"

#include <TArray.h>
#include <TArrayIterator.h>

#include "CSyncRecord.h"
#include "StSharedBuffer.h"

enum
{
	kPilotAttrNone		=	0x00,
	
	kPilotAttrPrivate	=	0x10,
	kPilotAttrModified	=	0x40,
	kPilotAttrDeleted	=	0x80,
	kPilotAttrPending	=	0x04,
	kPilotAttrArchived	=	0x08,
	//bits which should be zeroed out before a record is written back to the pilot
	kPilotAttrTempAttrsMask	=	kPilotAttrModified|kPilotAttrDeleted
};


typedef TArray<CSyncRecord*>			CSyncRecordArray;
typedef	TArrayIterator<CSyncRecord*>		CSyncRecordArrayIterator;


class CPilotCategoryInfo
{
public:
	CPilotCategoryInfo(){ ::memset( this, 0x00, sizeof( CPilotCategoryInfo ) ); };

	UInt16	namesChangedBits;
	char	catNames[CCategory::kMaxCategoryNameLen+1][CCategory::kMaxNumPilotCategories];
	UInt8	catIDs[CCategory::kMaxNumPilotCategories];
};

class CPilotRecordIterator : public CRecordIterator
{
public:
	CPilotRecordIterator(const CSyncProperties& inSyncProperties, short inDBIndex);
	virtual ~CPilotRecordIterator();
	
	virtual Boolean GetRecordByID(long inRecID, eSyncTypes inSyncType, CSyncRecord& ioSyncRecord);
	virtual UInt32 WriteRecord(const CSyncRecord& inRecord);
	virtual void DeleteRecord(const CSyncRecord& inRecord);
	virtual Boolean GetAppInfo(CDbGenInfo* ioAppInfo);
	virtual void WriteAppInfo(const CDbGenInfo& inAppInfo);
	virtual Boolean GetSortInfo(CDbGenInfo* ioSortInfo);
	virtual void WriteSortInfo(const CDbGenInfo& inSortInfo);
	virtual void PreSync();
	virtual void PostSync();
	virtual void DebugStreamRecord(const CSyncRecord& inCSyncRecord);
	virtual Boolean NextRecord(eSyncTypes inSyncType, CSyncRecord& ioSyncRecord); //Override
	virtual Boolean NextCategory( CCategory& ioCategory ); //Override
	virtual void AddCategory( CCategory& inCategory ); //Override
	virtual void SetCategoryName( CCategory& inCategory, const char* newName ); //override
	virtual void SetCategoryID( CCategory& inCategory, UInt8 newID ); //override
	virtual Boolean UsesCategories(); //override

	

protected:
	virtual void ConvertFromSyncRecord(const CSyncRecord& inCSyncRecord, void* outRecordData);
	virtual void ConvertToSyncRecord(const void* inRecordData, CSyncRecord* outCSyncRecord);
	void AddFieldToPilotRecord(UInt32 fieldNum, Ptr fieldData, short fieldLen, CRawRecordInfo* rawRecordInfo);
	void AddStringFieldToPilotRecord(UInt32 fieldNum,  char* inString,  CRawRecordInfo* rawRecordInfo);
	void TimeOperations( CRawRecordInfo& inRawRecord, const CSyncRecord& inCSyncRecord);
	static Handle AllocateHandle();
	void ReadPilotRecords(eSyncTypes inSyncType);
	virtual Boolean OBSOLETE_GetRecordByIndex(long inIndex, eSyncTypes inSyncType, CSyncRecord& ioSyncRecord);
	virtual Boolean GetRecordByIndex(long inIndex, eSyncTypes inSyncType, CSyncRecord& ioSyncRecord);
	UInt8 ConvertCategoryIndexToID( UInt16 inIndex );
	UInt16 ConvertCategoryIDToIndex( UInt8 inID );

	BYTE	mPilotDBHandle;
	DWORD	mLastIDRetrieved;
	WORD	mLastIndexRetrieved;
	
	CDbGenInfo mAppInfo;
	CDbGenInfo mSortInfo;
	CDbList		mCDbList;
	
	CSyncRecordArray mPilotRecords;
	
	StSharedBuffer mSharedBuffer;

	long mCurrentIndex;
	
	SInt8 mCurrentCategory;
	
	CPilotCategoryInfo* mCategoryInfo;
};



#endif	//__CPilotRecordIterator__
