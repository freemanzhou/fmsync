/*
	File:		CSynchronizer.h

	Contains:	xxx put contents here xxx

	Created by:	Chris LeCroy on Sun, Nov 9, 1997

	Copyright:	© 1997 by Now Software, Inc., all rights reserved.

	Change History (most recent first):

				00/00/00---	file created

	To Do:
*/

#pragma once

#ifndef __CSynchronizer__
#define __CSynchronizer__

#define ThrowIfErrorC_(err)											\
	do {															\
		ExceptionCode	__theErr = err;								\
		if (__theErr != 0) {										\
			Throw_(__theErr);										\
		}															\
	} while (false)
	
#include "SyncMgr.h"
#include "StSharedBuffer.h"
#include "CRecordIterator.h"
#include <string.h>

#include <TArray.h>
#include <TArrayIterator.h>

class CRecordIterator;
class CSyncRecord;



class CCategoryMap
{
public:
	CCategoryMap(){ flagBits = 0x00; catName[0] = '\0'; action = 0x00000000; };

	enum
	{
		kUnfiledCatID = 	0,
		kValidOnPilot =		1 << 0,
		kValidOnLocal =		1 << 1
	};

	UInt8		pilotID;
	UInt8		localID;
	UInt8		flagBits;
	UInt32		action;
	char		catName[CCategory::kMaxCategoryNameLen+1];
};

#pragma warn_hidevirtual off
typedef TArray<CCategoryMap>			CCategoryMapArray;
typedef TArrayIterator<CCategoryMap>	CCategoryMapArrayIterator;
#pragma warn_hidevirtual reset

class CSynchronizer
{
public:
	CSynchronizer(const CSyncProperties& inSyncProperties);
	virtual ~CSynchronizer();

	virtual void Synchronize();
	
	virtual CRecordIterator* MakeRemoteIterator(short inDBIndex) = 0;
	virtual CRecordIterator* MakeLocalIterator(short inDBIndex) = 0;
	virtual CRecordIterator* MakeArchiveIterator(short inDBIndex);

#ifdef _DEBUG
	char* SyncStateToString(UInt32 inSyncState);
#endif
	
protected:
	
	typedef enum
	{
		kPilotToLocal,
		kLocalToPilot
	} SyncDirection;
	
	typedef enum
	{
		kPilotSource,
		kLocalSource
	} CategorySource;
	
	typedef enum
	{
		kNoAction					= 0x00000000,
		kCatSyncErr					= 0x00000001,
		kRemoteAddToLocal			= 0x00000002,
		kLocalAddToRemote			= 0x00000004,
		kRemoteReplaceLocalName		= 0x00000008,
		kLocalReplaceRemoteName		= 0x00000010,
		kRemoteReplaceLocalID		= 0x00000020,
		kLocalReplaceRemoteID		= 0x00000040,
		kLocalMapToRemoteUnfiled	= 0x00000080,
		kNumCatSyncActions			= 32
	} CatSyncAction;
	
	virtual void MakeIterators(short inDBIndex);
	virtual void DeleteIterators();
	
	virtual void SynchronizeAppInfoBlock();
	virtual void SynchronizeSortInfoBlock();
	void SynchronizeCategories();
	void GetCategories();
	void BuildCategoryMapArray();
	void WriteCategoryMapArray();
	void MapCategoriesOntoRecord( CSyncRecord& theRecord, SyncDirection direction );
	
	void ExtractCategory( const CCategoryMap& catMap, CategorySource catSource, CCategory& outCategory );
	
	//the various types of syncs
	void DoFastSync();
	void DoSlowSync();
	void DoReplaceRemoteSync();
	void DoReplacePCSync();
	
	virtual UInt32 GetSyncActions(const CSyncRecord& inRemoteRecord, const CSyncRecord& inLocalRecord);
	virtual void PerformSyncActions(UInt32 inActionsMap, const CSyncRecord& inRemoteRecord, const CSyncRecord& inLocalRecord);

	void GetCatSyncActions();

	
	virtual void DoPreSync();
	virtual void DoPostSync(bool wasCanceled);
	
	CSyncProperties 	mSyncProperties;			// contains info (fast/slow etc)
	CSystemInfo			mSystemInfo;				//might be useful, I suppose
	
	CRecordIterator* 	mPilotIterator;
	CRecordIterator* 	mLocalRecIterator;
	CRecordIterator* 	mArchiveIterator;
	
	CCategoryMapArray	mCategoryMapArray;
	CCategoryArray		mPilotCategories;
	CCategoryArray		mLocalCategories;
	
	Boolean				mCategoriesPresent;
	Boolean				mTriedPostSync;
	
private:
	StSharedBuffer mSharedBuffer;

};


#endif	//__CSynchronizer__
