#include "CDatabaseUpload.h"
#include "CUploadRequest.h"
#include "CConduitSettings.h"
#include "ErrorStrings.h"
#include "ErrorCodes.h"
#include "Utilities.h"
#include "Str255.h"
#include "hslog.h"
#include "CFileMaker.h"
#include "WriteDIF.h"
#include "CDatabaseInfo.h"
#include "CDatabase.h"
#include "SyncMgr.h"
#include "CSynchronizer.h"
#include "CJFileRecordIterator.h"
#include "ReadJFilePro.h"
#include "ReadJFile5.h"
//#include "ReadJFile2.h"

class CUploadOneDatabase
{
public:
	CUploadOneDatabase(const CUploadRequest&, bool jFilePro);
	~CUploadOneDatabase();
	
	bool Upload();
	void PutErrorCodeIntoLog(ExceptionCode errorCode);

private:
	void DoUpload();
	void OpenDatabase();
	void UploadDatabaseInfo();
	void UploadData();
	void OutputAsDIF();
	void CloseDatabase();

	CUploadRequest	fReq;
	CDatabase		fDatabase;
	CFieldIDList	fFieldIDs;
	Byte			fDatabaseHandle;
	bool			fJFilePro;
	int				fFieldCount;
	bool			fTranslateText;
	
};


CUploadOneDatabase::CUploadOneDatabase(const CUploadRequest& r, bool jFilePro)
	: fReq(r), fDatabaseHandle(0), fJFilePro(jFilePro)
{
	for(int i = 1; i <= 50; i+= 1)
		fFieldIDs.push_back(FMAE::FieldID(i));
}

CUploadOneDatabase::~CUploadOneDatabase()
{
	CloseDatabase();
}

void
CUploadOneDatabase::CloseDatabase()
{
	if (fDatabaseHandle) {
		SyncCloseDB(fDatabaseHandle);
		fDatabaseHandle = 0;
	}
}

void
CUploadOneDatabase::OpenDatabase()
{
	ThrowIfErrorC_(::SyncOpenDB(fReq.fName.c_str(), 0, fDatabaseHandle));
}

void
CUploadOneDatabase::DoUpload()
{
	OpenDatabase();
	UploadDatabaseInfo();
	UploadData();
	OutputAsDIF();
}

void
CUploadOneDatabase::UploadDatabaseInfo()
{
	const vector<int> kMaxRepeats(CReadJFile5::kMaxFields, 1);
	const int kMaxAppInfoBuffer = 10240;
	CDatabaseInfo info;
	vector<char> appInfoBuffer(kMaxAppInfoBuffer);
	CDbGenInfo appInfo = {};
	appInfo.m_pBytes = (BYTE*)&appInfoBuffer[0];
	appInfo.m_TotalBytes = appInfoBuffer.size();
	ThrowIfErrorC_(::SyncReadDBAppInfoBlock(fDatabaseHandle, appInfo));
	if (fJFilePro)
		CReadJFilePro::ExtractFromAppInfo(&appInfoBuffer[0], fReq.fTranslateText, fFieldIDs, info);
	else
		CReadJFile5::ExtractFromAppInfo(&appInfoBuffer[0], appInfo.m_BytesRead, fReq.fTranslateText, fFieldIDs, kMaxRepeats, info);

	fFieldCount = info.fFieldCount;
	fFieldIDs.erase(fFieldIDs.begin() + fFieldCount, fFieldIDs.end());
	vector<string> fieldNames(info.GetFieldNames(fFieldIDs));
	fDatabase.SetFieldNames(fieldNames);
}

void
CUploadOneDatabase::UploadData()
{
	long syncErr = 0;
	long pilotRecIndex = 0;
	CRawRecordInfo rawRecInfo = {};
	vector<char> vec(10240);
	
	rawRecInfo.m_FileHandle = fDatabaseHandle;
	rawRecInfo.m_TotalBytes = vec.size();
	rawRecInfo.m_pBytes = (Byte*)&vec[0];

	do
	{
		if (fReq.fChanges) {
			syncErr = ::SyncReadNextModifiedRec(rawRecInfo);
			if (syncErr != SYNCERR_FILE_NOT_FOUND)
				ThrowIfErrorC_(syncErr);
		} else {
			rawRecInfo.m_RecIndex = pilotRecIndex;
			syncErr = ::SyncReadRecordByIndex(rawRecInfo);
			if (syncErr != SYNCERR_FILE_NOT_FOUND)
				ThrowIfErrorC_(syncErr);
			++pilotRecIndex;
		}
			
		if (( (rawRecInfo.m_Attribs & kPilotAttrDeleted) || rawRecInfo.m_RecSize > 0 ) )
		{
			LDataStream stream(&vec[0], rawRecInfo.m_TotalBytes);
			vector<string> fields;
			vector<int> repeats(50, 1);
			if (fJFilePro)
				CReadJFilePro::ReadRecord(&stream, fields, fFieldCount, fReq.fTranslateText);
			else
				CReadJFile5::ReadRecord(&stream, fields, fFieldCount, fReq.fTranslateText, repeats);
			fDatabase.AppendFields(fields);
		}
	} while (syncErr != SYNCERR_FILE_NOT_FOUND && ( (rawRecInfo.m_Attribs & kPilotAttrDeleted) || rawRecInfo.m_RecSize > 0 ) );
}

void
CUploadOneDatabase::OutputAsDIF()
{
	FSSpec spec;
	fReq.GetLocation(spec);
	LFileStream fs(spec);
	
	fs.CreateNewDataFile(kUSFileMakerCreator, 'TEXT');
	fs.OpenDataFork(fsRdWrPerm);
	
	CWriteDIF writer(&fDatabase);
	writer.DoWrite(&fs, AsStr255(fReq.fName));
}

bool
CUploadOneDatabase::Upload()
{
	try {
		DoUpload();
		return true;
	}
	
	catch (const LException& inErr) {
		PutErrorCodeIntoLog(inErr.GetErrorCode());
	}

	catch(std::bad_alloc) {
		PutErrorCodeIntoLog(memFullErr);
	}
	
	catch(...) {
		PutErrorCodeIntoLog(kUnknownError);
	}
	
	return false;
}

void
CUploadOneDatabase::PutErrorCodeIntoLog(ExceptionCode errorCode)
{
	string databaseName(fReq.fName);
	static string errorStr(LoadString(kConverterErrorStrings, kUploadFailedIndex));
	errorStr.append(fReq.fName);
	errorStr.append(LoadString(kConverterErrorStrings, kUploadFailed2Index));
	errorStr.append(ConvertErrorToString(errorCode));
	::LogAddEntry(errorStr.c_str(), slWarning, false);
}


CDatabaseUpload::CDatabaseUpload(CConduitSettings* settings, const map<string,bool>& m)
	: fSettings(settings), fProDBs(m)
{
}

CDatabaseUpload::~CDatabaseUpload()
{
}

void
CDatabaseUpload::UploadDatabases()
{
	vector<CUploadRequest> reqs(fSettings->GetUploadRequests());
	vector<CUploadRequest> failedReqs;
	
	for (vector<CUploadRequest>::const_iterator i = reqs.begin(); i != reqs.end(); ++i) {
		CUploadOneDatabase doOne(*i, fProDBs[i->fName]);
		if (!doOne.Upload()) {
			failedReqs.push_back(*i);
		}
	}
	fSettings->SetUploadRequests(failedReqs);
}
