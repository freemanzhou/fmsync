#pragma once

#ifndef __CFileMakerRecordIterator__
#define __CFileMakerRecordIterator__

#include "CRecordIterator.h"
#include "CSynchronizer.h"
#include "CDatabase.h"
#include "CDatabaseFile.h"
#include "FMDatabasePtr.h"

#include <map>
#include <vector>
#include <deque>

class CDatabaseFile;
class CDatabase;
class CConduitSettings;
class CDatabaseInfo;

class CFileMakerRecordIterator : public CRecordIterator
{
public:
	CFileMakerRecordIterator(const CSyncProperties& inSyncProperties, short inDBIndex, CDatabaseFile::Ptr dFile, FileMaker::DatabasePtr database);
	virtual ~CFileMakerRecordIterator();

	virtual Boolean NextRecord(eSyncTypes inSyncType, CSyncRecord& ioSyncRecord);

	virtual Boolean NextCategory( CCategory& ioCategory ); //Override
	virtual void AddCategory( CCategory& inCategory ); //Override
	virtual void SetCategoryName( CCategory& inCategory, const char* newName ); //override
	virtual void SetCategoryID( CCategory& inCategory, UInt8 newID ); //override
	virtual Boolean UsesCategories(); //override

	virtual Boolean GetRecordByID(long inRecID, eSyncTypes inSyncType, CSyncRecord& ioSyncRecord);

	virtual UInt32 WriteRecord(const CSyncRecord& inRecord);
	virtual UInt32 AddRecord(const CSyncRecord& inRecord);
	virtual void UpdateRecordID(CSyncRecord& inRecord, UInt32 inNewRecordID);

	virtual void DeleteRecord(const CSyncRecord& inRecord);

	virtual void PreSync();
	virtual void PostSync(bool wasCanceled);
	
	virtual Boolean GetAppInfo(CDbGenInfo* ioAppInfo);
	
	virtual void DebugStreamRecord(const CSyncRecord& inCSyncRecord);

	virtual Boolean GetRecordByIndex(long inIndex, eSyncTypes inSyncType, CSyncRecord& ioSyncRecord);

	void	AssociateLocalWithRemote(int localID, int remoteID);
	void	UpdateLocalData(int localID, const vector<string>& fieldData);
	void	ForgetLocal(int localID);
	void	GetDatabaseInfo(CDatabaseInfo& info);
	bool	IsReallyDeleted(int localID);
	void	SetDeletedByPilot(int fmID);

	void	SetIgnoreFMRecords(bool ignore) {fIgnore = ignore;}
	
private:
	void				DoDeleteRecords();
	void				DoAddRecords();
	void				DoChangeRecords();
	bool				IsDeletedByIndex(long inIndex);
	void				LogDatabaseError();
	
	int					mCurrentIndex;
	int					fFieldCount;
	CDbGenInfo			fAppInfo;
	CDatabaseFile::Ptr	fDatabaseFile;
	CDatabase			fDatabase;
	FileMaker::DatabasePtr	fFileMakerDatabase;
	map<UInt32, bool>	fRecordsToDelete;
	map<UInt32, bool>	fDeletedByPilot;
	deque<CRecordInfo>	fPilotRecordsToDelete;
	vector<string>		fFieldNames;
	CFieldIDList		fFieldIDs;
	vector<int>			fRecordIDs;
	vector<int>			fAllRecordIDs;
	vector<int>			fRecordIDsToDelete;
	deque<CRecord>		fRecordsToAdd;
	deque<CRecord>		fRecordsToChange;
	map<UInt32, UInt32>	fPilotToFM;
	map<UInt32, UInt32>	fRemoteIDToIndex;
	map<UInt32, UInt32>	fLocalIDToIndex;
	map<UInt32, UInt32>	fIndexToRecID;
	CRecordMap			fFMIDToRecInfo;
	bool				fIgnore;
	bool				fDoingDeleted;
	char				fTimeSeparator;
};
#endif	//__CFileMakerRecordIterator__
