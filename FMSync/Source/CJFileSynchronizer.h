#pragma once

#ifndef __CJFileSynchronizer__
#define __CJFileSynchronizer__

#ifndef __CSynchronizer__
#include "CSynchronizer.h"
#endif

#include "Reader.h"
#include "CDataTask.h"
#include "CDatabaseFile.h"
#include "CDatabaseInfo.h"
#include "CFieldIDList.h"
#include "CFieldOverrides.h"
#include "CSyncRecord.h"
#include "CWarnings.h"
#include "FMDatabasePtr.h"

class CConduitSettings;
class CDatabaseFile;
class CFileMakerRecordIterator;
class CJFileRecordIterator;

class CConduitProgress : public CReaderProgress
{
public:
	 CConduitProgress();
	 ~CConduitProgress();
	 
	virtual void DoProgress(const string& progressString);
};

class CJFileSynchronizer : public CSynchronizer
{
public:
	CJFileSynchronizer(const CSyncProperties& inSyncProperties, CConduitSettings *settings, PROGRESSFN progress);
	virtual ~CJFileSynchronizer();

	static void SetupDefaults(bool, bool);

	virtual void Synchronize();

	virtual CRecordIterator* MakeRemoteIterator(short inDBIndex);
	virtual CRecordIterator* MakeLocalIterator(short inDBIndex);
	virtual void MakeIterators(short inDBIndex);
	virtual void DeleteIterators();
	virtual void SynchronizeAppInfoBlock();
	virtual void SynchronizeSortInfoBlock();
	
	void SetupForSyncMode();
	void RecordJFileNames();
	void UploadDatabases();

	virtual UInt32 GetSyncActions(const CSyncRecord& inRemoteRecord, const CSyncRecord& inLocalRecord);
	virtual void PerformSyncActions(UInt32 inActionsMap, const CSyncRecord& inRemoteRecord, const CSyncRecord& inLocalRecord);

	virtual void DoPreSync();
	virtual void DoPostSync(bool wasCanceled);
	virtual bool ExecuteTasks();

	static bool LayoutIsOK(FileMaker::DatabasePtr theDB, int layoutID, string& string1, string& string2);
	static bool ScriptIsOK(FileMaker::DatabasePtr theDB, FMAE::ScriptID scriptID);
	bool ErrorOccurred() { return fErrorOccurred;};
	static void PutErrorCodeIntoLogNoFile(ExceptionCode errorCode);
	
	static bool TypeDeterminationNeedsData(int fieldType);
	static int JFileFromFMType(int fieldType, const string& sampleData, bool hasChoices);
	
	int GetTotalCount() const {return fTotalTaskCount;}

private:

	string GetTranslatedRemoteName() const;
	void TruncateLongField(string& fieldData) const;
	void CountedProgress(const string& baseString, int currentCount, int maxCount);

	void MakeDataHolder(CDataHolder& holder, const CSyncRecord& inCSyncRecord, const CFieldIDList& fieldIDs, bool useLocal);
	void ExecuteTasksInList(const string& processingString, bool forPalm);
	
	void ExecuteTaskForPalm(CDataTask& task);
	void LocalReplaceRemotePalm(CDataTask& theTask);
	void RemoteReplaceLocalPalm(CDataTask& theTask);
	void LocalAddToRemotePalm(CDataTask& theTask);
	void RemoteAddToLocalPalm(CDataTask& theTask);
	void LocalDeletePalm(CDataTask& theTask);
	void RemoteDeletePalm(CDataTask& theTask);
	void MergePalm(CDataTask& theTask);
	void MergeDeleteRemotePalm(CDataTask& theTask);
	void DuplicatePalm(CDataTask& theTask);
	void DuplicateDeleteRemotePalm(CDataTask& theTask);
	
	void ExecuteTaskForFM(CDataTask& task);
	void LocalReplaceRemoteFM(CDataTask& theTask);
	void RemoteReplaceLocalFM(CDataTask& theTask);
	void LocalAddToRemoteFM(CDataTask& theTask);
	void RemoteAddToLocalFM(CDataTask& theTask);
	void LocalDeleteFM(CDataTask& theTask);
	void RemoteDeleteFM(CDataTask& theTask);
	void MergeFM(CDataTask& theTask);
	void MergeDeleteRemoteFM(CDataTask& theTask);
	void DuplicateFM(CDataTask& theTask);
	void DuplicateDeleteRemoteFM(CDataTask& theTask);
	
	void ForgetDatabase();
	bool MapFieldNames();
	void UpdateDatabaseInfo();
	CSyncRecord::IgnoreMap MakeFieldsToIgnore();
	
	map<FMAE::FieldID,string> GetDefaultFieldNameMap();
	map<FMAE::FieldID,int> GetDefaultFieldTypeMap();
	map<FMAE::FieldID,int> GetDefaultFieldWidthMap();
	map<FMAE::FieldID,int> GetDefaultFieldReadOnlyMap();
	map<FMAE::FieldID,string_vector> GetDefaultChoicesMap();
	
	void SynchronizeOne(int remoteDBIndex);
	
	void PutTaskAndErrorIntoLog(CDataTask& theTask, ExceptionCode errorCode);
	void PutRemoteDataHolderIntoLog(CDataTask& theTask);
	void PutLocalDataHolderIntoLog(CDataTask& theTask);
	void PutDataHolderIntoLog(CDataHolder& theHolder, const CFieldIDList& fieldIDs);
	void PutFieldErrorIntoLog();
	void PutAppleEventErrorIntoLog();
	void PutTabbedDataIntoLog(CDataHolder& holder);
	void PutErrorCodeIntoLog(ExceptionCode errorCode);
	
	void PutRecordBackToRemote(CDataHolder& holder, const vector<string>& fields);
	
	void OutputChangesAsTabbed();
	
	void CheckExistingFields(const CFieldIDList&, const vector<int>&);
	
	void PickUnusedName();
	
	void UpdateLastSyncTime(bool errorOccurred);

	CConduitSettings* fSettings;
	CDatabaseFile::Ptr fDatabaseFile;
	FileMaker::DatabasePtr fDatabase;
	CConduitProgress fProgress;
	CDatabaseInfo fFMInfo;
	CDatabaseInfo fJFileInfo;
	CFieldOverrides fOverrides;
	CDatabaseInfo fResolvedInfo;
	vector<CDataTask> fTasks;
	map<FMAE::RecordID, bool> fUpdatedLocalRecords;
	map<string, bool> fJFileProDatabases;
	CFileMakerRecordIterator* fFMIterator;
	CJFileRecordIterator* fJFileIterator;
	CFieldIDList fRemoteFieldIDs;
	CFieldIDList fLocalFieldIDs;
	CFieldIDList fAllLocalFieldIDs;
	CFieldIDList fExistingFieldIDs;
	CFieldIDList fExistingLocalFields;
	vector<int> fExistingLocalRepeats;
	int fRecordCount;
	int fSyncMode;
	int fTotalTaskCount;
	bool fFieldsWereMapped;
	bool fReSync;
	bool fErrorOccurred;
	bool fExistingFieldDeleted;
	bool fIgnorePalmRecords;
	bool fIgnoreFileMakerRecords;
	bool fDeletePalmRecordsBeforeSync;
	bool fDeletePalmRecordsAfterSync;
	bool fDuplicate;
	bool fUnattended;
	bool fDoFileMakerNow;
	bool fDoPalmNow;
	static bool gDefaultToDate;
	static bool gDefaultToInt;
};

#endif //__CJFileSynchronizer__
