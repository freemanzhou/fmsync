#pragma once

#include <LString.h>
#include <map>
#include <string>

#include "CRecord.h"
#include "CDatabaseInfo.h"
#include "CDatabaseName.h"
#include "CFileMaker.h"
#include "CFieldIDList.h"
#include "CFieldOverrides.h"
#include "CDatabaseFilePtr.h"
#include "Str255.h"

class LStream;

typedef map<UInt32,CRecordInfo> CRecordMap;

class CDatabaseFile {
public:

	typedef boost::shared_ptr<CDatabaseFile> Ptr;

	enum {JFile32 = 100, JFilePro, JFile5};
	enum {Type = 'pref', Creator = 'FMSy'};
	enum {SyncNormal = 1, SyncUploadAndClear, SyncClearAndDownload};
	enum {kEveryTime, kOnceADay, kOnceAWeek};

	CDatabaseFile(const CDatabaseName& filemakerName);
	CDatabaseFile(const FSSpec& inFile); 
	CDatabaseFile(AliasHandle inAlias); 
	CDatabaseFile(LStream& inStream, int version); 
	CDatabaseFile(const CDatabaseFile& inDatabaseFile);
	
	void WriteToStream(LStream& inStream);
	void WriteToStreamNoMap(LStream& inStream);
	static void CreateSettingsFile(const FSSpec& theFile);
    static void WriteSettingsFile(const FSSpec& theFile, CDatabaseFilePtr file, bool includeMap);
    static CDatabaseFilePtr ReadSettingsFile(const FSSpec&);

	virtual ~CDatabaseFile();
	void GetFSSpec(FSSpec *spec);
	void GetFSSpec(FSSpec& spec);
	void SetFSSpec(const FSSpec& spec);
	bool FileInTrash() const {return fInTrash;}
	AliasHandle CreateAliasToFile();
	string FieldName(int);
	void ClearRecordMap();
	CRecordMap GetRecordMap();
	void SetRecordMap(const CRecordMap&);
	CFieldIDList GetFieldIDList();
	vector<int> GetFieldRepeats();
	void RememberFieldInfo(const CFieldIDList& ids, const vector<int>& v);
	void SetFieldOverrides(const CFieldOverrides&);
	CFieldOverrides FieldOverrides() const;
	bool NewInstall();
	void SetNewInstall(bool newInstall);
	bool HasFile();
	int GetLayoutID();
	void SetLayoutID(int);
	int GetSyncMode();
	void SetSyncMode(int);
	int GetJFileVersion();
	void SetJFileVersion(int);
	FMAE::ScriptID GetPreSyncScriptID();
	void SetPreSyncScriptID(FMAE::ScriptID);
	FMAE::ScriptID GetPostSyncScriptID();
	void SetPostSyncScriptID(FMAE::ScriptID);
	bool GetTranslateText();
	void SetTranslateText(bool);
	bool GetUseFoundSet();
	void SetUseFoundSet(bool);
	bool GetDefaultToInt() const;
	void SetDefaultToInt(bool);
	bool GetDefaultToDate() const;
	void SetDefaultToDate(bool);
	bool GetDisabled();
	void SetDisabled(bool);
	bool GetDuplicateOnConflict();
	void SetDuplicateOnConflict(bool);
	CDatabaseName GetFileMakerDatabaseName();
	string GetPilotDatabaseName();
	void SetPilotDatabaseName(const string&);
	string GetNewPilotDatabaseName();
	void SetNewPilotDatabaseName(const string&);
	
	FMAE::FieldID GetUserNameField() const;
	void SetUserNameField(const FMAE::FieldID&);
	
	bool GetByName() const;

	void RememberJFileInfo(const CDatabaseInfo&);
	CDatabaseInfo GetJFileInfo() const;
	
	UInt32 GetSchedule() const;
	void SetSchedule(UInt32);
	
	UInt32 GetLastSyncTime() const;
	UInt32 GetNextSyncTimeForSchedule(UInt32 schedule, bool override) const;
	UInt32 GetNextSyncTime() const;
	void SetLastSyncTime(UInt32);
	
	bool GetOverrideSchedule() const;
	void SetOverrideSchedule(bool);
	
	string DescribeDatabase() const;

	friend bool operator == (const CDatabaseFile&, const CDatabaseFile&);

private:
	void WriteOptionsToStream(LStream& inStream, bool forExport);
	void WriteRecordMapToStream(LStream& inStream);
	void WriteEmptyRecordMapToStream(LStream& inStream);
	void WriteRecordMapCountToStream(LStream& inStream, int recordMapCount);

	void ReadCurrentVersion(LStream& inStream);
	void ReadVersion115(LStream& inStream);
	
	map<FMAE::FieldID,int> GetFieldExtraData() const;
	void SetFieldExtraData(const map<FMAE::FieldID,int>&);

	map<FMAE::FieldID,int> GetFieldExtraData2() const;
	void SetFieldExtraData2(const map<FMAE::FieldID,int>&);

	void Initialize();
	void CheckInTrash();
	void ForgetAlias();
	
	void SetDefaultTranslateText();
	void SetDatabaseNameFromFile(const FSSpec&);

	FSSpec fFile;
	AliasHandle fAlias;
	CDatabaseName fFileMakerName;
	string fPilotName;
	string fNewPilotName;
	CDatabaseInfo fJFileInfo;
	CFieldOverrides fFieldOverrides;
	bool fTranslateText;
	bool fDefaultToInt;
	bool fDefaultToDate;
	bool fDuplicateOnConflict;
	UInt32 fJFileVersion;
	UInt32 fSyncMode;
	UInt32 fArchiveFieldID;
	UInt32 fLayoutID;
	UInt32 fLastSyncTime;
	UInt32 fSyncSchedule;
	bool fOverrideSchedule;
	FMAE::ScriptID fPreSyncScriptID;
	FMAE::ScriptID fPostSyncScriptID;
	CFieldIDList fFieldList;
	vector<int> fFieldRepeats;
	CRecordMap fRecordMap;
	FMAE::FieldID fUserNameField;
	bool fUseFoundSet;
	bool fDisabled;
	bool fNewInstall;
	bool fInTrash;
	bool fByName;
 	OSErr fResolveErr;
};

class CompareDatabaseFileByName {
public:
	bool operator() (const CDatabaseFilePtr& p1, const CDatabaseFilePtr& p2) const
	{
		LStr255 name1(AsStr255(p1->GetPilotDatabaseName()));
		LStr255 name2(AsStr255(p2->GetPilotDatabaseName()));
		name1.SetCompareFunc(LString::CompareIgnoringCase);
		return name1.CompareTo(name2) < 0;
	}
};

