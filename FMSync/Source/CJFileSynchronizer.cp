#include "CJFileSynchronizer.h"
#include "CFieldIDList.h"
#include "JFileConduit.h"
#include "JFilePro.h"
#include "JFile5.h"
#include "StSyncClasses.h"

#include "CConduitSettings.h"
#include "CConduitDialog.h"
#include "CDataTasksDialog.h"
#include "CEditNamesDialog.h"
#include "CEditDBDialog.h"
#include "CJFileRecordIterator.h"
#include "CFileMakerIterator.h"
#include "CDatabase.h"
#include "FMDatabase.h"
#include "CFileMaker.h"
#include "CUniqueNamer.h"
#include "ErrorStrings.h"
#include "ErrorCodes.h"
#include "Utilities.h"
#include "StAppleEvent.h"
#include "Stringiness.h"
//#include "SyncStates.h"
#include "Str255.h"
#include "LErrorMsg.h"
#include "CDataTask.h"
#include "OtherStrings.h"
#include "charset.h"
#include "FieldIDFactory.h"
#include "DebugOutput.h"
#include "WriteJFile5.h"
#include "WriteJFilePro.h"
#include "CDatabaseUpload.h"
#include "CFMFieldTyper.h"

using JFile5::CWriteJFile5;
extern Boolean gSafeCancel;
extern Boolean gCanceling;

const bool kOutputTasks = true;
const bool kOutputSyncActions = true;

CConduitProgress::CConduitProgress()
{
}

CConduitProgress::~CConduitProgress()
{
}


void
CConduitProgress::DoProgress(const string& progressString)
{
	CallConduitProgress(progressString.c_str());
}

bool CJFileSynchronizer::gDefaultToDate;
bool CJFileSynchronizer::gDefaultToInt;

CJFileSynchronizer::CJFileSynchronizer(const CSyncProperties& inSyncProperties, CConduitSettings *settings, PROGRESSFN progress) :
	CSynchronizer(inSyncProperties), fSettings(settings), 
	fDatabaseFile(0), fDatabase(0), fFieldsWereMapped(false), fReSync(false), fErrorOccurred(false), fUnattended(false), 
	fDuplicate(false)
{
}


CJFileSynchronizer::~CJFileSynchronizer()
{
	ForgetDatabase();
}


void
CJFileSynchronizer::SetupDefaults(bool defaultToDate, bool defaultToInt)
{
	gDefaultToDate = defaultToDate;
	gDefaultToInt = defaultToInt;
}

void CJFileSynchronizer::DeleteIterators()
{
	ForgetObject(mArchiveIterator);
	ForgetObject(mLocalRecIterator);
	ForgetObject(mPilotIterator);

	ForgetDatabase();
}

void
CJFileSynchronizer::Synchronize()
{
	DebugOutput::Output("CJFileSynchronizer::Synchronize()");
	try {
		StSyncRegisterConduit registeredConduit;

		// Obtain System Information
		DebugOutput::Output("	SyncReadSystemInfo()");
		ThrowIfErrorC_(::SyncReadSystemInfo(mSystemInfo));
		
		DebugOutput::Output("	RecordJFileNames()");
		RecordJFileNames();
		DebugOutput::Output("	UploadDatabases()");
		UploadDatabases();

		string userName(mSyncProperties.m_UserName);

		for (int remoteDBIndex=0; remoteDBIndex < mSyncProperties.m_nRemoteCount; remoteDBIndex++) 
		{
			int reSyncCount = 0;
			do {
				fReSync = false;
				SynchronizeOne(remoteDBIndex);
				if (gCanceling) {
					gCanceling = false;
					Throw_(userCanceledErr);
				}
				++reSyncCount;
			} while (fReSync && reSyncCount < 2);
		}
	}

	catch (const LException& inErr) {
		DebugOutput::Output("	catch (const LException& inErr)", inErr.GetErrorCode());
		PutErrorCodeIntoLogNoFile(inErr.GetErrorCode());
	}
}

void CJFileSynchronizer::MakeIterators(short inDBIndex)
{
	string userName(mSyncProperties.m_UserName);
	ThrowIfNil_(fDatabaseFile.get());
	if (!fDatabaseFile->HasFile())
		Throw_(fnfErr);

	FSSpec spec;
	bool useFoundSet = fDatabaseFile->GetUseFoundSet();
	fDatabaseFile->GetFSSpec(&spec);
	fDatabase.reset(new FileMaker::Database(spec, useFoundSet, &fProgress));

	fDatabase->EnterBrowseMode();

	int layoutID = fDatabaseFile->GetLayoutID();
	string s1, s2;
	if (!LayoutIsOK(fDatabase, layoutID, s1, s2)) {
		::LogAddEntry(s1.c_str(), slWarning, false);
		::LogAddEntry(s2.c_str(), slWarning, false);
		Throw_(kSyncQuietAbortErr);
	}
	FMAE::ScriptID scriptID(fDatabaseFile->GetPostSyncScriptID());
	if (scriptID.IsValid()) {
		if (!ScriptIsOK(fDatabase, scriptID)) {
			static string m1(LoadString(kFMJErrorStrings, kPostSyncNLEErrorIndex));
			::LogAddEntry(m1.c_str(), slWarning, false);
			Throw_(kSyncQuietAbortErr);
		}
	}
	 
	scriptID = fDatabaseFile->GetPreSyncScriptID();
	if (scriptID.IsValid()) {
		if (!ScriptIsOK(fDatabase, scriptID)) {
			static string m1(LoadString(kFMJErrorStrings, kPreSyncNLEErrorIndex));
			::LogAddEntry(m1.c_str(), slWarning, false);
			Throw_(kSyncQuietAbortErr);
		}
		fDatabase->DoScript(scriptID);
	}
	 
	if (fDatabase->IsMultiUser()) {
		static string m1(LoadString(kFMJErrorStrings, kMultiUserNoHotSyncIndex));
		::LogAddEntry(m1.c_str(), slWarning, false);
		Throw_(kSyncQuietAbortErr);
	}

	if (!useFoundSet)
		fDatabase->FindAll();

	CSynchronizer::MakeIterators(inDBIndex);
	
}

CRecordIterator* CJFileSynchronizer::MakeRemoteIterator(short inDBIndex) 
{
	fJFileIterator = new CJFileRecordIterator(mSyncProperties, inDBIndex, 
		fDatabaseFile->GetTranslateText(), fDatabaseFile->GetJFileVersion() == CDatabaseFile::JFilePro);
	ThrowIfNil_(fJFileIterator);
	fJFileIterator->SetIgnorePalmRecords(fIgnorePalmRecords);
	fJFileIterator->SetDeleteBeforeSync(fDeletePalmRecordsBeforeSync);
	fJFileIterator->SetDeleteAfterSync(fDeletePalmRecordsAfterSync);
	return fJFileIterator;
}

void
CJFileSynchronizer::SynchronizeAppInfoBlock()
{
}

void
CJFileSynchronizer::ForgetDatabase()
{
	fDatabaseFile.reset(0);
	fDatabase.reset(0);
	fUpdatedLocalRecords.clear();
	fTasks.clear();
}

CRecordIterator* CJFileSynchronizer::MakeLocalIterator(short inDBIndex) 
{
	fFMIterator = new CFileMakerRecordIterator(mSyncProperties, inDBIndex, fDatabaseFile, fDatabase);
	ThrowIfNil_(fFMIterator);
	fFMIterator->SetIgnoreFMRecords(fIgnoreFileMakerRecords);
	return fFMIterator;
}

const int kCountBy = 10;

void CJFileSynchronizer::ExecuteTaskForPalm(CDataTask& task)
{
	switch(task.GetAction()) {
	case CDataTask::kLocalAddToRemote:
		LocalAddToRemotePalm(task);
		break;
	case CDataTask::kRemoteAddToLocal:
		RemoteAddToLocalPalm(task);
		break;
	case CDataTask::kLocalReplaceRemote:
		LocalReplaceRemotePalm(task);
		break;
	case CDataTask::kRemoteReplaceLocal:
		RemoteReplaceLocalPalm(task);
		break;
	case CDataTask::kRemoteReplaceLocalDeleteRemote:
		RemoteReplaceLocalPalm(task);
		RemoteDeletePalm(task);
		break;
	case CDataTask::kDeleteLocal:
		LocalDeletePalm(task);
		break;
	case CDataTask::kDeleteRemote:
		RemoteDeletePalm(task);
		break;
	case CDataTask::kMerge:
		MergePalm(task);
		break;
	case CDataTask::kDuplicate:
		DuplicatePalm(task);
		break;
	case CDataTask::kDuplicateDeleteRemote:
		DuplicateDeleteRemotePalm(task);
		break;
	case CDataTask::kMergeDeleteRemote:
		MergeDeleteRemotePalm(task);
		break;
	}
}

void CJFileSynchronizer::ExecuteTaskForFM(CDataTask& task)
{
	switch(task.GetAction()) {
	case CDataTask::kLocalAddToRemote:
		LocalAddToRemoteFM(task);
		break;
	case CDataTask::kRemoteAddToLocal:
		RemoteAddToLocalFM(task);
		break;
	case CDataTask::kLocalReplaceRemote:
		LocalReplaceRemoteFM(task);
		break;
	case CDataTask::kRemoteReplaceLocal:
		RemoteReplaceLocalFM(task);
		break;
	case CDataTask::kRemoteReplaceLocalDeleteRemote:
		RemoteReplaceLocalFM(task);
		RemoteDeleteFM(task);
		break;
	case CDataTask::kDeleteLocal:
		LocalDeleteFM(task);
		break;
	case CDataTask::kDeleteRemote:
		RemoteDeleteFM(task);
		break;
	case CDataTask::kMerge:
		MergeFM(task);
		break;
	case CDataTask::kDuplicate:
		DuplicateFM(task);
		break;
	case CDataTask::kDuplicateDeleteRemote:
		DuplicateDeleteRemoteFM(task);
		break;
	case CDataTask::kMergeDeleteRemote:
		MergeDeleteRemoteFM(task);
		break;
	}
}

void CJFileSynchronizer::CountedProgress(const string& baseString, int currentOne, int maxNumber)
{
	static string kOfString(LoadString(kOtherStrings, kNofNIndex));
	if (currentOne < kCountBy || currentOne%kCountBy == 0) {
		string totalCountString(AsString(maxNumber));
		string progressString(baseString);
		progressString.append(AsString(currentOne));
		progressString.append(kOfString);
		progressString.append(totalCountString);
		fProgress.DoProgress(progressString);
	}
}

void CJFileSynchronizer::ExecuteTasksInList(const string& processingString, bool forPalm)
{

	vector<CDataTask>::iterator i = fTasks.begin();
	
	int totalCount = fTasks.size();
	int currentOne = 1;

	string totalCountString(AsString(totalCount));
	while (i != fTasks.end() && !fErrorOccurred) {
		CountedProgress(processingString, currentOne, totalCount);
		try {
			if (!forPalm) {
				if (i->GetAction() == CDataTask::kConflict)
					i->OverrideAction(fDuplicate);
				
				ExecuteTaskForFM(*i);
			} else {
				ExecuteTaskForPalm(*i);
			}
		}
		catch (const LException& inErr) {
			PutTaskAndErrorIntoLog(*i, inErr.GetErrorCode());
			fErrorOccurred = true;
		}
		++i;
		++currentOne;		
	}
}

bool CJFileSynchronizer::ExecuteTasks()
{
	static string kProcessingFM(LoadString(kOtherStrings, kProcessingFMTaskIndex));
	static string kProcessingPalm(LoadString(kOtherStrings, kProcessingPalmTaskIndex));
	fProgress.DoProgress(LoadString(kOtherStrings, kLookingForConflictsIndex));
	int conflictCount = 0;
	vector<CDataTask>::iterator i = fTasks.begin();
	int currentOne = 1;
	int totalCount = fTasks.size();
	fTotalTaskCount = fTasks.size();
	bool canceled = false;
	if (!fDuplicate) {
		while (i != fTasks.end()) {
			CountedProgress("Checking for conflicts for task ", currentOne, totalCount);
			if (i->IsConflict()) {
				conflictCount += 1;
			}
			currentOne += 1;
			++i;
		}
		if (conflictCount > 0) {
			CDataTasksDialog tasksDialog(fDatabaseFile, fDatabase, fTasks, fExistingLocalFields, fExistingLocalRepeats);
			canceled = tasksDialog.DoDialog();
		}
	}
	if (!canceled) {
		gSafeCancel = (fExistingFieldIDs == fRemoteFieldIDs);
		ExecuteTasksInList(kProcessingFM, false);
		ExecuteTasksInList(kProcessingPalm, true);
		if (!fErrorOccurred) {
			fFMIterator->GetDatabaseInfo(fFMInfo);
			UpdateDatabaseInfo();
			fDatabaseFile->RememberFieldInfo(fRemoteFieldIDs, fDatabase->GetFieldRepeats());
			fDatabaseFile->SetNewInstall(false);
		}
	}
	return canceled;
}

string
CJFileSynchronizer::GetTranslatedRemoteName() const
{
	string remoteName(fDatabaseFile->GetPilotDatabaseName());
	if (fDatabaseFile->GetTranslateText()) {
		ConvertToPilotText(remoteName);
	}
	return remoteName;
}

void
CJFileSynchronizer::DoPreSync()
{
	SetupDefaults(fDatabaseFile->GetDefaultToDate(), fDatabaseFile->GetDefaultToInt());
	ConfirmFreeStackSpace(); 
	fRecordCount = 0;
	fReSync = false;
	fTasks.clear();
	static string kInitForJFile(LoadString(kOtherStrings, kInitForJFileIndex));
	fProgress.DoProgress(kInitForJFile);
	PickUnusedName();
	fJFileIterator->SetRemoteFileName(GetTranslatedRemoteName());
	fJFileIterator->PreSync();
	if (fJFileIterator->CreatedDatabase()) {
		fDatabaseFile->ClearRecordMap();
		fDatabaseFile->SetNewInstall(true);
		fDatabaseFile->RememberFieldInfo(fDatabase->GetFieldIDs(), fDatabase->GetFieldRepeats());
	} else if (fDatabaseFile->NewInstall()) {
		vector<string> jNames(fSettings->GetJFileNames());
		if (find(jNames.begin(), jNames.end(), fDatabaseFile->GetPilotDatabaseName()) == jNames.end())
			Throw_(kNotJFileDatabase);
		Throw_(kCantStartExisting);
	}
	static string kInitForFM(LoadString(kOtherStrings, kInitForFMIndex));
	fProgress.DoProgress(kInitForFM);
	mLocalRecIterator->PreSync();
	mArchiveIterator->PreSync();
	fExistingFieldIDs = fDatabaseFile->GetFieldIDList();
	fOverrides = fDatabaseFile->FieldOverrides();
	fDuplicate = fDatabaseFile->GetDuplicateOnConflict() || fUnattended;
	fJFileIterator->SetExistingFieldInfo(fExistingFieldIDs, fDatabaseFile->GetFieldRepeats());
	CheckExistingFields(fExistingFieldIDs, fDatabaseFile->GetFieldRepeats());
	fRemoteFieldIDs = fDatabase->GetFieldIDs();
	fAllLocalFieldIDs = fDatabase->GetAllFieldIDs();
	fJFileIterator->SetDesiredFieldInfo(fRemoteFieldIDs, fDatabase->GetFieldRepeats());
	fLocalFieldIDs = fRemoteFieldIDs;
	fFMIterator->GetDatabaseInfo(fFMInfo);
	if (fDatabaseFile->NewInstall()) {
		DebugOutput::Output( "CJFileSynchronizer::DoPreSync() - new install" );
		fJFileInfo = fFMInfo;
		UpdateDatabaseInfo();
	} else {
		DebugOutput::Output( "CJFileSynchronizer::DoPreSync() - existing database" );
		fJFileIterator->SetExistingFieldInfo(fDatabaseFile->GetFieldIDList(), fDatabaseFile->GetFieldRepeats());
		fJFileIterator->GetDatabaseInfo(fJFileInfo);
		if (MapFieldNames())
			Throw_(userCanceledErr);
	}
	if (fDatabaseFile->NewInstall() || fDatabaseFile->GetSyncMode() == CDatabaseFile::SyncClearAndDownload)
		mSyncProperties.m_SyncType = eSlow;
	else
		mSyncProperties.m_SyncType = eFast;
	DebugOutput::Output( "CJFileSynchronizer::DoPreSync() - done" );
}

namespace {

bool IsCalcFieldType(int theType)
{
#ifdef JFILE_CALCULATIONS
	return (theType >= JFile5::FLDTYPE_CALC && theType <= JFile5::FLDTYPE_CALC_V2);
#else
	return FALSE;
#endif
}

pair<int,bool> IsCalcFieldOp(const pair<FMAE::FieldID,int>& theType)
{
	return make_pair(FieldIDFactory::GetID(theType.first), IsCalcFieldType(theType.second)); 
}

}

void CJFileSynchronizer::DoPostSync(bool wasCanceled)
{
	mTriedPostSync = true;
	if(mLocalRecIterator) {
		try {
			mLocalRecIterator->PostSync(wasCanceled);
		}
		catch (const LException& inErr) {
			PutErrorCodeIntoLog(inErr.GetErrorCode());
		}
	}
	if(mPilotIterator)
		mPilotIterator->PostSync(wasCanceled);
	if(mArchiveIterator)
		mArchiveIterator->PostSync(wasCanceled);
}

UInt32 
CJFileSynchronizer::GetSyncActions(const CSyncRecord& inRemoteRecord, const CSyncRecord& inLocalRecord )
{
	static string kExamining(LoadString(kOtherStrings, kExaminingRecordIndex));
	++fRecordCount;
	if (fRecordCount < kCountBy || fRecordCount%kCountBy == 0) {
		string progressString(kExamining);
		progressString.append(AsString(fRecordCount));
		fProgress.DoProgress(progressString);
	}
	Boolean recordsAreEqual = false;
	Boolean remoteEqualsLocal = false;
	Boolean localEqualsRemote = false;
	Boolean orderChanged = false;
	bool localReqFields = inLocalRecord.RequiredFieldsExist();
	bool remoteReqFields = inRemoteRecord.RequiredFieldsExist();

	if (localReqFields &&	
		remoteReqFields)
	{
		remoteEqualsLocal = inRemoteRecord.RecordDataEqualTo(inLocalRecord);
		localEqualsRemote = inLocalRecord.RecordDataEqualTo(inRemoteRecord);
		recordsAreEqual = remoteEqualsLocal && localEqualsRemote;
		if (remoteEqualsLocal)
			DebugOutput::Output( kOutputSyncActions, "\trremote == local\r" );
		if (localEqualsRemote)
			DebugOutput::Output( kOutputSyncActions, "\tlocale == remote\r" );
		if (recordsAreEqual)
			DebugOutput::Output( kOutputSyncActions, "\trecords are equal\r" );
	}
	
	if (fExistingFieldIDs != fLocalFieldIDs) {
		DebugOutput::Output( kOutputSyncActions, "\tfield order changed\r" );
		orderChanged = true;
	}

	if (!localReqFields) {
		DebugOutput::Output( kOutputSyncActions, "\tlocal not present\r" );
		if (inRemoteRecord.IsDeleted()) {
			DebugOutput::Output( kOutputSyncActions, "\tremote deleted --> do nothing\r" );
			return CDataTask::kDoNothing;
		}
		return CDataTask::kRemoteAddToLocal;
	}
	
	if (inLocalRecord.IsDeleted()) {
		DebugOutput::Output( kOutputSyncActions, "\tlocal deleted\r" );
		if (inRemoteRecord.IsDeleted()) {
			DebugOutput::Output( kOutputSyncActions, "\tremote deleted --> do nothing\r" );
			return CDataTask::kDoNothing;
		}
		if (inRemoteRecord.IsModified()) {
			DebugOutput::Output( kOutputSyncActions, "\tremote modified\r" );
			if (fFMIterator->IsReallyDeleted(inLocalRecord.GetLocalRecordID())) {
				DebugOutput::Output( kOutputSyncActions, "\tlocal deleted --> conflict\r" );
				return CDataTask::kConflict;
			} else {
				if (inLocalRecord.IsModified()) {
					DebugOutput::Output( kOutputSyncActions, "\tlocal modified --> conflict\r" );
					return CDataTask::kConflictDeleteRemote;
				}
				DebugOutput::Output( kOutputSyncActions, "\tlocal unchanged --> remote replace local delete remote\r" );
				return CDataTask::kRemoteReplaceLocalDeleteRemote;
			}
		}
		DebugOutput::Output( kOutputSyncActions, "\tremote unchanged --> delete remote\r" );
		return CDataTask::kDeleteRemote;
	}

	if (inLocalRecord.IsModified()) {
		DebugOutput::Output( kOutputSyncActions, "\tlocal modified\r" );
		if (!remoteReqFields) {
			DebugOutput::Output( kOutputSyncActions, "\tremote not present\r" );
			return CDataTask::kLocalAddToRemote;
		}
		
		if (inRemoteRecord.IsModified() || inRemoteRecord.IsDeleted()) {
			if (recordsAreEqual) {
				if (orderChanged) {
					DebugOutput::Output( kOutputSyncActions, "\tequal but order changed --> local replace remote\r" );
					return CDataTask::kLocalReplaceRemote;
				}
				DebugOutput::Output( kOutputSyncActions, "\trecords equal --> do nothing\r" );
				return CDataTask::kDoNothing;
			}
			DebugOutput::Output( kOutputSyncActions, "\tremote modified or deleted --> conflict\r" );
			return CDataTask::kConflict;
		}

		if (orderChanged) {
			DebugOutput::Output( kOutputSyncActions, "\torder changed --> local replace remote\r" );
			return CDataTask::kLocalReplaceRemote;
		}
			
		if (recordsAreEqual) {
			DebugOutput::Output( kOutputSyncActions, "\trecords equal --> do nothing\r" );
			return CDataTask::kDoNothing;
		}
		DebugOutput::Output( kOutputSyncActions, "\tremote unchanged --> local replace remote\r" );
		return CDataTask::kLocalReplaceRemote;
	}

	if (!remoteReqFields || inRemoteRecord.GetRemoteRecordID() == 0) {
		DebugOutput::Output( kOutputSyncActions, "\tremote not present --> local add to remote\r" );
		return CDataTask::kLocalAddToRemote;
	}
	
	if (inRemoteRecord.IsDeleted()) {
		DebugOutput::Output( kOutputSyncActions, "\tremote deleted\r" );
		if (inLocalRecord.IsDeleted()) {
			DebugOutput::Output( kOutputSyncActions, "\tlocal deleted --> do nothing\r" );
			return CDataTask::kDoNothing;
		}
		if (inLocalRecord.IsModified()) {
			DebugOutput::Output( kOutputSyncActions, "\tlocal modified --> conflict\r" );
			return CDataTask::kConflict;
		}
		DebugOutput::Output( kOutputSyncActions, "\tlocal unchanged --> delete local\r" );
		return CDataTask::kDeleteLocal;
	}

	if (inRemoteRecord.IsModified()) {
		DebugOutput::Output( kOutputSyncActions, "\tremote modified\r" );
		if (!localReqFields || inLocalRecord.GetLocalRecordID() == 0) {
			DebugOutput::Output( kOutputSyncActions, "\tlocal not present --> remote add to local\r" );
			return CDataTask::kRemoteAddToLocal;
		}
		if (inLocalRecord.IsModified() || inLocalRecord.IsDeleted()) {
			if (recordsAreEqual) {
				DebugOutput::Output( kOutputSyncActions, "\trecords equal --> do nothing\r" );
				return CDataTask::kDoNothing;
			}
			DebugOutput::Output( kOutputSyncActions, "\tlocal modified --> conflict\r" );
			return CDataTask::kConflict;
		}
			
		if (recordsAreEqual) {
			DebugOutput::Output( kOutputSyncActions, "\trecords are equal\r" );
			if (fFieldsWereMapped) {
				DebugOutput::Output( kOutputSyncActions, "\tfields mapped --> local replace remote\r" );
				return CDataTask::kLocalReplaceRemote;
			} else {
				DebugOutput::Output( kOutputSyncActions, "\trecords are equal --> do nothing\r" );
				return CDataTask::kDoNothing;
			}
		}
		DebugOutput::Output( kOutputSyncActions, "\tlocal unchanged --> remote replace local\r" );
		return CDataTask::kRemoteReplaceLocal;
	}

	DebugOutput::Output( kOutputSyncActions, "\tnothing changed --> do nothing\r" );
	return CDataTask::kDoNothing;
}

void
CJFileSynchronizer::MakeDataHolder(CDataHolder& holder, const CSyncRecord& inCSyncRecord, const CFieldIDList& fieldIDs, bool useLocal)
{
	bool exists = inCSyncRecord.RequiredFieldsExist();
	
	int recordID, dataState;
	int fieldCount = fieldIDs.size();
	
	if (exists) {
		for (int fieldIndex = 0; fieldIndex < fieldCount; fieldIndex += 1) {
			FMAE::FieldID fieldID = fieldIDs[fieldIndex];
			holder.AddToMap(fieldID, inCSyncRecord.GetValue( FieldIDFactory::GetID(fieldID)));
		}
		if (useLocal)
			recordID = inCSyncRecord.GetLocalRecordID();
		else
			recordID = inCSyncRecord.GetRemoteRecordID();
		if (recordID == 0) {
			dataState = CDataHolder::kAdded;
		} else if (inCSyncRecord.IsDeleted()) {
			dataState = CDataHolder::kDeleted;
		} else if (inCSyncRecord.IsModified()) {
			dataState = CDataHolder::kChanged;
		} else {
			dataState = CDataHolder::kUnchanged;
		}
	} else {
		recordID = 0;
		dataState = CDataHolder::kDoesNotExist;
	}
	holder.fRecordID = recordID;
	holder.fDataState = dataState;
}

static Boolean HasLocalData(int action)
{
	switch(action) {
	case CDataTask::kLocalAddToRemote:
	case CDataTask::kLocalReplaceRemote:
	case CDataTask::kMerge:
	case CDataTask::kMergeDeleteRemote:
	case CDataTask::kDuplicate:
	case CDataTask::kDuplicateDeleteRemote:
	case CDataTask::kConflict:
		return true;
	}
	return false;
}

static Boolean IsConflict(int action)
{
	switch(action) {
	case CDataTask::kConflictDeleteRemote:
	case CDataTask::kConflict:
		return true;
	}
	return false;
}

void
CJFileSynchronizer::PerformSyncActions(UInt32 inAction, const CSyncRecord& inRemoteRecord, const CSyncRecord& inLocalRecord)
{
	if(inAction == CDataTask::kDoNothing)
		return;
		
	if (inLocalRecord.RequiredFieldsExist()) {
		int localRecordID = inLocalRecord.GetLocalRecordID();
		if (fUpdatedLocalRecords[localRecordID])
			return;
		fUpdatedLocalRecords[localRecordID] = true;
	}
	
	CDataTask theTask(inAction);
	MakeDataHolder(theTask.fLocal, inLocalRecord, fLocalFieldIDs, true);
	if (inAction != CDataTask::kLocalAddToRemote) {
		MakeDataHolder(theTask.fRemote, inRemoteRecord, fExistingFieldIDs, false);
		if (IsConflict(inAction) && fExistingFieldIDs != fLocalFieldIDs) {
			vector<string> entireRecord(fDatabase->GetRecordAllFields(theTask.fLocal.fRecordID));
			theTask.fLocal.AddToDataMap(fAllLocalFieldIDs, entireRecord);
			if (theTask.RemoteMatchesLocal()) {
				DebugOutput::Output( kOutputSyncActions, "\trremote == local: conflict averted\r" );
				if (theTask.GetAction() == CDataTask::kConflictDeleteRemote)
					theTask.fAction = CDataTask::kDeleteRemote;
				else
					theTask.fAction = CDataTask::kLocalReplaceRemote;
			}
		}

	}

	theTask.InitializeFieldSelector(true);
	fTasks.push_back(theTask);
}

static void ConvertToSyncRecord(const CDataHolder& holder, CSyncRecord& targetRecord)
{
	vector<string> fields(holder.MakeDataVector());
	CFieldIDList fieldIDs(holder.MakeFieldIDVector());
	int fieldCount = fieldIDs.size();
	for (int fieldIndex = 0; fieldIndex < fieldCount; fieldIndex += 1) {
		targetRecord.SetValue(FieldIDFactory::GetID(fieldIDs[fieldIndex]), fields[fieldIndex]);
	}
		
	targetRecord.SetAttributes(0);
	enum {kUnchanged, kChanged, kDeleted, kAdded, kDoesNotExist};
	if (holder.fDataState == CDataHolder::kChanged)
		targetRecord.SetModified(true);
	else if (holder.fDataState == CDataHolder::kDeleted)
		targetRecord.SetDeleted(true);
}

void
CJFileSynchronizer::LocalAddToRemoteFM(CDataTask& theTask)
{
}

void
CJFileSynchronizer::LocalAddToRemotePalm(CDataTask& theTask)
{
	CSyncRecord rec;
	
	int localRecID = theTask.fLocal.fRecordID;
	ConvertToSyncRecord(theTask.fLocal, rec);
	rec.SetLocalRecordID(0);
	rec.SetRemoteRecordID(0);
	rec.SetAttributes(0);
	fFMIterator->AssociateLocalWithRemote(localRecID, mPilotIterator->AddRecord(rec));
	fFMIterator->UpdateLocalData(localRecID, theTask.fLocal.MakeOrderedDataVector(fLocalFieldIDs));
}

void
CJFileSynchronizer::RemoteAddToLocalFM(CDataTask& theTask)
{
	CDataHolder holder;
	theTask.GetRemoteData(holder);
	PutTabbedDataIntoLog(holder);
	theTask.fLocal.fRecordID = fDatabase->AddRecord(theTask.fRemote.MakeFieldIDVector(), theTask.fRemote.MakeDataVector());
	fFMIterator->AssociateLocalWithRemote(theTask.fLocal.fRecordID, theTask.fRemote.fRecordID);
}

void
CJFileSynchronizer::RemoteAddToLocalPalm(CDataTask& theTask)
{
	CDataHolder holder;
	theTask.GetRemoteData(holder);
	vector<string> fields(fDatabase->GetRecord(theTask.fLocal.fRecordID));
	fFMIterator->UpdateLocalData(theTask.fLocal.fRecordID, fields);
	holder.fRecordID = theTask.fRemote.fRecordID;
	PutRecordBackToRemote(holder, fields);
	fReSync = true;
}

void
CJFileSynchronizer::LocalReplaceRemoteFM(CDataTask& theTask)
{
	CDataHolder holder;
	theTask.GetLocalData(holder);
	fFMIterator->UpdateLocalData(theTask.GetLocalRecordID(), holder.MakeOrderedDataVector(fLocalFieldIDs));
}

void
CJFileSynchronizer::LocalReplaceRemotePalm(CDataTask& theTask)
{
	CSyncRecord rec;
	
	CDataHolder holder;
	theTask.GetLocalData(holder);
	holder.fDataState = 0;
	ConvertToSyncRecord(holder, rec);
	rec.SetRemoteRecordID(theTask.GetRemoteRecordID());
	mPilotIterator->WriteRecord(rec);
}

void
CJFileSynchronizer::RemoteReplaceLocalFM(CDataTask& theTask)
{
	CDataHolder holder;
	theTask.GetRemoteData(holder);
	PutTabbedDataIntoLog(holder);
	fDatabase->WriteRecord(theTask.GetLocalRecordID(), holder.MakeFieldIDVector(), holder.MakeDataVector());
	fFMIterator->UpdateLocalData(theTask.GetLocalRecordID(), holder.MakeOrderedDataVector(fLocalFieldIDs));
	fReSync = true;
}

void
CJFileSynchronizer::RemoteReplaceLocalPalm(CDataTask& theTask)
{
	CDataHolder holder;
	theTask.GetRemoteData(holder);
	holder.fRecordID = theTask.GetRemoteRecordID();
	vector<string> fields(fDatabase->GetRecord(theTask.GetLocalRecordID()));
	PutRecordBackToRemote(holder, fields);
	fReSync = true;
}

void
CJFileSynchronizer::PutRecordBackToRemote(CDataHolder& holder, const vector<string>& fields)
{
	if (!fDeletePalmRecordsAfterSync) {
		holder.SetDataMap(fDatabase->GetFieldIDs(), fDatabase->GetFieldRepeats(), fields);
		CSyncRecord rec;
		ConvertToSyncRecord(holder, rec);
		rec.SetRemoteRecordID(holder.fRecordID);
		mPilotIterator->WriteRecord(rec);
	}
}

void
CJFileSynchronizer::LocalDeleteFM(CDataTask& theTask)
{
	fDatabase->DeleteRecord(theTask.GetLocalRecordID());
	fFMIterator->ForgetLocal(theTask.GetLocalRecordID());
}

void
CJFileSynchronizer::LocalDeletePalm(CDataTask& theTask)
{
}

void
CJFileSynchronizer::RemoteDeleteFM(CDataTask& theTask)
{
	fFMIterator->ForgetLocal(theTask.GetLocalRecordID());
}

void
CJFileSynchronizer::RemoteDeletePalm(CDataTask& theTask)
{
	CSyncRecord rec;
	
	CDataHolder holder;
	theTask.GetRemoteData(holder);
	ConvertToSyncRecord(holder, rec);
	rec.SetRemoteRecordID(theTask.GetRemoteRecordID());
	mPilotIterator->DeleteRecord(rec);
}

void
CJFileSynchronizer::MergeFM(CDataTask& theTask)
{
	CDataHolder holder;
	theTask.GetMergedData(holder);
	PutTabbedDataIntoLog(holder);
	fDatabase->WriteRecord(theTask.GetLocalRecordID(), holder.MakeFieldIDVector(), holder.MakeDataVector());
}

void
CJFileSynchronizer::MergePalm(CDataTask& theTask)
{
	CSyncRecord rec;
	
	CDataHolder holder;
	theTask.GetMergedData(holder);
	vector<string> fields(fDatabase->GetRecord(theTask.GetLocalRecordID()));
	fFMIterator->UpdateLocalData(theTask.GetLocalRecordID(), fields);
	holder.SetDataMap(fDatabase->GetFieldIDs(), fDatabase->GetFieldRepeats(), fields);
	holder.fRecordID = theTask.GetRemoteRecordID();
	ConvertToSyncRecord(holder, rec);
	rec.SetRemoteRecordID(theTask.GetRemoteRecordID());
	mPilotIterator->WriteRecord(rec);
	fReSync = true;
}

void
CJFileSynchronizer::MergeDeleteRemoteFM(CDataTask& theTask)
{
	CDataHolder holder;
	theTask.GetMergedData(holder);
	PutTabbedDataIntoLog(holder);
	fDatabase->WriteRecord(theTask.GetLocalRecordID(), holder.MakeFieldIDVector(), holder.MakeDataVector());
}

void
CJFileSynchronizer::MergeDeleteRemotePalm(CDataTask& theTask)
{
	CDataHolder holder;
	theTask.GetMergedData(holder);
	CSyncRecord rec;
	ConvertToSyncRecord(holder, rec);
	mPilotIterator->DeleteRecord(rec);
	fReSync = true;
}

void
CJFileSynchronizer::DuplicateFM(CDataTask& theTask)
{
	DebugOutput::Output("CJFileSynchronizer::DuplicateFM()");
	RemoteAddToLocalFM(theTask);
}

void
CJFileSynchronizer::DuplicatePalm(CDataTask& theTask)
{
	DebugOutput::Output("CJFileSynchronizer::DuplicatePalm()");
	LocalAddToRemotePalm(theTask);
}

void
CJFileSynchronizer::DuplicateDeleteRemoteFM(CDataTask& theTask)
{
	RemoteAddToLocalFM(theTask);
}

void
CJFileSynchronizer::DuplicateDeleteRemotePalm(CDataTask& theTask)
{
	RemoteDeletePalm(theTask);
}

bool
CJFileSynchronizer::MapFieldNames()
{
#ifdef MAP_FIELD_NAMES
	vector<int> pilotFieldIDs(fDatabaseFile->GetFieldIDList());
	vector<string> oldPilotNames(fDatabaseFile->GetFieldNameList());
	vector<string> fmNames(fFMInfo.GetFieldNames(pilotFieldIDs));
	vector<string> pilotNames(fJFileInfo.GetFieldNames(pilotFieldIDs));
	
	if (pilotNames != oldPilotNames) {
		CEditNamesDialog dialog(fDatabaseFile->GetPilotDatabaseName(), pilotNames, fmNames);
		if (dialog.DoDialog()) {
			return true;
		}
		vector<int> order(dialog.GetOrder());
		vector<int> selection(dialog.GetSelection());
		vector<int> newPilotFieldIDs;
		vector<string> newPilotNames;
		int selectionCount = pilotFieldIDs.size();
		for (int i = 0; i < selectionCount; i += 1) {
			if (selection[i] == 0) {
				newPilotFieldIDs.push_back(0);
				newPilotNames.push_back("");
			} else {
				int orderIndex = order[i];
				newPilotFieldIDs.push_back(pilotFieldIDs[orderIndex]);
				newPilotNames.push_back(pilotNames[orderIndex]);
			}
		}
		fFieldsWereMapped = (pilotFieldIDs != newPilotFieldIDs);
		pilotFieldIDs = newPilotFieldIDs;
		fJFileIterator->SetExistingFieldIDs(pilotFieldIDs);
		fDatabaseFile->RememberFields(newPilotFieldIDs, newPilotNames);
		fJFileIterator->GetDatabaseInfo(fJFileInfo);
	}
	return false;
#else
	return false;
#endif
}

void
CJFileSynchronizer::UpdateDatabaseInfo()
{
	DebugOutput::Output( "CJFileRecordIterator::UpdateDatabaseInfo" );
	fResolvedInfo = fJFileInfo;
	fResolvedInfo.fFlags |= INFOFLAGS_STRUCTLOCK;
	fResolvedInfo.fPopupValues = fFMInfo.fPopupValues;
	fResolvedInfo.fFieldTypes = fFMInfo.fFieldTypes;
	fResolvedInfo.fColumnWidths = fFMInfo.fColumnWidths;
	fResolvedInfo.fFieldNames = fFMInfo.fFieldNames;
	fResolvedInfo.fFieldAccess = fFMInfo.fFieldAccess;
	fResolvedInfo.fFieldExtraData = fFMInfo.fFieldExtraData;
	fResolvedInfo.fFieldExtraData2 = fFMInfo.fFieldExtraData2;
	fResolvedInfo.OverrideWith(fOverrides);
	if (!fDatabaseFile->NewInstall()) {
		CDatabaseInfo oldJFileInfo(fDatabaseFile->GetJFileInfo());
		CFieldOverrides changed(oldJFileInfo.CompareWith(fJFileInfo));
		fOverrides.OverrideWith(changed);
		fResolvedInfo.OverrideWith(fOverrides);
		fDatabaseFile->SetFieldOverrides(fOverrides);
	}
	fJFileIterator->SetDatabaseInfo(fResolvedInfo, fDatabase->GetFieldRepeats());
	fDatabaseFile->RememberJFileInfo(fResolvedInfo);
}

void
CJFileSynchronizer::SynchronizeSortInfoBlock()
{
}

void
CJFileSynchronizer::UpdateLastSyncTime(bool errorOccurred)
{
	if (!errorOccurred) {
		UInt32 secs;
		GetDateTime(&secs);
		fDatabaseFile->SetLastSyncTime(secs);
		fDatabaseFile->SetOverrideSchedule(false);
	}
}		

void
CJFileSynchronizer::SynchronizeOne(int remoteDBIndex)
{
	gSafeCancel = true;
	mTriedPostSync = false;
	bool updatedFile = false;
	
	fDatabaseFile = fSettings->GetDatabase(remoteDBIndex);
	if (fDatabaseFile->GetDisabled())
		return;
		
	UInt32 secs;
	GetDateTime(&secs);
	
	if (secs < fDatabaseFile->GetNextSyncTime()) {
		return;
	}
	
	try
	{
		ConfirmFreeStackSpace(); 
		
		if (fDatabaseFile->FileInTrash()) {
			PutErrorCodeIntoLog(kDatabaseInTrashError);
			return;
		}

		if (!fDatabaseFile->HasFile()) {
			PutErrorCodeIntoLog(fnfErr);
			return;
		}
		
		SetupForSyncMode();
		
		MakeIterators(remoteDBIndex);
		
		ThrowIfNil_(mPilotIterator);
		ThrowIfNil_(mLocalRecIterator);
		
		//do some preprocessing before beginning the sync
		DoPreSync();

	    switch (mSyncProperties.m_SyncType)
		{
		    case eFast:
			   DoFastSync();
			    break;
		    case eSlow:
			    DoSlowSync();
			    break;

		    case ePCtoHH:
			    DoReplaceRemoteSync();
			    break;

		    case eHHtoPC:
			    DoReplacePCSync();
			    break;

		    case eInstall:
		    case eProfileInstall:
		    case eBackup:
				
				break;
				
		    default: // do nothing
				break;
	    }

		//do any post processing before flushing
		bool useFoundSet = fDatabaseFile->GetUseFoundSet();
		bool hasPreSync = (fDatabaseFile->GetPreSyncScriptID().IsValid());
		bool wasCanceled = ExecuteTasks();
		DoPostSync(wasCanceled || fErrorOccurred);
		UpdateLastSyncTime(wasCanceled || fErrorOccurred);
		fSettings->UpdateFile();
		updatedFile = true;
		fReSync = !wasCanceled && !fErrorOccurred && fReSync && useFoundSet && hasPreSync;
		if (!fReSync && !fErrorOccurred) {
			string success(LoadString(kConverterErrorStrings, kHotSyncSucceededIndex));
			Substitute(success, "%%1", fDatabaseFile->GetPilotDatabaseName());
			::LogAddEntry(success.c_str(), slText, false);
		}
    }
    
	catch (const LException& inErr) {
		PutErrorCodeIntoLog(inErr.GetErrorCode());
		PutAppleEventErrorIntoLog();
		fErrorOccurred = true;
	}

	catch(std::bad_alloc) {
		fErrorOccurred = true;
		PutErrorCodeIntoLog(memFullErr);
	}

	catch(...) {
		fErrorOccurred = true;
		PutErrorCodeIntoLog(kUnknownError);
	}

	if( !mTriedPostSync )
		this->DoPostSync(true);
		
	if (!updatedFile) {
		fSettings->UpdateFile();
		updatedFile = true;
	}
	
	DeleteIterators();
}

bool
CJFileSynchronizer::LayoutIsOK(FileMaker::DatabasePtr theDB, int layoutID, string& string1, string& string2)
{
	string1 = string();
	string2 = string();
	try {
		if (!theDB->LayoutExists(layoutID)) {
			string1 = LoadString(kFMJErrorStrings, kLayoutNLEErrorIndex);
			return false;
		}

		theDB->UseLayout(layoutID);

		if (theDB->HasContainerFields()) {
			string1 = LoadString(kFMJErrorStrings, kNoContainerFieldsErrorIndex);
			string2 = LoadString(kFMJErrorStrings, kNoContainerFieldsErrorIndex+1);
			return false;
		}

		const int kMaxFields = MAX_FIELDS;
		int fieldCount = theDB->FieldCount();
		if (fieldCount > kMaxFields) {
			string1 = LoadString(kFMJErrorStrings, kTooManyFieldsErrorIndex);
			string2 = LoadString(kFMJErrorStrings, kTooManyFieldsErrorIndex+1);
			Substitute(string2, "%%1", AsString(fieldCount));
			Substitute(string2, "%%2", AsString(kMaxFields));
			return false;
		}
			
		return true;
	} catch (...) {
		string1 = LoadString(kFMJErrorStrings, kUnknownErrorIndex);
	}
	return false;
}

bool
CJFileSynchronizer::ScriptIsOK(FileMaker::DatabasePtr theDB, FMAE::ScriptID scriptID)
{
	if (scriptID.IsValid())
		return theDB->ScriptExists(scriptID);
	return true;
}

void
CJFileSynchronizer::PutTaskAndErrorIntoLog(CDataTask& theTask, ExceptionCode errorCode)
{
	if (errorCode == kSyncQuietAbortErr)
		return;
	PutErrorCodeIntoLog(errorCode);
	int actionCode = theTask.GetAction();
	static string whileTryingTo(LoadString(kFMJErrorStrings, kWhileTryingToIndex));
	whileTryingTo.append(LoadString(kTaskDescriptionStrings, actionCode+1));
	::LogAddEntry(whileTryingTo.c_str(), slWarning, false);
	switch(actionCode) {
	case CDataTask::kLocalAddToRemote:
		PutLocalDataHolderIntoLog(theTask);
		break;
	case CDataTask::kRemoteAddToLocal:
		PutFieldErrorIntoLog();
		PutAppleEventErrorIntoLog();
		PutRemoteDataHolderIntoLog(theTask);
		break;
	case CDataTask::kLocalReplaceRemote:
		PutLocalDataHolderIntoLog(theTask);
		break;
	case CDataTask::kRemoteReplaceLocal:
		PutFieldErrorIntoLog();
		PutAppleEventErrorIntoLog();
		PutRemoteDataHolderIntoLog(theTask);
		break;
	case CDataTask::kRemoteReplaceLocalDeleteRemote:
		PutFieldErrorIntoLog();
		PutAppleEventErrorIntoLog();
		PutRemoteDataHolderIntoLog(theTask);
		break;
	case CDataTask::kDeleteLocal:
		PutAppleEventErrorIntoLog();
		PutLocalDataHolderIntoLog(theTask);
		break;
	case CDataTask::kMerge:
	case CDataTask::kDuplicate:
	case CDataTask::kDuplicateDeleteRemote:
		PutFieldErrorIntoLog();
		PutAppleEventErrorIntoLog();
		PutLocalDataHolderIntoLog(theTask);
		PutRemoteDataHolderIntoLog(theTask);
		break;
	case CDataTask::kMergeDeleteRemote:
		PutRemoteDataHolderIntoLog(theTask);
		break;
	}
}

const int kMaxFieldNameDisplayLength = 28;

void
CJFileSynchronizer::TruncateLongField(string& fieldData) const
{
	const string::size_type kMaxLength(80);
	if (fieldData.length() > kMaxLength) {
		string::size_type remainder = fieldData.length() - kMaxLength;
		string replaceString("... [and ");
		replaceString.append(AsString(remainder));
		replaceString.append(" more characters]");
		fieldData.replace(kMaxLength, remainder, replaceString);
	}
}

void
CJFileSynchronizer::PutDataHolderIntoLog(CDataHolder& theHolder, const CFieldIDList& fieldIDs)
{
	vector<string> fieldData(theHolder.MakeOrderedDataVector(fieldIDs));
	map<FMAE::FieldID, string> fieldNameMap(fDatabase->GetAllFieldsNameMap());
	
	CFieldIDList::const_iterator i = fieldIDs.begin();
	vector<string>::const_iterator j = fieldData.begin();
	while (i != fieldIDs.end()) {
		string fieldName(fieldNameMap[*i]);
		if (fieldName.length() > kMaxFieldNameDisplayLength) {
			fieldName = fieldName.substr(0, kMaxFieldNameDisplayLength);
		}
		string fieldDisplay(4, ' ');
		fieldDisplay.append(fieldName);
		fieldDisplay.append(":");
		if (fieldDisplay.length() < kMaxFieldNameDisplayLength) {
			string spacer(kMaxFieldNameDisplayLength - fieldDisplay.length(), ' ');
			fieldDisplay.append(spacer);
		}
		string fieldData(*j);
		ConvertToVerticalTab(fieldData);
		fieldDisplay.append(fieldData);
		TruncateLongField(fieldDisplay);
		::LogAddEntry(fieldDisplay.c_str(), slWarning, false);
		++i;
		++j;
	}
	::LogAddEntry("", slWarning, false);
}

void
CJFileSynchronizer::PutRemoteDataHolderIntoLog(CDataTask& theTask)
{
	static string remoteDataLabel(LoadString(kFMJErrorStrings, kRemoteRecordIDIndex));
	remoteDataLabel.append(AsString(theTask.fRemote.fRecordID));
	::LogAddEntry(remoteDataLabel.c_str(), slWarning, false);
	static string remoteDataFollowsLabel(LoadString(kFMJErrorStrings, kRemoteRecordDataIndex));
	::LogAddEntry(remoteDataFollowsLabel.c_str(), slWarning, false);
	PutDataHolderIntoLog(theTask.fRemote, fRemoteFieldIDs);
}

void
CJFileSynchronizer::PutLocalDataHolderIntoLog(CDataTask& theTask)
{
	static string localDataLabel(LoadString(kFMJErrorStrings, kLocalRecordIDIndex));
	localDataLabel.append(AsString(theTask.fLocal.fRecordID));
	::LogAddEntry(localDataLabel.c_str(), slWarning, false);
	static string remoteDataFollowsLabel(LoadString(kFMJErrorStrings, kLocalRecordDataIndex));
	::LogAddEntry(remoteDataFollowsLabel.c_str(), slWarning, false);
	PutDataHolderIntoLog(theTask.fLocal, fRemoteFieldIDs);
}

void
CJFileSynchronizer::PutFieldErrorIntoLog()
{
	FMAE::FieldID errorField(fDatabase->LastFieldWritten());
	if (errorField.IsValid()) {
		static string errorOccurredWhileWriting(LoadString(kFMJErrorStrings, kErrorOccurredWhileIndex));
		map<FMAE::FieldID, string> fieldNameMap(fDatabase->GetAllFieldsNameMap());
		errorOccurredWhileWriting.append(fieldNameMap[errorField]);
		errorOccurredWhileWriting.append(1, 'Ó');
		::LogAddEntry(errorOccurredWhileWriting.c_str(), slWarning, false);
	}
}

void
CJFileSynchronizer::PutAppleEventErrorIntoLog()
{
	if (StAppleEvent::gLastErrorString.length() > 0) {
		static string appleEventReturned(LoadString(kFMJErrorStrings, kAppleEventReturnedIndex));
		appleEventReturned.append(StAppleEvent::gLastErrorString.c_str());
		appleEventReturned.append("Ó");
		::LogAddEntry(appleEventReturned.c_str(), slWarning, false);
		StAppleEvent::gLastErrorString.clear();
	}
}

void
CJFileSynchronizer::PutErrorCodeIntoLog(ExceptionCode errorCode)
{
	if (errorCode == kSyncQuietAbortErr)
		return;

	string databaseName(fDatabaseFile->GetPilotDatabaseName());
	static string errorStr(LoadString(kConverterErrorStrings, kHotSyncFailedIndex));
	errorStr.append(databaseName);
	errorStr.append(LoadString(kConverterErrorStrings, kHotSyncFailed2Index));
	errorStr.append(ConvertErrorToString(errorCode));
	::LogAddEntry(errorStr.c_str(), slWarning, false);
	if (errorCode == kCannotFindFileMakerErr) {
		static string howToFix(LoadString(kFMJErrorStrings, kRebuildDesktopIndex));
		::LogAddEntry(howToFix.c_str(), slWarning, false);
	}
}

void
CJFileSynchronizer::PutErrorCodeIntoLogNoFile(ExceptionCode errorCode)
{
	if (errorCode == kSyncQuietAbortErr)
		return;
		
	static string errorStr(LoadString(kConverterErrorStrings, kHotSyncFailedNoFileIndex));
	errorStr.append(ConvertErrorToString(errorCode));
	::LogAddEntry(errorStr.c_str(), slWarning, false);
}

void
CJFileSynchronizer::CheckExistingFields(const CFieldIDList& existingFieldIDs, const vector<int>& repeats)
{
	fExistingFieldDeleted = false;
	fExistingLocalFields.clear();
	fExistingLocalRepeats.clear();
	CFieldIDList::const_iterator i = existingFieldIDs.begin();
	vector<int>::const_iterator j = repeats.begin();
	while (i != existingFieldIDs.end()) {
		if (!fDatabase->FieldExists(*i)) {
			fExistingFieldDeleted = true;
			static string warningString(LoadString(kFMJErrorStrings, kFieldDeletedIndex));
			::LogAddEntry(warningString.c_str(), slWarning, false);
			return;
		} else {
			fExistingLocalFields.push_back(*i);
			fExistingLocalRepeats.push_back(*j);
		}
		++i;
		++j;
	}
}

void
CJFileSynchronizer::PutTabbedDataIntoLog(CDataHolder& holder)
{
	if (!fExistingFieldDeleted)
		return;
		
	vector<string> fieldData(holder.MakeOrderedDataVector(fExistingFieldIDs));
	if (fieldData.size() < 1)
		return;
	
	vector<string>::const_iterator j = fieldData.begin();
	string dataString(*j);
	++j;
	while (j != fieldData.end()) {
		dataString.append(1, '\t');
		string fieldData(*j);
		ConvertToVerticalTab(fieldData);
		dataString.append(fieldData);
		++j;
	}
	::LogAddEntry(dataString.c_str(), slWarning, false);
}

void
CJFileSynchronizer::OutputChangesAsTabbed()
{
	::LogAddEntry("Changed records follow:", slWarning, false);
 
 	vector<int> deleted;
	CSyncRecord remoteRecord;
	while (mPilotIterator->NextRecord(eFast, remoteRecord)) //allocates
	{
		if (remoteRecord.IsDeleted()) {
		#if 0
			CSyncRecord localRecord;
			UInt32 remoteRecID = remoteRecord.GetRemoteRecordID();
			
			//grab matching local record if there is one
			if (mLocalRecIterator->GetRecordByID(remoteRecID, eFast, localRecord))
				deleted.push_back(localRecord.GetLocalRecordID());
		#endif
			continue;
		}
			
		CDataHolder remoteHolder;
		Assert_(remoteRecord.RequiredFieldsExist());
		MakeDataHolder(remoteHolder, remoteRecord, fExistingFieldIDs, false);
		vector<string> fieldData(remoteHolder.MakeOrderedDataVector(fExistingFieldIDs));
		if (fieldData.size() < 1)
			continue;
			
		vector<string>::const_iterator j = fieldData.begin();
		string dataString(*j);
		++j;
		while (j != fieldData.end()) {
			dataString.append(1, '\t');
			string fieldData(*j);
			ConvertToVerticalTab(fieldData);
			dataString.append(fieldData);
			++j;
		}

		::LogAddEntry(dataString.c_str(), slWarning, false);
		remoteRecord.RemoveFields();
	}
	
	if (deleted.size() > 0) {
		::LogAddEntry("Ê", slWarning, false);
		string deletedString("Deleted record IDs: ");
		vector<int>::const_iterator i = deleted.begin();
		deletedString.append(AsString(*i));
		++i;
		while (i != deleted.end()) {
			deletedString.append(1, ',');
			deletedString.append(1, ' ');
			deletedString.append(AsString(*i));
			++i;
		}
		::LogAddEntry(deletedString.c_str(), slWarning, false);
	}
}

int
CJFileSynchronizer::JFileFromFMType(int fieldType, const string& sampleData, bool hasChoices)
{
	return CFMFieldTyper::JFileFromFMType(fieldType, sampleData, hasChoices);
}

map<FMAE::FieldID,string>
CJFileSynchronizer::GetDefaultFieldNameMap()
{
	return MakeMap(fDatabase->GetFieldIDs(), fDatabase->GetFieldNames());
}

map<FMAE::FieldID,int> 
CJFileSynchronizer::GetDefaultFieldTypeMap()
{
	return MakeMap(fDatabase->GetFieldIDs(), fDatabase->GetFieldTypes());
}

map<FMAE::FieldID,int> 
CJFileSynchronizer::GetDefaultFieldWidthMap()
{
	return MakeMap(fDatabase->GetFieldIDs(), fDatabase->GetFieldWidths());
}

map<FMAE::FieldID,int> 
CJFileSynchronizer::GetDefaultFieldReadOnlyMap()
{
	return MakeMap(fDatabase->GetFieldIDs(), fDatabase->GetFieldReadOnly());
}

map<FMAE::FieldID,string_vector> 
CJFileSynchronizer::GetDefaultChoicesMap()
{
	return MakeMap(fDatabase->GetFieldIDs(), fDatabase->GetFieldChoices());
}

// SyncNormal, SyncUploadAndClear, SyncClearAndDownload

void CJFileSynchronizer::SetupForSyncMode()
{
	switch(fDatabaseFile->GetSyncMode()) {
	default:
	case CDatabaseFile::SyncNormal:
		fIgnorePalmRecords = false;
		fIgnoreFileMakerRecords = false;
		fDeletePalmRecordsBeforeSync = false;
		fDeletePalmRecordsAfterSync = false;
		break;
	case CDatabaseFile::SyncUploadAndClear:
		fIgnorePalmRecords = false;
		fIgnoreFileMakerRecords = true;
		fDeletePalmRecordsBeforeSync = false;
		fDeletePalmRecordsAfterSync = true;
		break;
	case CDatabaseFile::SyncClearAndDownload:
		fIgnorePalmRecords = true;
		fIgnoreFileMakerRecords = false;
		fDeletePalmRecordsBeforeSync = true;
		fDeletePalmRecordsAfterSync = false;
		break;
	}
}

void
CJFileSynchronizer::RecordJFileNames()
{
	const int kDatabaseListSize = 256;
	vector<CDbList> theDB(kDatabaseListSize);
	Byte cardNum = 0;
	bool bRam = false;
	int count;
	
	vector<string> names;
	vector<string> palmNames;
	int error;
	int loopCount = 0;
	do {
		Word startIDX = 0;
		do {
			count = theDB.size();
			error = SyncReadDBList(cardNum, startIDX, bRam, &theDB[0], count);
			if (error == SYNCERR_NONE) {
				for (int i = 0; i < count; ++i) {
					if (theDB[i].m_Creator == CWriteJFile5::kCreator && theDB[i].m_DbType == CWriteJFile5::kType) {
						names.push_back(theDB[i].m_Name);
					}
					palmNames.push_back(theDB[i].m_Name);
					if (theDB[i].m_Creator == CWriteJFilePro::kCreator && theDB[i].m_DbType == CWriteJFilePro::kType) {
						names.push_back(theDB[i].m_Name);
						fJFileProDatabases[theDB[i].m_Name] = true;
					}
				}
			}
			startIDX += count;
		} while(error == SYNCERR_NONE && count == theDB.size());
		loopCount += 1;
		bRam = !bRam;
	} while (loopCount < 2);
	fSettings->SetAllJFileNames(names);
	fSettings->SetPalmNames(palmNames);
	UInt32 seconds;
	ReadDateTime(&seconds);
	fSettings->SetLastSyncTime(seconds);
}

void
CJFileSynchronizer::UploadDatabases()
{
	CDatabaseUpload uploader(fSettings, fJFileProDatabases);
	uploader.UploadDatabases();
}

void
CJFileSynchronizer::PickUnusedName()
{
	if (fDatabaseFile->NewInstall()) {
		CUniqueNamer namer(fSettings->GetPalmNames());
		string originalName(fDatabaseFile->GetPilotDatabaseName());
		if (!namer.NameIsOK(originalName)) {
			string newName(namer.MakeValidName(originalName));
			fDatabaseFile->SetPilotDatabaseName(newName);
			string warningString(LoadString(kConverterErrorStrings, kNameInUsendex));
			Substitute(warningString, "%%1", originalName);
			Substitute(warningString, "%%2", newName);
			::LogAddEntry(warningString.c_str(), slWarning, false);
		}
	}
}
