#include "CDatabaseFile.h"
#include "CConduitSettings.h"
#include "Utilities.h"
#include "BinaryFormat.h"
#include "CDatabase.h"
#include "DescribeFiles.h"
#include "FolderUtilities.h"
#include "Stringiness.h"
#include "Str255.h"
#include "ErrorCodes.h"
#include "FMAE.h"
#ifdef CONDUIT
#include "CSyncRecord.h"
#include "DebugOutput.h"
#endif

#include <LStream.h>

using Folders::GetFileType;

CDatabaseFile::CDatabaseFile(const FSSpec& inFile)
	: fAlias(0), fArchiveFieldID(0), fLayoutID(0), 
		fUseFoundSet(false), fResolveErr(fnfErr), fDisabled(false), fJFileVersion(JFile5),
		fDefaultToInt(true), fDefaultToDate(true), fSyncMode(SyncNormal), fDuplicateOnConflict(false),
		fInTrash(false), fByName(false), fLastSyncTime(0), fSyncSchedule(kEveryTime), fOverrideSchedule(false)
{
	SetDefaultTranslateText();
	fResolveErr = ::FSMakeFSSpec(inFile.vRefNum, inFile.parID, inFile.name, &fFile);
	fAlias = CreateAliasToFile();
	fPilotName = AsString(fFile.name);
	SetDatabaseNameFromFile(inFile);
	Initialize();
}

CDatabaseFile::CDatabaseFile(const CDatabaseName& filemakerName)
	: fAlias(0), fArchiveFieldID(0), fLayoutID(0), fFileMakerName(filemakerName),
		fUseFoundSet(false), fResolveErr(fnfErr), fDisabled(false), fJFileVersion(JFile5),
		fDefaultToInt(true), fDefaultToDate(true), fSyncMode(SyncNormal), fDuplicateOnConflict(false),
		fInTrash(false), fByName(true), fLastSyncTime(0), fSyncSchedule(kEveryTime), fOverrideSchedule(false)
{
	SetDefaultTranslateText();
	fAlias = (AliasHandle)NewHandle(0);
	fPilotName = filemakerName.GetName();
	Initialize();
}

CDatabaseFile::CDatabaseFile(AliasHandle inAlias)
	: fAlias(inAlias), fArchiveFieldID(0), fLayoutID(0), 
		fUseFoundSet(false), fDisabled(false), 
		fJFileVersion(JFile5),
		fDefaultToInt(true), fDefaultToDate(true), fSyncMode(SyncNormal), fDuplicateOnConflict(false),
		fInTrash(false), fByName(false), fLastSyncTime(0), fSyncSchedule(kEveryTime), fOverrideSchedule(false)
{
	SetDefaultTranslateText();
	Boolean wasChanged;
	short fileCount = 1;
	int rulesMask = kARMNoUI | kARMSearch;
	fResolveErr = ::MatchAlias(0, rulesMask, fAlias, &fileCount, &fFile, &wasChanged, 0, 0);
	fPilotName = AsString(fFile.name);
	SetDatabaseNameFromFile(fFile);
	Initialize();
}

CDatabaseFile::CDatabaseFile(LStream& inStream, int version)
	: fArchiveFieldID(0), fLayoutID(0),
		fDefaultToInt(true), fDefaultToDate(true), fSyncMode(SyncNormal), fDuplicateOnConflict(false),
		fJFileVersion(JFile5),
		fInTrash(false), fByName(false), fLastSyncTime(0), fSyncSchedule(kEveryTime), fOverrideSchedule(false)
{
	SetDefaultTranslateText();
	ReadCurrentVersion(inStream);
	Initialize();
}

CDatabaseFile::~CDatabaseFile()
{
	ForgetAlias();
}

void CDatabaseFile::ReadCurrentVersion(LStream& inStream)
{
	Handle h;
	inStream >> h;
	fAlias = (AliasHandle)h;
	if (GetHandleSize(h) > 0) {
		Boolean wasChanged;
		int rulesMask = kARMNoUI | kARMSearch;
		short fileCount = 1;
		fResolveErr = ::MatchAlias(0, rulesMask, fAlias, &fileCount, &fFile, &wasChanged, 0, 0);
	} else {
		fByName = true;
		fResolveErr = 0;
	}
	
	BinaryFormat::Read(inStream, fFileMakerName);
	BinaryFormat::Read(inStream, fPilotName);
	BinaryFormat::Read(inStream, fNewPilotName);
	if (!fByName && fResolveErr == noErr)
		SetDatabaseNameFromFile(fFile);
	inStream >> fArchiveFieldID;
	inStream >> fLayoutID;
	BinaryFormat::Read(inStream, fPreSyncScriptID);
	BinaryFormat::Read(inStream, fPostSyncScriptID);
	BinaryFormat::Read(inStream, fUserNameField);
	inStream >> fTranslateText;
	inStream >> fDefaultToInt;
	inStream >> fDefaultToDate;
	inStream >> fJFileVersion;
	inStream >> fSyncMode;
	inStream >> fUseFoundSet;
	inStream >> fDisabled;
	inStream >> fDuplicateOnConflict;
	inStream >> fNewInstall;
	BinaryFormat::Read(inStream, fJFileInfo);
	BinaryFormat::Read(inStream, fFieldOverrides);
	BinaryFormat::Read(inStream, fFieldList);
	BinaryFormat::Read(inStream, fFieldRepeats);
	inStream >> fLastSyncTime;
	inStream >> fSyncSchedule;
	inStream >> fOverrideSchedule;

	long itemCount;
	inStream >> itemCount;
	for (int i = 0; i < itemCount; i += 1) {
		UInt32 recordID, fmRecordID;
		inStream >> fmRecordID;
		inStream >> recordID;
		string digest;
		BinaryFormat::Read(inStream, digest);
		CRecordInfo info(recordID, digest);
		fRecordMap[fmRecordID] = info;
	}
}

void CDatabaseFile::ReadVersion115(LStream& inStream)
{
	Handle h;
	inStream >> h;
	fAlias = (AliasHandle)h;
	Boolean wasChanged;
	int rulesMask = kARMNoUI | kARMSearch;
	short fileCount = 1;
	fResolveErr = ::MatchAlias(0, rulesMask, fAlias, &fileCount, &fFile, &wasChanged, 0, 0);
	long itemCount;
	string s;
	BinaryFormat::Read(inStream, s);
	BinaryFormat::Read(inStream, fPilotName);
	BinaryFormat::Read(inStream, fNewPilotName);
	if (fResolveErr == noErr) {
		fFileMakerName = CDatabaseName(AsString(fFile.name), GetFileType(fFile));
	}
	inStream >> fArchiveFieldID;
	inStream >> fLayoutID;
	BinaryFormat::Read(inStream, fPreSyncScriptID);
	BinaryFormat::Read(inStream, fPostSyncScriptID);
	inStream >> fTranslateText;
	inStream >> fDefaultToInt;
	inStream >> fDefaultToDate;
	inStream >> fJFileVersion;
	inStream >> fSyncMode;
	inStream >> fUseFoundSet;
	inStream >> fDisabled;
	inStream >> fDuplicateOnConflict;
	inStream >> fNewInstall;
	CDatabaseInfo::Read115(inStream, fJFileInfo);
	CFieldOverrides::Read115(inStream, fFieldOverrides);
	BinaryFormat::Read(inStream, fFieldList);
	fFieldRepeats = vector<int>(fFieldList.size(), 1);
	inStream >> itemCount;
	for (int i = 0; i < itemCount; i += 1) {
		UInt32 recordID, fmRecordID;
		inStream >> fmRecordID;
		inStream >> recordID;
		string digest;
		BinaryFormat::Read(inStream, digest);
		CRecordInfo info(recordID, digest);
		fRecordMap[fmRecordID] = info;
	}
}

bool operator == (const CDatabaseFile& a, const CDatabaseFile& b)
{
	if (!(a.fFileMakerName == b.fFileMakerName))
		return false;

	if (a.fPilotName != b.fPilotName)
		return false;

	if (a.fNewPilotName != b.fNewPilotName)
		return false;

	if (!(a.fJFileInfo == b.fJFileInfo))
		return false;

	if (!(a.fFieldOverrides == b.fFieldOverrides))
		return false;

	if (a.fDefaultToInt != b.fDefaultToInt)
		return false;

	if (a.fDefaultToDate != b.fDefaultToDate)
		return false;

	if (a.fDefaultToDate != b.fDefaultToDate)
		return false;

	if (a.fDuplicateOnConflict != b.fDuplicateOnConflict)
		return false;

	if (a.fJFileVersion != b.fJFileVersion)
		return false;

	if (a.fSyncMode != b.fSyncMode)
		return false;

	if (a.fArchiveFieldID != b.fArchiveFieldID)
		return false;

	if (a.fLayoutID != b.fLayoutID)
		return false;

	if (a.fPreSyncScriptID != b.fPreSyncScriptID)
		return false;

	if (a.fPostSyncScriptID != b.fPostSyncScriptID)
		return false;

	if (a.fFieldList != b.fFieldList)
		return false;

	if (a.fFieldRepeats != b.fFieldRepeats)
		return false;

	if (a.fLastSyncTime != b.fLastSyncTime)
		return false;

	if (a.fSyncSchedule != b.fSyncSchedule)
		return false;

	if (a.fOverrideSchedule != b.fOverrideSchedule)
		return false;

	if (a.fRecordMap != b.fRecordMap)
		return false;

	if (!(a.fUserNameField == b.fUserNameField))
		return false;

	if (a.fUseFoundSet != b.fUseFoundSet)
		return false;

	if (a.fDisabled != b.fDisabled)
		return false;

	if (a.fNewInstall != b.fNewInstall)
		return false;

	if (a.fInTrash != b.fInTrash)
		return false;

	if (a.fByName != b.fByName)
		return false;

	if (!AliasHandlesEquivalent(a.fAlias, b.fAlias))
		return false;

	return true;
}

void
CDatabaseFile::ForgetAlias()
{
	if (fAlias)
		DisposeHandle((Handle)fAlias);
	fAlias = 0;
}

void
CDatabaseFile::GetFSSpec(FSSpec *spec)
{
	GetFSSpec(*spec);
}

void
CDatabaseFile::GetFSSpec(FSSpec& spec)
{
	if (GetByName())
		Throw_(paramErr);
	spec = fFile;
}

void
CDatabaseFile::SetFSSpec(const FSSpec& spec)
{
	ForgetAlias();
	fResolveErr = FSMakeFSSpec(spec.vRefNum, spec.parID, spec.name, &fFile);
	fAlias = CreateAliasToFile();
	SetDatabaseNameFromFile(fFile);
}

void
CDatabaseFile::Initialize()
{
	CheckInTrash();
}

void
CDatabaseFile::CheckInTrash()
{
	short foundVRefNum;
	long foundDirID;
	if (FindFolder(fFile.vRefNum, kTrashFolderType, false, &foundVRefNum, &foundDirID) == noErr) {
		fInTrash = (foundDirID == fFile.parID && foundVRefNum == fFile.vRefNum);
	}
}

AliasHandle
CDatabaseFile::CreateAliasToFile()
{
	AliasHandle alias;
	ThrowIfOSErr_(::NewAlias(0, &fFile, &alias));
	return alias;
}

bool
CDatabaseFile::HasFile()
{
	return fResolveErr == noErr;
}

void
CDatabaseFile::WriteRecordMapCountToStream(LStream& inStream, int recordMapCount)
{
	UInt32 itemCount = recordMapCount;
	inStream << itemCount;
}

void
CDatabaseFile::WriteEmptyRecordMapToStream(LStream& inStream)
{
	WriteRecordMapCountToStream(inStream, 0);
}

void
CDatabaseFile::WriteRecordMapToStream(LStream& inStream)
{
	WriteRecordMapCountToStream(inStream, fRecordMap.size());

	if (fRecordMap.size() == 0)
		return;

	CRecordMap::iterator i = fRecordMap.begin();
	while (i != fRecordMap.end()) {
		ThrowIf_((*i).second.fDeleted);
		UInt32 recordID = (*i).second.fRecordID;
		UInt32 fmID = (*i).first;
		inStream << fmID;
		inStream << recordID;
		BinaryFormat::Write(inStream, (*i).second.fDigest);
		i++;
	}
}

void
CDatabaseFile::WriteOptionsToStream(LStream& inStream, bool forExport)
{
	inStream << (Handle)fAlias;
	BinaryFormat::Write(inStream, fFileMakerName);
	BinaryFormat::Write(inStream, fPilotName);
	BinaryFormat::Write(inStream, fNewPilotName);
	inStream << fArchiveFieldID;
	inStream << fLayoutID;
	BinaryFormat::Write(inStream, fPreSyncScriptID);
	BinaryFormat::Write(inStream, fPostSyncScriptID);
	BinaryFormat::Write(inStream, fUserNameField);
	inStream << fTranslateText;
	inStream << fDefaultToInt;
	inStream << fDefaultToDate;
	inStream << fJFileVersion;
	inStream << fSyncMode;
	inStream << fUseFoundSet;
	inStream << fDisabled;
	inStream << fDuplicateOnConflict;
	if (forExport)
		inStream << true;
	else
		inStream << fNewInstall;
	
	BinaryFormat::Write(inStream, fJFileInfo);
	BinaryFormat::Write(inStream, fFieldOverrides);
	BinaryFormat::Write(inStream, fFieldList);
	BinaryFormat::Write(inStream, fFieldRepeats);
	inStream << fLastSyncTime;
	inStream << fSyncSchedule;
	inStream << fOverrideSchedule;
}

void
CDatabaseFile::WriteToStream(LStream& inStream)
{
	WriteOptionsToStream(inStream, false);
	WriteRecordMapToStream(inStream);
}

void
CDatabaseFile::WriteToStreamNoMap(LStream& inStream)
{
	WriteOptionsToStream(inStream, true);
	WriteEmptyRecordMapToStream(inStream);
}

void
CDatabaseFile::ClearRecordMap()
{
	fRecordMap = CRecordMap();
}

CRecordMap
CDatabaseFile::GetRecordMap()
{
	return fRecordMap;
}

void
CDatabaseFile::SetRecordMap(const CRecordMap& keyFieldMap)
{
	//DEBUG_OUT_DEF("CDatabaseFile::SetRecordMap - this = ");
	//DEBUG_OUT_DEF(HexAsString((long)this).c_str());
	//DEBUG_OUT_DEF("\r");
	fRecordMap.clear();
	CRecordMap::const_iterator i = keyFieldMap.begin();
	while (i != keyFieldMap.end()) {
		if (!(*i).second.fDeleted) {
			UInt32 fmID = (*i).first;
			fRecordMap[fmID] = i->second;
		}
		i++;
	}
	//DEBUG_OUT_DEF("CDatabaseFile::SetRecordMap - fRecordMap.size() = ");
	//DEBUG_OUT_DEF(AsString(fRecordMap.size()).c_str());
	//DEBUG_OUT_DEF("\r");
}

CFieldIDList
CDatabaseFile::GetFieldIDList()
{
	return fFieldList;
}

vector<int> CDatabaseFile::GetFieldRepeats()
{
	return fFieldRepeats;
}

void
CDatabaseFile::RememberFieldInfo(const CFieldIDList& ids, const vector<int>& v)
{
	fFieldList = ids;
	fFieldRepeats = v;
}

bool
CDatabaseFile::NewInstall()
{
	return fNewInstall;
}

void
CDatabaseFile::SetNewInstall(bool newInstall)
{
	fNewInstall = newInstall;
}

int
CDatabaseFile::GetLayoutID()
{
	return fLayoutID;
}

void
CDatabaseFile::SetLayoutID(int theID)
{
	fLayoutID = theID;
}

FMAE::ScriptID
CDatabaseFile::GetPreSyncScriptID()
{
	return fPreSyncScriptID;
}

void
CDatabaseFile::SetPreSyncScriptID(FMAE::ScriptID theID)
{
	fPreSyncScriptID = theID;
}

FMAE::ScriptID
CDatabaseFile::GetPostSyncScriptID()
{
	return fPostSyncScriptID;
}

void
CDatabaseFile::SetPostSyncScriptID(FMAE::ScriptID theID)
{
	fPostSyncScriptID = theID;
}

int
CDatabaseFile::GetSyncMode()
{
	return fSyncMode;
}

void
CDatabaseFile::SetSyncMode(int syncMode)
{
	fSyncMode = syncMode;
}

int
CDatabaseFile::GetJFileVersion()
{
	return fJFileVersion;
}

void
CDatabaseFile::SetJFileVersion(int version)
{
	if (version != fJFileVersion) {
		fJFileVersion = version;
		SetNewInstall(true);
	}
}

bool
CDatabaseFile::GetTranslateText()
{
	return fTranslateText;
}

void
CDatabaseFile::SetTranslateText(bool translate)
{
	fTranslateText = translate;
}

bool
CDatabaseFile::GetUseFoundSet()
{
	return fUseFoundSet;
}

void
CDatabaseFile::SetUseFoundSet(bool useFoundSet)
{
	fUseFoundSet = useFoundSet;
}

bool
CDatabaseFile::GetDefaultToInt() const
{
	return fDefaultToInt;
}

void
CDatabaseFile::SetDefaultToInt(bool defaultTo)
{
	fDefaultToInt = defaultTo;
}

bool
CDatabaseFile::GetDefaultToDate() const
{
	return fDefaultToDate;
}

void
CDatabaseFile::SetDefaultToDate(bool defaultTo)
{
	fDefaultToDate = defaultTo;
}

bool
CDatabaseFile::GetDisabled()
{
	return fDisabled;
}

void
CDatabaseFile::SetDisabled(bool disabled)
{
	fDisabled = disabled;
}

bool
CDatabaseFile::GetDuplicateOnConflict()
{
	return fDuplicateOnConflict;
}

void
CDatabaseFile::SetDuplicateOnConflict(bool dup)
{
	fDuplicateOnConflict = dup;
}

string
CDatabaseFile::GetPilotDatabaseName()
{
	return fPilotName;
}

CDatabaseName
CDatabaseFile::GetFileMakerDatabaseName()
{
	return fFileMakerName;
}

void
CDatabaseFile::SetPilotDatabaseName(const string& theName)
{
	fPilotName = theName;
}

string
CDatabaseFile::GetNewPilotDatabaseName()
{
	return fNewPilotName;
}

void
CDatabaseFile::SetNewPilotDatabaseName(const string& theName)
{
	fNewPilotName = theName;
}

void
CDatabaseFile::RememberJFileInfo(const CDatabaseInfo& info)
{
	fJFileInfo = info;
}

CDatabaseInfo
CDatabaseFile::GetJFileInfo() const
{
	return fJFileInfo;
}

void
CDatabaseFile::SetFieldOverrides(const CFieldOverrides& o)
{
	fFieldOverrides = o;
}

CFieldOverrides
CDatabaseFile::FieldOverrides() const
{
	return fFieldOverrides;
}

map<FMAE::FieldID,int>
CDatabaseFile::GetFieldExtraData() const
{
	return fJFileInfo.GetFieldExtraData();
}

void
CDatabaseFile::SetFieldExtraData(const map<FMAE::FieldID,int>& inExtra)
{
	fJFileInfo.SetFieldExtraData(inExtra);
}

map<FMAE::FieldID,int>
CDatabaseFile::GetFieldExtraData2() const
{
	return fJFileInfo.GetFieldExtraData2();
}

void
CDatabaseFile::SetFieldExtraData2(const map<FMAE::FieldID,int>& inExtra)
{
	fJFileInfo.SetFieldExtraData2(inExtra);
}

FMAE::FieldID
CDatabaseFile::GetUserNameField() const
{
	return fUserNameField;
}

void
CDatabaseFile::SetUserNameField(const FMAE::FieldID& f)
{
	fUserNameField = f;
}

bool
CDatabaseFile::GetByName() const
{
	return fByName;
}

void
CDatabaseFile::SetDefaultTranslateText()
{
	long systemScript = GetScriptManagerVariable(smSysScript);
	fTranslateText = (systemScript == smRoman);
}

void
CDatabaseFile::SetDatabaseNameFromFile(const FSSpec& spec)
{
	fFileMakerName = CDatabaseName(AsString(spec.name), Folders::GetFileCreator(spec));
}

void CDatabaseFile::CreateSettingsFile(const FSSpec& theFile)
{
	OSErr err = FSpDelete(&theFile);
	if (err != fnfErr)
		ThrowIfOSErr_(err);
	LFile file(theFile);
	file.CreateNewDataFile(Creator, Type);
}

class CSettingsFileHeader {
public:
	enum {kMagic = 'Expo', kVersion = 0x0001001};
	
	CSettingsFileHeader();
	
	bool	Valid();

	UInt32 fMagic;
	UInt32 fVersion;
};

CSettingsFileHeader::CSettingsFileHeader()
	: fMagic(kMagic), fVersion(kVersion)
{
}

bool
CSettingsFileHeader::Valid()
{
	return fMagic = kMagic && fVersion == kVersion;
}

void CDatabaseFile::WriteSettingsFile(const FSSpec& theFile, CDatabaseFilePtr file, bool includeMap)
{
	LFileStream fs(theFile);
	fs.OpenDataFork(fsRdWrPerm);
	CSettingsFileHeader header;
	fs.WriteBlock(&header, sizeof(header));
	if (includeMap)
		file->WriteToStream(fs);
	else
		file->WriteToStreamNoMap(fs);
}

CDatabaseFilePtr
CDatabaseFile::ReadSettingsFile(const FSSpec& theFile)
{
	LFileStream fs(theFile);
	fs.OpenDataFork(fsRdPerm);
	CSettingsFileHeader header;
	fs.ReadBlock(&header, sizeof(header));
	if (!header.Valid()) {
		Throw_(kSettingsFileDamaged);
	}
	CDatabaseFilePtr file(new CDatabaseFile(fs, 0));
	return file;
}

string
CDatabaseFile::DescribeDatabase() const
{
	if (fByName) {
		return fFileMakerName.GetName();
	}
	return DescribeFile(fFile);
}

UInt32 CDatabaseFile::GetSchedule() const
{
	return fSyncSchedule;
}

void CDatabaseFile::SetSchedule(UInt32 v)
{
	fSyncSchedule = v;
}

UInt32 CDatabaseFile::GetLastSyncTime() const
{
	return fLastSyncTime;
}

UInt32 CDatabaseFile::GetNextSyncTimeForSchedule(UInt32 schedule, bool override) const
{
	if (override)
		return 0;
	
	if (schedule == kEveryTime)
		return 0;
	
	const UInt32 kOneDayInSeconds = 24*60*60;
	UInt32 delta = kOneDayInSeconds;
	if (schedule == kOnceAWeek) {
		delta = 7*kOneDayInSeconds;
	}
	return fLastSyncTime + delta;
}


UInt32 CDatabaseFile::GetNextSyncTime() const
{
	return GetNextSyncTimeForSchedule(fSyncSchedule, fOverrideSchedule);
}

void CDatabaseFile::SetLastSyncTime(UInt32 v)
{
	fLastSyncTime = v;
}

bool CDatabaseFile::GetOverrideSchedule() const
{
	return fOverrideSchedule;
}

void CDatabaseFile::SetOverrideSchedule(bool v)
{
	fOverrideSchedule = v;
}

#if 0
#pragma mark ===== CField =====

CField::CField()
	: fColumnWidth(80), fFMID(0)
{
}

CField::~CField()
{
}
#endif