#pragma once

#include "CAEDescriptor.h"

const OSType kUSFileMakerCreator = 'FMP3';
const OSType kUSFileMaker5Creator = 'FMP5';
const OSType kJapaneseFileMakerCreator = 'FMJ3';
const OSType kJapaneseFileMaker5Creator = 'FMJ5';
const OSType kFMP21Creator = 'FMPR';

const int kAEFlagsForConduit = kAEWaitReply | kAECanInteract | kAECanSwitchLayer | kAEDontRecord;
const int kAEFlagsForConduitNoReply = kAENoReply | kAECanInteract | kAECanSwitchLayer | kAEDontRecord;

class CDatabaseName;

class CFileMaker {
public:
	enum {kNeverQuit = 1, kQuitIfLaunched, kAlwaysQuit};
							~CFileMaker();
	static CFileMaker&		Get(OSType creator);
	void					Open(const FSSpec& databaseSpec, AEDesc *databaseDescriptor);
	void					GetDatabaseDescriptor(const string& databaseName, AEDesc *databaseDescriptor);
	void					Close(const AEDesc& databaseDescriptor);
	void					Release();
	string					GetVersionString();
	short					MajorVersion();
	short					MinorVersion();
	short					UpdateVersion();
	bool					WasRunning();
	void					CheckVersion();
	
	static int				QuitMode();
	static void				SetQuitMode(int);
	static void				ReleaseAll();
	
	Boolean					WasInitialized();
	static bool				AskForFileMakerDatabase(FSSpec& outSpec, OSType& outType);
	static bool				AskForFileMakerDatabasePlus(OSType extraType, FSSpec& outSpec, OSType& outType);
	bool					FileMakerIsRunning();
	static bool 			CreatorIsFileMaker(OSType creator);

	int						GetDatabaseCount();
	CDatabaseName			GetDatabaseName(int index);
	vector<CDatabaseName>	GetAllDatabaseNames();
	
	static vector<OSType>	GetAllCreators();
	
			operator	AEAddressDesc*() { return fAddress; }
			operator	AEAddressDesc&() { return fAddress; }

private:
							CFileMaker(OSType creator);
							CFileMaker(const CFileMaker&);
	
	Boolean					Initialize(const FSSpec& sourceFile);
	void					PutFileMakerInFront();
	string					AskForPassword(const FSSpec& databaseSpec);
	void					TellFinderToOpen(const FSSpec& sourceFile);
	void					SendOpenEvent(const FSSpec& databaseSpec);
	void					AskPasswordAndSendOpenEvent(const FSSpec& databaseSpec);
	void					DoSendOpenEvent(const FSSpec& databaseSpec, const string& password = "");
	void					QuitFileMaker();
	void					ParseVersionString(const string&);
	
	void					HandleLaunchWithDocumentFailed(OSErr err, const FSSpec& databaseSpec);
	static bool				ShouldTryWithPassword(OSType creator, OSErr err);
	void					DoAskPasswordAndSendOpenEvent(const FSSpec& databaseSpec);
			
	CAEDescriptor			fAddress;
	CAEDescriptor			fOurAddress;
	ProcessSerialNumber		fFilemakerPSN;
	OSType					fCreator;
	Boolean					fInitialized;
	Boolean					fWasRunning;
	static int				gQuitMode;
	short					fMajorVersion;
	short					fMinorVersion;
	short					fUpdateVersion;
};

namespace FileMaker {

vector<CDatabaseName>	GetAllDatabaseNamesFromAllVersions();

}