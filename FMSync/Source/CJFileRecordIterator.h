#pragma once

#ifndef __CJFileRecordIterator__
#define __CJFileRecordIterator__

#include "CRecordIterator.h"
#include "CFieldIDList.h"

#include <TArray.h>
#include <TArrayIterator.h>

#include "CSyncRecord.h"
#include "StSharedBuffer.h"

class CDatabaseFile;
class CDatabaseInfo;

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


typedef vector<CSyncRecord> CSyncRecordArray;
#pragma warn_hidevirtual off
typedef	TArrayIterator<CSyncRecord*>		CSyncRecordArrayIterator;
#pragma warn_hidevirtual reset

class CConduitSettings;
class CDatabaseFile;

class CJFileRecordIterator : public CRecordIterator
{
public:
	friend class JFileIteratorTestCase;
	CJFileRecordIterator(const CSyncProperties& inSyncProperties, int index, bool translateText, bool jFile5);
	virtual ~CJFileRecordIterator();
	
	virtual Boolean GetRecordByID(long inRecID, eSyncTypes inSyncType, CSyncRecord& ioSyncRecord);
	virtual UInt32 WriteRecord(const CSyncRecord& inRecord);
	virtual void DeleteRecord(const CSyncRecord& inRecord);
	virtual Boolean GetAppInfo(CDbGenInfo* ioAppInfo);
	virtual void WriteAppInfo(const CDbGenInfo& inAppInfo);
	virtual Boolean GetSortInfo(CDbGenInfo* ioSortInfo);
	virtual void WriteSortInfo(const CDbGenInfo& inSortInfo);
	virtual void PreSync();
	virtual void PostSync(bool wasCanceled);
	virtual void DebugStreamRecord(const CSyncRecord& inCSyncRecord);
	virtual Boolean NextRecord(eSyncTypes inSyncType, CSyncRecord& ioSyncRecord); //Override
	virtual Boolean NextCategory( CCategory& ioCategory ); //Override
	virtual void AddCategory( CCategory& inCategory ); //Override
	virtual void SetCategoryName( CCategory& inCategory, const char* newName ); //override
	virtual void SetCategoryID( CCategory& inCategory, UInt8 newID ); //override
	virtual Boolean UsesCategories(); //override
	void	GetDatabaseInfo(CDatabaseInfo& info);
	void	SetDatabaseInfo(const CDatabaseInfo& info, const vector<int>& repeats);
	void	SetExistingFieldInfo(const CFieldIDList& fids, const vector<int>&);
	void	SetDesiredFieldInfo(const CFieldIDList& fids, const vector<int>&);
	void	ValidateData(const CSyncRecord& localData);
	void	SetFieldsToIgnore(const map<FMAE::FieldID, bool>& toIgnore) {fIgnore = toIgnore;}
	
	bool	CreatedDatabase() const;

	void	SetIgnorePalmRecords(bool ignore) {fIgnorePalmRecords = ignore;}
	void	SetDeleteBeforeSync(bool del) {fDeleteBeforeSync = del;}
	void	SetDeleteAfterSync(bool del) {fDeleteAfterSync = del;}
	void	SetRemoteFileName(const string& fileName) {mFileName = fileName;}
	
protected:
	virtual void ConvertFromSyncRecord(const CSyncRecord& inCSyncRecord, void* outRecordData);
	virtual void ConvertToSyncRecord(CRawRecordInfo* inRecordData, CSyncRecord* outCSyncRecord);
	void AddFieldToPilotRecord(UInt32 fieldNum, Ptr fieldData, short fieldLen, CRawRecordInfo* rawRecordInfo);
	void AddStringFieldToPilotRecord(UInt32 fieldNum,  char* inString,  CRawRecordInfo* rawRecordInfo);
	static Handle AllocateHandle();
	void ReadPilotRecords(eSyncTypes inSyncType);
	virtual Boolean GetRecordByIndex(long inIndex, eSyncTypes inSyncType, CSyncRecord& ioSyncRecord);
	UInt8 ConvertCategoryIndexToID( UInt16 inIndex );
	UInt16 ConvertCategoryIDToIndex( UInt8 inID );
	
	void DebugStreamRecord(const CFieldIDList& fieldIDs, const CSyncRecord& inCSyncRecord);

	BYTE	mPilotDBHandle;
	DWORD	mLastIDRetrieved;
	WORD	mLastIndexRetrieved;
	
	DWORD		mType;
	DWORD		mCreator;
	string		mFileName;
	
	CFieldIDList		fExistingFieldIDs;
	vector<int>			fExistingRepeats;
	CFieldIDList		fDesiredFieldIDs;
	vector<int>			fDesiredRepeats;
	map<FMAE::FieldID,bool> fIgnore;
	//map<int, int>	fIndexMap;
	bool			fTranslateText;
	bool			fCreated;
	bool			fJFilePro;
	bool			fIgnorePalmRecords;
	bool			fDeleteBeforeSync;
	bool			fDeleteAfterSync;
	bool			fNeedsSortOrSync;
	
	CSyncRecordArray mPilotRecords;
	
	StSharedBuffer mSharedBuffer;

	long mCurrentIndex;

private:
	CJFileRecordIterator(const CJFileRecordIterator&);

public:
	int		fFieldCount;
};



#endif	//__CJFileRecordIterator__
