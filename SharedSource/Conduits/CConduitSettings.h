#pragma once

#include <PP_Prefix.h>
#include <deque>
#include "CUploadRequest.h"
#include "CDatabaseFilePtr.h"
#include "CWarnings.h"

class CDatabaseFile;
class CConduitSettingsHeader;

class CConduitSettings {
public:
	enum {kMagic = 'cSet', k115Version = 0x0001002d, kCurrentVersion = 0x00020005};
	enum {kDoNothing, kSyncDatabases};
	enum {kWarnFMP3Order = 1};
	enum {kAccessByFile = 1, kAccessByName};
	typedef int WarningID;

	CConduitSettings(const FSSpec* spec); 
	virtual  ~CConduitSettings();

	void UpdateFile();
	int DatabaseCount();
	int SyncAction();
	void SetSyncAction(int actionCode);
	int QuitMode();
	void SetQuitMode(int quitMode);
	int DefaultSyncAction();
	void SetDefaultSyncAction(int actionCode);
	string RegCode();
	void SetRegCode(const string& regCode);
	CDatabaseFilePtr GetDatabase(int index);
	CDatabaseFilePtr FindDatabase(const FSSpec& thisFile);
	CDatabaseFilePtr FindDatabaseByRemoteName(const string& remoteFileName);
	vector<CUploadRequest> GetUploadRequests() const;
	void SetUploadRequests(const vector<CUploadRequest>&);
	vector<string> GetAllRemoteNames(CDatabaseFilePtr exceptThis);
	vector<string> GetJFileNames() const;
	void SetAllJFileNames(const vector<string>&);
	vector<string> GetPalmNames() const;
	void SetPalmNames(const vector<string>&);
	UInt32 GetLastSyncTime() const;
	void SetLastSyncTime(UInt32);
	void AddDatabase(CDatabaseFilePtr f);
	void RemoveDatabase(CDatabaseFilePtr f);

	void CheckRegistration(const string& userName);
	
	bool WarningDisabled(WarningID warningID) const;
	void SetWarningDisabled(WarningID warningID, bool warned);
	
	CWarnings GetWarnings() const;
	void SetWarnings(const CWarnings& w);
	
	int LastAccessMode() const {return fAccessMode;}
	void SetLastAccessMode(int l) {fAccessMode = l;}

private:

	void ReadVersion115(CConduitSettingsHeader&, LFileStream&);
	void ReadCurrentVersion(CConduitSettingsHeader&, LFileStream&);

	typedef vector<CDatabaseFilePtr> DatabaseList;
	DatabaseList fDatabases;
	vector<string> fJFileDatabases;
	vector<string> fPalmDatabases;
	vector<CUploadRequest> fUploadRequests;
	CWarnings fWarnings;
	UInt32 fLastSyncTime;
	int fSyncAction;
	int fDefaultSyncAction;
	int fQuitMode;
	int fAccessMode;
	FSSpec fFile;
	string fRegCode;
};