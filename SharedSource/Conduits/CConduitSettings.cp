#include <LFileStream.h>

#include "CConduitSettings.h"
#include "MoreFilesExtras.h"
#include "CDatabaseFile.h"
#include "BinaryFormat.h"
#include "PaneUtilities.h"
#include "FolderUtilities.h"
#include "TempFileUtilities.h"
#include "Utilities.h"
#include "DebugOutput.h"
#include "CFileMaker.h"
#include "WarningsStrings.h"
#include "Str255.h"

class CConduitSettingsHeader {
public:
	CConduitSettingsHeader(int count, int syncAction, int defaultSyncAction, int quitMode);
	
	bool	Valid(UInt32 versionNumber);
	UInt32 fMagic;
	UInt32 fVersion;
	UInt32 fCount;
	UInt32 fSyncAction;
	UInt32 fDefaultSyncAction;
	UInt32 fQuitMode;
};

CConduitSettingsHeader::CConduitSettingsHeader(int count, int syncAction, int defaultSyncAction, int quitMode)
	: fMagic(CConduitSettings::kMagic), fVersion(CConduitSettings::kCurrentVersion), fCount(count), fSyncAction(syncAction), 
	fDefaultSyncAction(defaultSyncAction), fQuitMode(quitMode)
{
}

bool
CConduitSettingsHeader::Valid(UInt32 versionNumber)
{
	return fMagic = CConduitSettings::kMagic && fVersion == versionNumber;
}

CConduitSettings::CConduitSettings(const FSSpec* spec)
	: fSyncAction(kSyncDatabases), fDefaultSyncAction(kSyncDatabases), fLastSyncTime(0), fQuitMode(CFileMaker::kNeverQuit), fAccessMode(kAccessByFile)
{
	//DEBUG_OUT_DEF("CConduitSettings::CConduitSettings(const FSSpec* spec)\r");
	OSErr err = ::FSMakeFSSpec(spec->vRefNum, spec->parID, spec->name,
					&fFile);
	if (err != noErr && err != fnfErr)
		Throw_(err);

	if (err != fnfErr) {
		LFileStream aFile(fFile);
		aFile.OpenDataFork(fsRdPerm);
		CConduitSettingsHeader header(0, fSyncAction, fDefaultSyncAction, fQuitMode);
		aFile.ReadBlock(&header, sizeof(header));
		if (header.Valid(kCurrentVersion)) {
			ReadCurrentVersion(header, aFile);
		} else if (header.Valid(k115Version)) {
			ReadVersion115(header, aFile);
		}
	}
}


CConduitSettings::~CConduitSettings()
{
}

void
CConduitSettings::ReadVersion115(CConduitSettingsHeader& header, LFileStream& aFile)
{
	fSyncAction = header.fSyncAction;
	fDefaultSyncAction = header.fDefaultSyncAction;
	fQuitMode = header.fQuitMode;
	CFileMaker::SetQuitMode(fQuitMode);
	BinaryFormat::Read(aFile, fWarnings);
	BinaryFormat::Read(aFile, fRegCode);
	BinaryFormat::Read(aFile, fJFileDatabases);
	BinaryFormat::Read(aFile, fPalmDatabases);
	BinaryFormat::Read(aFile, fUploadRequests);
	BinaryFormat::Read(aFile, fLastSyncTime);
	int count = header.fCount;
	while (--count >= 0) {
		CDatabaseFile::Ptr f(new CDatabaseFile(aFile, header.fVersion));
		AddDatabase(f);
	}
}

void
CConduitSettings::ReadCurrentVersion(CConduitSettingsHeader& header, LFileStream& aFile)
{
	fSyncAction = header.fSyncAction;
	fDefaultSyncAction = header.fDefaultSyncAction;
	fQuitMode = header.fQuitMode;
	CFileMaker::SetQuitMode(fQuitMode);
	BinaryFormat::Read(aFile, fWarnings);
	BinaryFormat::Read(aFile, fRegCode);
	BinaryFormat::Read(aFile, fJFileDatabases);
	BinaryFormat::Read(aFile, fPalmDatabases);
	BinaryFormat::Read(aFile, fUploadRequests);
	BinaryFormat::Read(aFile, fLastSyncTime);
	BinaryFormat::Read(aFile, fAccessMode);
	int count = header.fCount;
	while (--count >= 0) {
		CDatabaseFile::Ptr f(new CDatabaseFile(aFile, header.fVersion));
		AddDatabase(f);
	}
}

void
CConduitSettings::UpdateFile()
{
	bool usingTempFile = false;
	FSSpec dest;

	OSErr err = ::FSMakeFSSpec(fFile.vRefNum, fFile.parID, fFile.name,
					&dest);

	if (err != noErr && err != fnfErr)
		Throw_(err);

	if (err != fnfErr) {
		GetTemporaryFileNear(fFile, &dest);
	}

	LFileStream aFile(dest);
	aFile.CreateNewDataFile('cSet', 'pref');
	aFile.OpenDataFork(fsRdWrPerm);
	CConduitSettingsHeader header(fDatabases.size(), fSyncAction, fDefaultSyncAction, fQuitMode);
	aFile.WriteBlock(&header, sizeof(header));
	BinaryFormat::Write(aFile, fWarnings);
	BinaryFormat::Write(aFile, fRegCode);
	BinaryFormat::Write(aFile, fJFileDatabases);
	BinaryFormat::Write(aFile, fPalmDatabases);
	BinaryFormat::Write(aFile, fUploadRequests);
	BinaryFormat::Write(aFile, fLastSyncTime);
	BinaryFormat::Write(aFile, fAccessMode);
	DatabaseList::iterator i = fDatabases.begin();
	while (i != fDatabases.end()) {
		(*i)->WriteToStream(aFile);
		i++;
	}
	aFile.CloseDataFork();
	if (err != fnfErr) {
		ThrowIfOSErr_(FSpExchangeFiles(&fFile, &dest));
		FSpDelete(&dest);
	}
}

void
CConduitSettings::AddDatabase(CDatabaseFile::Ptr f)
{
	fDatabases.push_back(f);
	sort(fDatabases.begin(), fDatabases.end(), CompareDatabaseFileByName());
}

CDatabaseFile::Ptr
CConduitSettings::FindDatabaseByRemoteName(const string& remoteFileName)
{
	DatabaseList::iterator i = fDatabases.begin();
	while (i != fDatabases.end()) {
		if (cmp_nocase(remoteFileName, (*i)->GetPilotDatabaseName()) == 0)
			return *i;
		++i;
	}
	return CDatabaseFile::Ptr(0);
}

vector<string>
CConduitSettings::GetAllRemoteNames(CDatabaseFile::Ptr exceptThis)
{
	vector<string> allNames;
	for (DatabaseList::iterator i = fDatabases.begin(); i != fDatabases.end(); ++i) {
		if (*i != exceptThis) {
			allNames.push_back((*i)->GetPilotDatabaseName());
		}
	}

	bool exceptIsNewInstall = false;
	string exceptPilotName;
	if (exceptThis.get()) {
		exceptPilotName = exceptThis->GetPilotDatabaseName();
		exceptIsNewInstall = exceptThis->NewInstall();
	}

	for (vector<string>::const_iterator i = fPalmDatabases.begin(); i != fPalmDatabases.end(); ++i) {
		if (exceptThis.get() == 0 || exceptIsNewInstall || cmp_nocase(*i, exceptPilotName)) {
			allNames.push_back(*i);
		}
	}
	
	return allNames;
}

CDatabaseFile::Ptr
CConduitSettings::FindDatabase(const FSSpec& thisFile)
{
	DatabaseList::iterator i = fDatabases.begin();
	while (i != fDatabases.end()) {
		FSSpec fileSpec;
		(*i)->GetFSSpec(&fileSpec);
		if (LFile::EqualFileSpec(thisFile, fileSpec))
			return *i;
		++i;
	}
	return CDatabaseFile::Ptr(0);
}

void
CConduitSettings::CheckRegistration(const string& userName)
{
}

int
CConduitSettings::DatabaseCount()
{
	return fDatabases.size();
}

CDatabaseFile::Ptr
CConduitSettings::GetDatabase(int index)
{
	return fDatabases.at(index);
}

void
CConduitSettings::RemoveDatabase(CDatabaseFile::Ptr f)
{
	DatabaseList::iterator i = find(fDatabases.begin(), fDatabases.end(), f);
	if (i != fDatabases.end()) {
		fDatabases.erase(i);
	}
}

int
CConduitSettings::SyncAction()
{
	return fSyncAction;
}

void
CConduitSettings::SetSyncAction(int actionCode)
{
	fSyncAction = actionCode;
}

int
CConduitSettings::DefaultSyncAction()
{
	return fDefaultSyncAction;
}

void
CConduitSettings::SetDefaultSyncAction(int actionCode)
{
	fDefaultSyncAction = actionCode;
}

int
CConduitSettings::QuitMode()
{
	return fQuitMode;
}

void
CConduitSettings::SetQuitMode(int mode)
{
	fQuitMode = mode;
	CFileMaker::SetQuitMode(fQuitMode);
}

string
CConduitSettings::RegCode()
{
	return fRegCode;
}

void
CConduitSettings::SetRegCode(const string& regCode)
{
	fRegCode = regCode;
}

vector<CUploadRequest>
CConduitSettings::GetUploadRequests() const
{
	return fUploadRequests;
}

void
CConduitSettings::SetUploadRequests(const vector<CUploadRequest>& r)
{
	fUploadRequests = r;
}

vector<string>
CConduitSettings::GetJFileNames() const
{
	return fJFileDatabases;
}

void
CConduitSettings::SetAllJFileNames(const vector<string>& i)
{
	fJFileDatabases = i;
}

vector<string>
CConduitSettings::GetPalmNames() const
{
	return fPalmDatabases;
}

void
CConduitSettings::SetPalmNames(const vector<string>& i)
{
	fPalmDatabases = i;
}

UInt32
CConduitSettings::GetLastSyncTime() const
{
	return fLastSyncTime;
}

void
CConduitSettings::SetLastSyncTime(UInt32 i)
{
	fLastSyncTime = i;
}

bool
CConduitSettings::WarningDisabled(WarningID warningID) const
{
	return fWarnings.Disabled(warningID);
}

void
CConduitSettings::SetWarningDisabled(WarningID warningID, bool warned)
{
	fWarnings.SetDisabled(warningID, warned);
}

CWarnings
CConduitSettings::GetWarnings() const
{
	return fWarnings;
}

void
CConduitSettings::SetWarnings(const CWarnings& w)
{
	fWarnings = w;
}
