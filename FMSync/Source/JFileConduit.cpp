#include "JFileConduit.h"
#include "ErrorCodes.h"
#include "CConfigDialog.h"
#include "CFileMaker.h"
#include "charset.h"
#include "LErrorMsg.h"
#include "JFCResources.h"
#include "ErrorStrings.h"
#include "OtherStrings.h"
#include "DebugOutput.h"
#include "CFrontProcess.h"

#if __profile__
#include "Profiler.h"
#endif

#include <stdio.h>
#include <string.h>

#include <Threads.h>
#include <LString.h>
#include <UEnvironment.h>


#include <Gestalt.h>
#include <UDebugging.h>
#include <UException.h>

#include "UConduitUtils.h"

#include "syncmgr.h"
#include "hslog.h"

#include "CJFileSynchronizer.h"
#include "CConduitSettings.h"
#include "CDatabaseFile.h"
#include "WritePDB.h"
#include "Stringiness.h"
#include "Str255.h"
#include "Utilities.h"
#include "TempFileUtilities.h"
#include "MoreFilesExtras.h"
#include "StAppleEvent.h"
#include "FMCResources.h"

#include "ConduitDebug.h"

#pragma export on

extern "C" 
{
	// The Template conduit does not use the new bundling APIs

	long OpenConduit(PROGRESSFN inProgressCallBack, CSyncProperties& inSyncProperties);

	long GetConduitName(char* oConduitName, WORD inStrLen);

	DWORD GetConduitVersion();
	
	long ConfigureConduit(CSyncPreference& inSyncPrefs);
	
	long GetActionString(CSyncPreference& inSyncPrefs,char* ioActionString,WORD inStrLen);

	// ¼¼¼ TikiSoft. Export these for the conduit manager to call.
	pascal OSErr __initialize(const CFragInitBlock *theInitBlock);
	pascal void __terminate(void);
	pascal OSErr ConduitInit(const CFragInitBlock *theInitBlock);
	pascal void ConduitExit(void);
}

#pragma export off

static short gRsrcFileRefNum = -1;
static Str63 gFragName = "\p";
ConstStringPtr gLockFileName = "\pFMSFJ5 Lock";
PROGRESSFN gProgressCallBack;
static short gProgressRsrcFileRefNum;
ProcessSerialNumber gConduitManagerPSN;
Boolean gSafeCancel = true;
Boolean gCanceling = false;


static string kCanceling;

class StResourceFile {
public:
		StResourceFile(short resFileRefNum);
		~StResourceFile();

private:
	short fResFileRefNum;
};

class StLockFile {
public:
		StLockFile(ConstStringPtr lockFileName);
		~StLockFile();

private:
	FSSpec	fLockFile;
};

StLockFile::StLockFile(ConstStringPtr lockFileName)
{
	bool fileExists = GetTempFileNamed(lockFileName, &fLockFile);
	if (!fileExists)
		ThrowIfOSErr_(FSpCreate(&fLockFile, 0, 0, 0));
}

StLockFile::~StLockFile()
{
	FSpDelete(&fLockFile);
}


static pascal OSErr OurGestalt(OSType selector, long *response)
{
	Debugger();
	*response = 0;
	return noErr;
}

void
CallConduitProgress(const char *progressString)
{
	static char progressBuffer[128];
	if (gCanceling) {
		strncpy(progressBuffer, kCanceling.c_str(), 128);
	} else if (progressString)
		strncpy(progressBuffer, progressString, 128);
	StResourceFile theirs(gProgressRsrcFileRefNum);
	if (gProgressCallBack) {
		#ifdef BRING_TO_FRONT
		ProcessSerialNumber frontProc;
		if( noErr == GetFrontProcess(&frontProc)) {
			Boolean isSame;
			if (noErr == SameProcess(&frontProc, &gConduitManagerPSN, &isSame) && !isSame) {
				SetFrontProcess(&gConduitManagerPSN);
			}
		}
		#endif
	}
}

static bool
NamedPreferencesSpec(const FSSpec& userDataFolder, ConstStringPtr prefName, FSSpec* outSpec)
{
	FSSpec userFolderSpec;
	FSMakeFSSpec(userDataFolder.vRefNum, userDataFolder.parID, 
		userDataFolder.name, &userFolderSpec);
	long dirID;
	Boolean isDir;
	FSpGetDirectoryID(&userFolderSpec, &dirID, &isDir);
	return FSMakeFSSpec(userFolderSpec.vRefNum, dirID, 
		prefName, outSpec) == noErr;
}

ConstStringPtr kSettingsFileName = "\pFMSync for JFile 5";

static bool
PreviousPreferencesSpec(const FSSpec& userDataFolder, FSSpec* outSpec)
{
	return false;
}

static bool
ShippingPreferencesSpec(const FSSpec& userDataFolder, FSSpec* outSpec)
{
	return NamedPreferencesSpec(userDataFolder, kSettingsFileName, outSpec);
}

static void
PreferencesSpec(const FSSpec& userDataFolder, FSSpec* outSpec)
{
	if (ShippingPreferencesSpec(userDataFolder, outSpec))
		return;
	
	FSSpec betaSpec;
	if (PreviousPreferencesSpec(userDataFolder, &betaSpec)) {
		FSpRename(&betaSpec, kSettingsFileName);
	}
}

static void
PreferencesSpec(const CSyncProperties& inSyncProperties, FSSpec* outSpec)
{
	PreferencesSpec(inSyncProperties.u.m_UserDirFSSpec, outSpec);
}

StResourceFile::StResourceFile(short resFileRefNum)
	: fResFileRefNum(0)
{
	fResFileRefNum = ::CurResFile();
	UseResFile(resFileRefNum);
	
}

StResourceFile::~StResourceFile()
{
	if (fResFileRefNum)
		UseResFile(fResFileRefNum);
}

long OpenConduit(PROGRESSFN inProgressCallBack, CSyncProperties& inSyncProperties)
{
	return CJFileConduit::OpenConduit(inProgressCallBack, inSyncProperties);
}

long GetConduitName(char* ioConduitName, WORD inStrLen)
{
	return CJFileConduit::GetConduitName(ioConduitName, inStrLen);
}

DWORD GetConduitVersion()
{
	return CJFileConduit::GetConduitVersion();
}

long ConfigureConduit(CSyncPreference& inSyncProperties)
{
	return CJFileConduit::ConfigureConduit(inSyncProperties);
}
#if PP_Target_Classic
// ---------------------------------------------------------------------------
//	¥ SetUpQD
// ---------------------------------------------------------------------------
//	This is how we obtain qd for our code resource.
//
//	This code is taken verbatium (with minor modifications) from the CW CR
//	Example "Multi-Seg CR w/ Callback" in InitResource.c by Mark Anderson.

static QDGlobals*	SetUpQD( void )
{
	// variables necessary to prepare for matching or local qdPtr to the
	// app's qd
	ProcessSerialNumber PSN;
	FSSpec				myFSSpec;
	Str63				name;
	ProcessInfoRec		infoRec;
	OSErr				result = noErr;
	CFragConnectionID 	connID;
	Str255 				errName;
	CFragSymbolClass	symClass;
	Ptr					mainAddr; 	
	// variables to check if CFM is present
	OSErr 				err;
	SInt32				response;
	SInt16				myBit;
	Boolean				hasCFM;
	
	QDGlobals			*qdPtr;
	
		// Ask the system if CFM is available
	err = Gestalt(gestaltCFMAttr, &response);
	myBit = gestaltCFMPresent;
	hasCFM = BitTst(&response, 31-myBit);
			
	if (hasCFM) {
		
		// GetProcessInformation needs the address of a FSSpec and a string
		// for the name of the current App (this one) and the following fields
		// filled out.  See the Process Manager in "Inside Macintosh:Processes".
		
		infoRec.processInfoLength = sizeof(ProcessInfoRec);
		infoRec.processName = name;
		infoRec.processAppSpec = &myFSSpec;
		
		PSN.highLongOfPSN = 0;
		PSN.lowLongOfPSN = kCurrentProcess;
		
		result = GetProcessInformation( &PSN, &infoRec);
		if (result != noErr)
			DebugStr("\pFailed in GetProcessInformation");

		// GetDiskFragment will return the connID number so we can use it in
		// subsequent calls to FindSymbol.  It will also return the address of
		// main in the native App which we can ignore.  If GetDiskFragment 
		// doesn't return noErr, the app is 68K.

		result =  GetDiskFragment(infoRec.processAppSpec, 0L, 0L, infoRec.processName, kLoadCFrag, &connID, 
					(Ptr*)&mainAddr, errName);
	} else {
		result = -1; // initialize for 68K machines which don't use GetDiskFragment
	}

	if (result == noErr) { // The app's a PPC code fragment

		// FindSymbol will return qd from the mother app.
		
		result = FindSymbol(connID, "\pqd", (Ptr*)&qdPtr, &symClass);
	}
	
	if (result != noErr) {
		qdPtr = (QDGlobals*)(*((long*)SetCurrentA5()) - (sizeof(QDGlobals) - sizeof(GrafPtr)));
	}

	return qdPtr;
}
#endif

void CJFileConduit::CheckCanRun()
{
	static bool callEnvironmentInit = true;
	if (callEnvironmentInit) {
		UEnvironment::InitEnvironment();
#if PP_Target_Classic
		UQDGlobals::SetQDGlobals(SetUpQD());
#endif
		UCursor::InitTheCursor();
		callEnvironmentInit = false;
	}
	
	if (!UEnvironment::HasFeature(env_HasAppearance)) {
		Throw_(kNeedAppearanceCode);
	}

	if (UEnvironment::GetOSVersion() < 0x0755) {
		Throw_(kNeedSys8Code);
	}

	if (!UEnvironment::HasGestaltAttribute(gestaltAppleScriptAttr, gestaltAppleScriptPresent)) {
		Throw_(kNeedsAppleScript);
	}
}

long CJFileConduit::OpenConduit(PROGRESSFN inProgressCallBack, CSyncProperties& inSyncProperties)
{
#ifdef _DEBUG
	UDebugging::SetDebugThrow(debugAction_Alert);
	UDebugging::SetDebugSignal(debugAction_Alert);
#endif
	DebugOutput::Output("CJFileConduit::OpenConduit");
	ExceptionCode err = GetFrontProcess(&gConduitManagerPSN);
	CFrontProcess fp;
	
	if (err != noErr)
		return err;

	gProgressRsrcFileRefNum = ::CurResFile();
	StResourceFile file(gRsrcFileRefNum);
	
	
	kCanceling = LoadString(kOtherStrings, kCancelingIndex);
	gCanceling = false;
	
	volatile bool namesAllocated = false;
	bool errorOccurred = false;

	try
	{
#if __profile__
		ThrowIfOSErr_(ProfilerInit(collectDetailed, bestTimeBase, 2000, 20));
#endif

		FSSpec settingsSpec;
		DebugOutput::Output("	PreferencesSpec");
		PreferencesSpec(inSyncProperties, &settingsSpec);
		DebugOutput::Output("	new CConduitSettings");
		StDeleter<CConduitSettings> settings(new CConduitSettings(&settingsSpec));

		DebugOutput::Output("	CheckCanRun()");
		CheckCanRun();
		CONDHANDLE conduitHandle = 0;
		Byte dbHandle = 0;
		short recCount = 0;
		CRawRecordInfo rawRecordInfo = {};
		static char s[255] = {};
		
		string userName(inSyncProperties.m_UserName);
		
		if (settings->SyncAction() != CConduitSettings::kDoNothing) {
			vector<char*> gNames(settings->DatabaseCount());
			FSSpec lockFile;
			DebugOutput::Output("	GetTempFileNamed()");
			bool fileExists = GetTempFileNamed(gLockFileName, &lockFile);
			if (fileExists)	
				Throw_(kLockFileInUse);

			SetConduitProgressCallback(inProgressCallBack);
			int totalToDo = 0;
			int totalToCheck = settings->DatabaseCount();
			inSyncProperties.m_nRemoteCount = settings->DatabaseCount();
			for (int i = 0; i < totalToCheck; i+= 1) {
				CDatabaseFile::Ptr file(settings->GetDatabase(i));
				totalToDo += 1;
				gNames[i] = new char[64];
				string dbName(file->GetPilotDatabaseName());
				strncpy(gNames[i], dbName.c_str(), 64);
				if (file->GetTranslateText())
					ConvertToPilotText(gNames[i]);
			}
			namesAllocated = true;
			inSyncProperties.m_nRemoteCount = totalToDo;
			inSyncProperties.m_RemoteName = &gNames[0];
			inSyncProperties.m_Creator = CWritePDB::kCreator;
			inSyncProperties.m_DbType = CWritePDB::kType;
			inSyncProperties.m_CardNo = 0;
			DebugOutput::Output("	CJFileSynchronizer()");
			CJFileSynchronizer synchronizer(inSyncProperties, settings.Get(), inProgressCallBack);
			DebugOutput::Output("	Synchronize()");
			synchronizer.Synchronize();
			if (synchronizer.ErrorOccurred())
				errorOccurred = true;
			inSyncProperties.m_RemoteName = 0;
		}
		settings->SetSyncAction(settings->DefaultSyncAction());
		settings->UpdateFile();
	}
	
	catch (const LException& inErr)
	{
		DebugOutput::Output("	catch const LException& inErr");
		err = inErr.GetErrorCode();
		CJFileSynchronizer::PutErrorCodeIntoLogNoFile(err);
	}
	
	catch(...)
	{
		DebugOutput::Output("	catch kUnknownError");
		err = kUnknownError;
	}
	
	CFileMaker::ReleaseAll();
	

#if __profile__
		ProfilerDump("\pFMSync for JFile.prof");
		ProfilerTerm();
#endif

	if (errorOccurred)
		return SYNCERR_UNKNOWN;
	return err;
}

long CJFileConduit::GetConduitName(char* ioConduitName, WORD inStrLen)
{	
	::strncpy(ioConduitName, "FMSync: JFile X", inStrLen);
	return 0;
}

void SetConduitProgressCallback(PROGRESSFN proc)
{
	gProgressCallBack = proc;
}

long CJFileConduit::ConfigureConduit(CSyncPreference& inSyncProperties)
{
#if __profile__
		ThrowIfOSErr_(ProfilerInit(collectDetailed, bestTimeBase, 2000, 20));
#endif

	UDebugging::SetDebugThrow(debugAction_Debugger);
	CFrontProcess fp;
	SetConduitProgressCallback(0);
	OSErr err = noErr;
	gProgressRsrcFileRefNum = ::CurResFile();
	StResourceFile file(gRsrcFileRefNum);
	GrafPtr thePort;
	GetPort(&thePort);
	try {
		StLockFile lock(gLockFileName);
		CheckCanRun();
		FSSpec settingsSpec;
		PreferencesSpec(inSyncProperties.u.m_UserDirFSSpec, &settingsSpec);
		UCursor::SetWatch();
		CConfigDialog theDialog(inSyncProperties, settingsSpec);
		theDialog.DoDialog();
	} catch (const LException& inErr) {
		ExceptionCode thrownErr(inErr.GetErrorCode());
		if (thrownErr == kNeedAppearanceCode) {
			::StopAlert(kALRTNeedsAppMgr, 0);
			err = noErr;
		} else if (thrownErr == kNeedSys8Code) {
			::StopAlert(kALRTNeedsSystem81, 0);
			err = noErr;
		} else if (!IsQuietError(inErr.GetErrorCode())) {
			string couldNotComplete(LoadString(kFMJErrorStrings, kCouldNotCompleteIndex));
			couldNotComplete.append(ConvertErrorToString(inErr.GetErrorCode()));
			LErrorMsg::AlertWithMessage(kAlertStopAlert, couldNotComplete);
			err = noErr;
		} else 
			err = thrownErr;
	}
	catch(...)
	{
		err = SYNCERR_UNKNOWN;
	}
	CFileMaker::ReleaseAll();
	UCursor::SetArrow();
	SetPort(thePort);
	
#if __profile__
		ProfilerDump("\pDemos.prof");
		ProfilerTerm();
#endif
	return err;
}



DWORD CJFileConduit::GetConduitVersion()
{	
	return 0x00000300;
}

Boolean CJFileConduit::IsThisKeyDown(const short theKey)
{
	union
	{
		KeyMap asMap;
		Byte asBytes[16];
	};

	GetKeys(asMap);
	return asBytes[theKey >> 3] & (1 << (theKey & 0x07)) ? TRUE : FALSE;
}

pascal OSErr ConduitInit(const CFragInitBlock* theInitBlock)
{
	OSErr err = noErr;

	__initialize(theInitBlock);

	if (theInitBlock->fragLocator.where == kDataForkCFragLocator)
	{
		gRsrcFileRefNum = ::FSpOpenResFile(theInitBlock->fragLocator.u.onDisk.fileSpec, fsRdPerm);
		if (gRsrcFileRefNum == -1)
			err = ResError();
	}

	if (theInitBlock->libName != nil)
		memcpy(gFragName, theInitBlock->fragLocator.u.onDisk.fileSpec->name,
						  theInitBlock->fragLocator.u.onDisk.fileSpec->name[0] + 1);

	return err;
}

pascal void ConduitExit(void)
{
	if (gRsrcFileRefNum != -1) {
		CloseResFile(gRsrcFileRefNum);
		gRsrcFileRefNum = -1;
	}

	__terminate();
}

long GetActionString(CSyncPreference& inSyncPrefs,char* ioActionString,WORD inStrLen)
{
	long err = noErr;
	try {
		StLockFile lock(gLockFileName);
		gProgressRsrcFileRefNum = ::CurResFile();
		StResourceFile file(gRsrcFileRefNum);
		FSSpec settingsSpec;
		PreferencesSpec(inSyncPrefs.u.m_UserDirFSSpec, &settingsSpec);
		StDeleter<CConduitSettings> settings(new CConduitSettings(&settingsSpec));
		int action = settings->SyncAction();
		LStr255 actionpString(kFMCActionStrings, action+1);
		string actionString(AsString(actionpString));
		strncpy(ioActionString, actionString.c_str(), inStrLen);
	}
	
	catch(...) {
		err = kUnknownError;
	}
	return err;
}