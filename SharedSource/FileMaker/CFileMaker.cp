#include "LaunchFMProWithDoc2.h"
#include "CDatabaseName.h"
#include "CPasswordDialog.h"
#include "CFileMaker.h"
#include "CFrontProcess.h"
#include "FMAE.h"
#include "Utilities.h"
#include "FolderUtilities.h"
#include "ErrorCodes.h"
#include "StAppleEvent.h"
#include "Stringiness.h"
#include "Str255.h"
#include "CAEDescriptor.h"

#include <UStandardDialogs.h>
#include <LaunchServices.h>

class StBringToFront {
public:
	StBringToFront(ProcessSerialNumber psn);
	~StBringToFront();
	
	void DoBringToFront(ProcessSerialNumber psn);
	
	ProcessSerialNumber fOriginal;
	ProcessSerialNumber fTarget;
};

StBringToFront::StBringToFront(ProcessSerialNumber psn) 
	: fTarget(psn)
{
	GetCurrentProcess(&fOriginal);
	DoBringToFront(fTarget);
}

StBringToFront::~StBringToFront()
{
	//DoBringToFront(fOriginal);
}

void
StBringToFront::DoBringToFront(ProcessSerialNumber psn)
{
	ThrowIfOSErr_(SetFrontProcess(&psn));
	Boolean isSame;
	do {
		EventRecord eventRecord;
		WaitNextEvent(0, &eventRecord, 0, 0);
		ProcessSerialNumber fp;
		ThrowIfOSErr_(GetFrontProcess(&fp));
		ThrowIfOSErr_(SameProcess(&fp, &psn, &isSame));
	} while (!isSame);
}

const int kAEFlagsForReply = kAEWaitReply | kAECanInteract | kAECanSwitchLayer | kAEDontRecord;

int CFileMaker::gQuitMode = kNeverQuit;

const int kNumTypes = 5;
const OSType gTypes[kNumTypes] = {kUSFileMakerCreator, kJapaneseFileMakerCreator, kUSFileMaker5Creator, kJapaneseFileMaker5Creator, kFMP21Creator};
static map<OSType, CFileMaker*> gFileMakers;

const OSType gAllCreators[] = {kUSFileMakerCreator, kJapaneseFileMakerCreator, kUSFileMaker5Creator, kJapaneseFileMaker5Creator};
const int gAllCreatorsCount = sizeof(gAllCreators)/sizeof(gAllCreators[0]);

CFileMaker::~CFileMaker()
{
	Release();
}

CFileMaker::CFileMaker(OSType creator)
	: fInitialized(false), fCreator(creator), fWasRunning(false), 
	fMajorVersion(0), fMinorVersion(0), fUpdateVersion(0)
{
}

CFileMaker&
CFileMaker::Get(OSType creator)
{
	map<OSType, CFileMaker*>::iterator f = gFileMakers.find(creator);
	if (f == gFileMakers.end()) {
		CFileMaker *fm = new CFileMaker(creator);
		gFileMakers[creator] = fm;
		return *fm;
	}
	return *f->second;
}

vector<OSType>
CFileMaker::GetAllCreators()
{
	vector<OSType> allC;
	copy(gAllCreators, gAllCreators + gAllCreatorsCount, back_inserter(allC));
	return allC;
}

void
CFileMaker::ReleaseAll()
{
	for(map<OSType, CFileMaker*>::iterator i = gFileMakers.begin(); i != gFileMakers.end(); ++i) {
		i->second->Release();
	}
}

//const unsigned int kEventMask = updateMask | osMask;
const unsigned int kEventMask = everyEvent;

#define LAUNCH_SUPPORTED 1

Boolean
CFileMaker::Initialize(const FSSpec& sourceFile)
{
	Boolean launched = false;
	if(!fInitialized) {
		FMAE::gFileMakerCreator = fCreator;
		ThrowIfOSErr_(AECreateDesc(typeApplSignature, (Ptr) &fCreator,
			sizeof(fCreator), fAddress));

		fWasRunning = false;
		//StGrafPortSaver saver;
		ProcessSerialNumber myPSN;
		ProcessSerialNumber currPSN;
		ProcessInfoRec currProcessInfo;
		FSSpec applicationSpec;
		Boolean foundRunningProcessFlag;
		
		ThrowIfOSErr_(GetFrontProcess(&myPSN));
		ThrowIfOSErr_(AECreateDesc(typeProcessSerialNumber, (Ptr) &myPSN,
			sizeof(myPSN), fOurAddress));

		currPSN.lowLongOfPSN = kNoProcess;
		currPSN.highLongOfPSN = 0;
		
		currProcessInfo.processInfoLength = sizeof(ProcessInfoRec);
		currProcessInfo.processName = nil;
		currProcessInfo.processAppSpec = &applicationSpec;
		
		foundRunningProcessFlag = false;
		while (GetNextProcess(&currPSN) == noErr) {
			if (GetProcessInformation(&currPSN, &currProcessInfo) == noErr) {
				if (currProcessInfo.processSignature == fCreator) {
					foundRunningProcessFlag = true;
					fCreator = currProcessInfo.processSignature;
					fWasRunning = true;
					fFilemakerPSN = currPSN;
					break;
				}
			}
		}
		
		if (!foundRunningProcessFlag) {
			FSRef ref;
			FSRef appRef;
			ThrowIfOSErr_(FSpMakeFSRef(&sourceFile, &ref));
			OSStatus err = LSOpenFSRef(&ref, &appRef);
			ThrowIfNot_(WaitUntilFileMakerIsRunning());
			HandleLaunchWithDocumentFailed(err, sourceFile);
			launched = true;
			StBringToFront btf(fFilemakerPSN);
		}
					
		fInitialized = true;
		string versionString(GetVersionString());
		ParseVersionString(versionString);

	}
	return launched;
}

static FSSpec FindFileMakerForFile(const FSSpec& sourceFile)
{
	FSSpec fm;
	OSType creator = Folders::GetFileCreator(sourceFile);
	ThrowIfOSErr_(FindFileMaker(creator, &sourceFile, &fm));
	return fm;
}

void
CFileMaker::TellFinderToOpen(const FSSpec& sourceFile)
{
	FSSpec fm(FindFileMakerForFile(sourceFile));
	OSType creator = 'MACS';
	CAEDescriptor finderAddress(typeApplSignature, &creator, sizeof(creator));
	CAEDescriptor fmFile(fm);
	StAppleEvent openEvent(kCoreEventClass, kAEOpenDocuments, finderAddress);
	CAEDescriptor docDescList;
	ThrowIfOSErr_(AECreateList(0, 0, false, docDescList));
	CAEDescriptor docDesc(sourceFile);
	ThrowIfOSErr_(AEPutDesc(docDescList, 0, docDesc));
	ThrowIfOSErr_(AEPutParamDesc(openEvent, keyDirectObject, docDescList));
	ThrowIfOSErr_(AEPutParamDesc(openEvent, 'usin', fmFile));
	StAppleEvent theReply;
	openEvent.Send(theReply, kAEFlagsForReply);
}


const long cDatabase = 'cDB ';

void
CFileMaker::PutFileMakerInFront()
{
	Boolean sameProcess;
	ProcessSerialNumber curPSN;
	GetCurrentProcess(&curPSN);
	ThrowIfOSErr_(SameProcess(&curPSN, &fFilemakerPSN, &sameProcess));
	if (!sameProcess) {
		SetFrontProcess(&fFilemakerPSN);
		do {
			EventRecord eventRecord;
			WaitNextEvent(0, &eventRecord, 0, 0);
			GetCurrentProcess(&curPSN);
			ThrowIfOSErr_(SameProcess(&curPSN, &fFilemakerPSN, &sameProcess));
		} while (!sameProcess);
	}
}

string
CFileMaker::AskForPassword(const FSSpec& databaseSpec)
{
	string thePassword;
#ifdef CONDUIT
	CPasswordDialog theDialog(AsString(databaseSpec.name));
	
	if (!theDialog.DoDialog())
		thePassword = theDialog.GetPassword();
#endif	
	return thePassword;
}

void
CFileMaker::DoAskPasswordAndSendOpenEvent(const FSSpec& databaseSpec)
{
	string password(AskForPassword(databaseSpec));
	if (password.length() == 0)
		Throw_(userCanceledErr);
	DoSendOpenEvent(databaseSpec, password);
}

void
CFileMaker::AskPasswordAndSendOpenEvent(const FSSpec& databaseSpec)
{
	SInt32 errorCode;
	do {
		try {
			DoAskPasswordAndSendOpenEvent(databaseSpec);
			errorCode = noErr;
		} catch(LException& err) {
			errorCode = err.GetErrorCode();
			if (errorCode != noErr && errorCode != errAEEventFailed)
				throw;
		}
	} while (errorCode == errAEEventFailed);
}

void
CFileMaker::SendOpenEvent(const FSSpec& databaseSpec)
{
	try {
		DoSendOpenEvent(databaseSpec);
		return;
	}
	
	catch(LException& err) {
		if (!ShouldTryWithPassword(fCreator, err.GetErrorCode()))
			throw;
	}
	
	AskPasswordAndSendOpenEvent(databaseSpec);
}

void
CFileMaker::CheckVersion()
{
	string versionString(GetVersionString());
	ParseVersionString(versionString);
}

void
CFileMaker::DoSendOpenEvent(const FSSpec& databaseSpec, const string& password)
{
	StAppleEvent openEvent(kCoreEventClass, kAEOpenDocuments, fAddress);
	CAEDescriptor docDescList;
	ThrowIfOSErr_(AECreateList(0, 0, false, docDescList));
	CAEDescriptor docDesc(databaseSpec);
	ThrowIfOSErr_(AEPutDesc(docDescList, 0, docDesc));
	ThrowIfOSErr_(AEPutParamDesc(openEvent, keyDirectObject, docDescList));
	if (password.length()) {
		CAEDescriptor passwordDesc(password);
		ThrowIfOSErr_(AEPutParamDesc(openEvent, 'pPAS', passwordDesc));
	}
	StAppleEvent theReply;
	openEvent.Send(theReply, kAEFlagsForReply);
}

string
CFileMaker::GetVersionString()
{
	CAEDescriptor versionProp;
	CAEDescriptor nullEnclosure;
	FMAE::GetPropertyDescriptor(pVersion, versionProp, nullEnclosure);
	StAppleEvent theEvent;
	StAppleEvent theReply;
	FMAE::NewGetDataAppleEvent(fCreator, theEvent, versionProp, kAnyTransactionID);
	theEvent.Send(theReply, kAEFlagsForReply);
	DescType typeCode;
	long actualSize;
	const int versionBufferSize = 128;
	char versionBuffer[versionBufferSize];
	ThrowIfOSErr_(::AEGetParamPtr(theReply, keyAEResult, typeChar, 
		&typeCode, versionBuffer, versionBufferSize, &actualSize));
	return string(versionBuffer, actualSize);
}

void
CFileMaker::Open(const FSSpec& databaseSpec, AEDesc *databaseDescriptor)
{
	bool launched = false;
	if (!WasInitialized())
		launched = Initialize(databaseSpec);

	if (!launched)
		SendOpenEvent(databaseSpec);

	CAEDescriptor docName(databaseSpec.name);
	CAEDescriptor nullDesc;
	ThrowIfOSErr_(CreateObjSpecifier(cDatabase, nullDesc, formName, docName, false, databaseDescriptor));

}

void
CFileMaker::GetDatabaseDescriptor(const string& databaseName, AEDesc *databaseDescriptor)
{
	CAEDescriptor docName(AsStr255(databaseName));
	CAEDescriptor nullDesc;
	ThrowIfOSErr_(CreateObjSpecifier(cDatabase, nullDesc, formName, docName, false, databaseDescriptor));
}


void
CFileMaker::Close(const AEDesc &databaseDescriptor)
{
	StAppleEvent theEvent(kCoreEventClass, kAEClose, fAddress);
	CAEDescriptor docDescList;
	ThrowIfOSErr_(AECreateList(0, 0, false, docDescList));
	CAEDescriptor docDesc(&databaseDescriptor);
	ThrowIfOSErr_(AEPutDesc(docDescList, 0, docDesc));
	ThrowIfOSErr_(AEPutParamDesc(theEvent, keyDirectObject, docDescList));
	theEvent.Send();
}

void
CFileMaker::QuitFileMaker()
{
	StAppleEvent theEvent(kCoreEventClass, kAEQuitApplication, fAddress);
	theEvent.Send();
}

void
CFileMaker::Release()
{
	if (fInitialized) {
		fInitialized = false;
		if (gQuitMode == kAlwaysQuit || (!fWasRunning && (gQuitMode == kQuitIfLaunched)))
			QuitFileMaker();
	}
}

Boolean
CFileMaker::WasInitialized()
{
	return fInitialized && FileMakerIsRunning();
}

void
CFileMaker::ParseVersionString(const string& versionString)
{
	if (versionString.length() < 5)
		return;
		
	fMajorVersion = versionString[0] - '0';
	fMinorVersion = versionString[2] - '0';
	fUpdateVersion = versionString[4] - '0';
}

short
CFileMaker::MajorVersion()
{
	return fMajorVersion;
}

short
CFileMaker::MinorVersion()
{
	return fMinorVersion;
}

short
CFileMaker::UpdateVersion()
{
	return fUpdateVersion;
}


bool CFileMaker::CreatorIsFileMaker(OSType creator)
{
	for (int i = 0; i < kNumTypes; ++i) {
		if (gTypes[i] == creator)
			return true;
	}
	return false;
}

bool CFileMaker::WaitUntilFileMakerIsRunning()
{
	UInt32 endTicks = TickCount() + 1200;
	bool isFound = false;
	do {
		isFound = FileMakerIsRunning();
	} while (!isFound && TickCount() < endTicks);
	return isFound;
}

bool CFileMaker::FileMakerIsRunning()
{
	ProcessSerialNumber currPSN;
	ProcessInfoRec currProcessInfo;
	FSSpec applicationSpec;
	Boolean foundRunningProcessFlag;

	currPSN.lowLongOfPSN = kNoProcess;
	currPSN.highLongOfPSN = 0;
	
	currProcessInfo.processInfoLength = sizeof(ProcessInfoRec);
	currProcessInfo.processName = nil;
	currProcessInfo.processAppSpec = &applicationSpec;
	
	foundRunningProcessFlag = false;
	while (GetNextProcess(&currPSN) == noErr) {
		if (GetProcessInformation(&currPSN, &currProcessInfo) == noErr) {
			if (currProcessInfo.processSignature == fCreator) {
				foundRunningProcessFlag = true;
				fFilemakerPSN = currPSN;
				break;
			}
		}
	}
	return foundRunningProcessFlag;
}

int CFileMaker::QuitMode()
{
	return gQuitMode;
}

void CFileMaker::SetQuitMode(int quitMode)
{
	gQuitMode = quitMode;
}


bool
CFileMaker::AskForFileMakerDatabase(FSSpec& outSpec, OSType& outType)
{
	PP_StandardDialogs::LFileChooser	chooser;
			
	NavDialogOptions*	options = chooser.GetDialogOptions();
	if (options != nil) {
		options->dialogOptionFlags =	kNavDefaultNavDlogOptions
										+ kNavSelectAllReadableItem;
	}

	LFileTypeList fTypeList(kNumTypes, (OSType*)gTypes);
	if (chooser.AskOpenFile(fTypeList)) {
		chooser.GetFileSpec(1, outSpec);
		outType = Folders::GetFileType(outSpec);
		return true;
	}
	return false;
}

static bool
MyAskChooseOneFile(
	const LFileTypeList&	inFileTypes,
	FSSpec&					outFileSpec)
{
									// Create UPPs for callback functions
	UNavServicesDialogs::StNavReplyRecord		mNavReply;
	NavDialogOptions		mNavOptions = {};
	StAEDescriptor			mDefaultLocation;

	mNavReply.SetDefaultValues();

									// Can choose only one file
	mNavOptions.dialogOptionFlags &= ~kNavAllowMultipleFiles;

									// Set default location, the location
									//   that's displayed when the dialog
									//   first appears
	AEDesc*		defaultLocationDesc = nil;
	if (not mDefaultLocation.IsNull()) {
		defaultLocationDesc = mDefaultLocation;

		mNavOptions.dialogOptionFlags |= kNavSelectDefaultLocation;
	}

	OSErr err = ::NavChooseFile(
						defaultLocationDesc,
						mNavReply,
						&mNavOptions,
						0,
						0,
						0,
						inFileTypes.TypeListHandle(),
						0L);							// User Data


	if ( (err != noErr) && (err != userCanceledErr) ) {
		Throw_(err);
	}

	if (mNavReply.IsValid()) {
		mNavReply.GetFileSpec(outFileSpec);
	}

	return mNavReply.IsValid();
}

bool
CFileMaker::AskForFileMakerDatabasePlus(OSType extraType, FSSpec& outSpec, OSType& outType)
{
	vector<OSType> typesPlus;
	copy(gTypes, kNumTypes+gTypes, back_inserter(typesPlus));
	typesPlus.push_back(extraType);
			
	LFileTypeList fTypeList(typesPlus.size(), (OSType*)&typesPlus[0]);
	if (MyAskChooseOneFile(fTypeList, outSpec)) {
		outType = Folders::GetFileType(outSpec);
		return true;
	}
	return false;
}

vector<CDatabaseName>
CFileMaker::GetAllDatabaseNames()
{
	vector<CDatabaseName> results;
	if (FileMakerIsRunning()) {
		int count = GetDatabaseCount();
		for (int i = 1; i <= count; i += 1) {
			results.push_back(GetDatabaseName(i));
		}
	}
	return results;
}

void CFileMaker::HandleLaunchWithDocumentFailed(OSErr err, const FSSpec& databaseSpec)
{
	if (err == noErr)
		return;
		
	if (!FileMakerIsRunning())
		Throw_(kNoFileMakerErr);
		
	if (ShouldTryWithPassword(fCreator, err)) {
		AskPasswordAndSendOpenEvent(databaseSpec);
	} else {
		Throw_(err);
	}
}

bool CFileMaker::ShouldTryWithPassword(OSType creator, OSErr err)
{
	return ((creator == kUSFileMaker5Creator || creator == kJapaneseFileMaker5Creator) && err == errAEEventFailed);
}

static string
ParamAsString(StAppleEvent& desc, unsigned long key)
{
	DescType typeCode;
	long actualSize;
	ThrowIfOSErr_(AEGetParamPtr(desc, key, typeChar, &typeCode, 0, 0, &actualSize));
	vector<char> buffer(actualSize);
	ThrowIfOSErr_(AEGetParamPtr(desc, key, typeChar, &typeCode, &buffer[0], buffer.size(), &actualSize));
	return string(&buffer[0], actualSize);
}

CDatabaseName
CFileMaker::GetDatabaseName(int index)
{
	CAEDescriptor database;
	CAEDescriptor databaseProperty;
	FMAE::GetDocumentDescriptor(index, database);
	FMAE::GetPropertyDescriptor(pName, databaseProperty, database);
	StAppleEvent theEvent;
	StAppleEvent theReply;
	FMAE::NewGetDataAppleEvent(fCreator, theEvent, databaseProperty, kAnyTransactionID);
	theEvent.Send(theReply, kAEFlagsForConduit);
	CAEDescriptor result;
	return CDatabaseName(ParamAsString(theReply, keyDirectObject), fCreator);
}

int
CFileMaker::GetDatabaseCount()
{
	CAEDescriptor defaultDesc;
	CAEDescriptor database((unsigned long)cDocument);
	StAppleEvent theEvent;
	StAppleEvent theReply;
	FMAE::NewCountAppleEvent(fCreator, theEvent, defaultDesc, database);
	theEvent.Send(theReply, kAEFlagsForConduit);
	DescType typeCode;
	long actualSize;
	long theCount;
	ThrowIfOSErr_(::AEGetParamPtr(theReply, keyAEResult, typeLongInteger, 
		&typeCode, &theCount, sizeof(theCount), &actualSize));
	return theCount;
}

namespace FileMaker {

class NameCollector {
public:
	NameCollector(vector<CDatabaseName> &allN) : fAllN(allN) {};
	
	void operator () (const OSType& c)
	{
		vector<CDatabaseName> allN(CFileMaker::Get(c).GetAllDatabaseNames());
		copy(allN.begin(), allN.end(), back_inserter(fAllN));
	}
	
	vector<CDatabaseName> &fAllN;
};

vector<CDatabaseName> GetAllDatabaseNamesFromAllVersions()
{
	vector<OSType> allC(CFileMaker::GetAllCreators());
	vector<CDatabaseName> allNames;
	NameCollector collector(allNames);
	for_each(allC.begin(), allC.end(), collector);
	return allNames;
}

}