#include <LStaticText.h>
#include <LGAStaticTextImp.h>
#include <LAMStaticTextImp.h>
#include <LPushButton.h>

#include <iostream>
#include <sstream>

#include "CConduitSettings.h"
#include "CCreditsDialog.h"
#include "CDatabaseFile.h"
#include "CFileMaker.h"
#include "LErrorMsg.h"
#include "FMCResources.h"
#include "OtherStrings.h"
#include "SaveToFile.h"
#include "Stringiness.h"
#include "Str255.h"
#include "VersionUtilities.h"
#include "FolderUtilities.h"

using namespace Folders;
using namespace Version;

CCreditsDialog::CCreditsDialog(CConduitSettings* s)
	: CConduitDialog(kWINDCredits), fSettings(s)
{
}

CCreditsDialog::~CCreditsDialog()
{
}

void
CCreditsDialog::UpdateWindow()
{
	StResource versR('vers', 1, false, true);
	Handle h = versR;
	if (h) {
		LStr255 captionString;

		VersPtr versPtr = (VersPtr)*h;
		
		LStr255 versionString((long)versPtr->majorVersion);
		versionString += '.';
		long minorVersion = versPtr->minorVersion >> 4;
		long pointVersion = versPtr->minorVersion & 0x0f;
		versionString += minorVersion;
		if (pointVersion) {
			versionString += '.';
			versionString += pointVersion;
		}
		if (versPtr->releaseStage != final) {
			char releaseChar;
			switch(versPtr->releaseStage) {
			case alpha:
				releaseChar = 'a';
				break;
			case beta:
				releaseChar = 'b';
				break;
			case development:
				releaseChar = 'd';
				break;
			}
			versionString += releaseChar;
			versionString += (long)versPtr->nonFinalRelease;
		}
		#ifdef DEMO
		versionString += " Demo";
		#endif
		
		fVersionText->GetDescriptor(captionString);
		captionString += versionString;
		fVersionText->SetDescriptor(captionString);
	}

#ifdef CHECK_EXPIRATION_DATE	
	DateTimeRec exDate;
	SetupExpirationDate(exDate);
	LStr255 expDate;
	UInt32 expirationSecs;
	DateToSeconds(&exDate, &expirationSecs);
	DateString(expirationSecs, longDate, expDate, 0);

	LStr255 expString;
	fExpirationText->GetDescriptor(expString);
	expString += expDate;
	fExpirationText->SetDescriptor(expString);
	fExpirationText->Show();
#endif
}

void
CCreditsDialog::RegisterClasses()
{
	CConduitDialog::RegisterClasses();
	static Boolean firstRun = true;
	if (firstRun) {
		firstRun = false;
		RegisterClass_(LStaticText);
		if (UEnvironment::HasFeature (env_HasAppearance)) {
			RegisterClassID_(LAMStaticTextImp, LStaticText::imp_class_ID);
		} else {
			RegisterClassID_( LGAStaticTextImp,			LStaticText::imp_class_ID );
		}
	}
}

void
CCreditsDialog::FinishCreateSelf()
{
	FindPane(fView, 'vers', &fVersionText);
	FindPane(fView, 'expi', &fExpirationText);

	LPushButton* dataButton;
	FindPane(fView, 'dAta', &dataButton);
	dataButton->Enable();
	dataButton->Show();
	dataButton->AddListener(this);
	
	UpdateWindow();
}

static void OutputField(std::ostream& s, const std::string& label, const std::string& fieldV)
{
	s << label << " " << fieldV << "\r";
}

static void OutputField(std::ostream& s, const std::string& label, int fieldV)
{
	OutputField(s, label, AsString(fieldV));
}

static void OutputBooleanField(std::ostream& s, const std::string& label, bool value)
{
	OutputField(s, label, value?"true":"false");
}

static SInt32 GetGestaltValue(OSType inSelector)
{
	SInt32	response;
	OSErr err = ::Gestalt(inSelector, &response);
	return response;
}

static std::string MachineName()
{
	return LoadString(kMachineNameStrID, GetGestaltValue(gestaltMachineType));
}

static void OutputFileLocation(std::ostream& s, const FSSpec& fs)
{
	OutputField(s, "   Volume name:", VolumeName(fs.vRefNum));
	FSSpec parent(MakeFSSpec("", fs.vRefNum, fs.parID));
	OutputField(s, "   Folder name:", AsString(parent.name));
	OutputField(s, "   File name:", AsString(fs.name));
}

static FSSpec GetExtension(const std::string& name)
{
	short v;
	long p;
	ThrowIfOSErr_(::FindFolder(kOnSystemDisk, kExtensionFolderType, true, 
				&v, &p));
	return MakeFSSpec(name, v, p);
}

void
CCreditsDialog::OutputProfileForDatabaseFile(std::ostream& profileInfo, CDatabaseFile::Ptr dFile)
{
	OutputBooleanField(profileInfo, "Disabled:", dFile->GetDisabled());
	OutputField(profileInfo, "Palm name:", dFile->GetPilotDatabaseName());
	OutputField(profileInfo, "FileMaker name:", dFile->GetFileMakerDatabaseName().GetName());
	OutputBooleanField(profileInfo, "Has file:", dFile->HasFile());
	if (dFile->HasFile()) {
		FSSpec fs;
		dFile->GetFSSpec(fs);
		OutputFileLocation(profileInfo, fs);
		OutputField(profileInfo, "   Creator:", CharsAsString(GetFileCreator(fs)));
		OutputField(profileInfo, "   File type:", CharsAsString(GetFileType(fs)));
	}
	OutputBooleanField(profileInfo, "New install:", dFile->NewInstall());
	OutputField(profileInfo, "Layout ID:", dFile->GetLayoutID());
	OutputField(profileInfo, "Presync script ID:", dFile->GetPreSyncScriptID().GetValue());
	OutputField(profileInfo, "Postsync script ID:", dFile->GetPostSyncScriptID().GetValue());
	OutputBooleanField(profileInfo, "Translate text:", dFile->GetTranslateText());
	OutputBooleanField(profileInfo, "Use found set:", dFile->GetUseFoundSet());
	OutputBooleanField(profileInfo, "Duplicate on conflict:", dFile->GetDuplicateOnConflict());
}

const OSType gCreators[] = 
{
	kUSFileMakerCreator, kJapaneseFileMakerCreator, kUSFileMaker5Creator, kJapaneseFileMaker5Creator, kFMP21Creator
};

const char* gFMNames[] = 
{
	"FileMaker Pro 3/4", "FileMaker Pro 3/4 (J)", "FileMaker Pro 5", "FileMaker Pro 5 (J)", "FileMaker Pro 2.1"
};

const int kCreatorsCount = sizeof(gCreators)/sizeof(gCreators[0]);

void
CCreditsDialog::DoProfile()
{
	std::ostringstream profileInfo;
	
	try {
		unsigned long dateTime;
		GetDateTime(&dateTime);
		Str255 dateStr;
		DateString(dateTime, longDate, dateStr, 0);
		OutputField(profileInfo, "Profile Date:", AsString(dateStr));
		TimeString(dateTime, false, dateStr, 0);
		OutputField(profileInfo, "Profile Time:", AsString(dateStr));
		profileInfo << "FMSync Version: ";
		StResource versR('vers', 1, false, true);
		Handle h = versR;
		if (h) {
			profileInfo << VersionAsString(h) << "\r";
		} else {
			profileInfo << "Version resource not found.\r";
		}
		
		OutputField(profileInfo, "Machine Type:", HexAsString(GetGestaltValue(gestaltMachineType)));
		OutputField(profileInfo, "Machine Name:", MachineName());
		OutputField(profileInfo, "CPU:", HexAsString(GetGestaltValue(gestaltNativeCPUfamily)));
		OutputField(profileInfo, "MacOS Version:", VersionAsString(UEnvironment::GetOSVersion() << 16 | 0x8000));
		OutputField(profileInfo, "System Update Version:", VersionAsString(GetGestaltValue(gestaltSystemUpdateVersion)));
		OutputBooleanField(profileInfo, "Has AppleEvents:", UEnvironment::HasGestaltAttribute(gestaltAppleEventsAttr, gestaltAppleEventsPresent));
		if(UEnvironment::HasGestaltAttribute(gestaltAppleEventsAttr, gestaltAppleEventsPresent)) {
			OutputBooleanField(profileInfo, "Scripting Support:", UEnvironment::HasGestaltAttribute(gestaltAppleEventsAttr, gestaltScriptingSupport));
			OutputBooleanField(profileInfo, "OSL In System:", UEnvironment::HasGestaltAttribute(gestaltAppleEventsAttr, gestaltOSLInSystem));
		}
		OutputBooleanField(profileInfo, "Has AppleScript:", UEnvironment::HasGestaltAttribute(gestaltAppleScriptAttr, gestaltAppleScriptPresent));
		if(UEnvironment::HasGestaltAttribute(gestaltAppleScriptAttr, gestaltAppleScriptPresent)) {
			OutputField(profileInfo, "AppleScript Version:", VersionAsString(GetGestaltValue(gestaltAppleScriptVersion) << 16 | 0x8000));
		}
		OutputBooleanField(profileInfo, "ObjectSupportLib:", FileExists(GetExtension("ObjectSupportLib")));
		if(UEnvironment::HasGestaltAttribute(gestaltUSBAttr, gestaltUSBPresent)) {
			OutputField(profileInfo, "USB Version:", VersionAsString(GetGestaltValue(gestaltUSBVersion)));
		}
		OutputBooleanField(profileInfo, "VM:", UEnvironment::HasGestaltAttribute(gestaltVMAttr, gestaltVMPresent));
		OutputField(profileInfo, "Heap Memory:", BytesAsString(FreeMem()));
		OutputField(profileInfo, "Process Memory:", BytesAsString(TempFreeMem()));
		OutputField(profileInfo, "Largest Block:", BytesAsString(MaxBlock()));
		OutputField(profileInfo, "Disk Space:", VolumeSpaceFree(0));
		OutputField(profileInfo, "HotSync Libraries Version:", FileVersionAsString(MakeFSSpec("HotSync Libraries")));
		OutputField(profileInfo, "Sync Action:", fSettings->SyncAction());
		for (int index = 0; index < kCreatorsCount; index += 1) {
			OSType creator = gCreators[index];
			FSSpec fs;
			bool hasCreator = FindApplication(creator, fs);
			OutputBooleanField(profileInfo, "Has " + string(gFMNames[index]) + ":", hasCreator);
			if (hasCreator) {
				OutputFileLocation(profileInfo, fs);
				OutputField(profileInfo, "   Version:", FileVersionAsString(fs));
			}
		}
		OutputField(profileInfo, "Database count:", fSettings->DatabaseCount());
		for (int index = 0; index < fSettings->DatabaseCount(); index += 1) {
			profileInfo << "\rDatabase #" << index+1 << "\r";
			CDatabaseFile::Ptr df = fSettings->GetDatabase(index);
			OutputProfileForDatabaseFile(profileInfo, df);
		}
	}

	catch (const bad_alloc& inErr)
	{
		profileInfo << "Out of memory exception occured while writing FMSync data.\r";
	}
	
	catch (const LException& inErr)
	{
		profileInfo << "Exception (" << AsString(inErr.GetErrorCode()) << ") occured while writing FMSync data.\r";
	}
	
	catch(...)
	{
		profileInfo << "Exception occured while writing FMSync data.\r";
	}
	
	WriteStringAsFile(MakeFSSpec("FMSyncData.txt", 0, GetDesktopFolderOf(0)), 'ttxt', 'TEXT', profileInfo.str());
	LErrorMsg::AlertWithMessage(kAlertNoteAlert, LoadString(kOtherStrings, kDataFileSavedIndex));
}

void
CCreditsDialog::ListenToMessage(MessageT inMessage, void* param)
{
	if (inMessage == 'data')
		DoProfile();
	else
		CConduitDialog::ListenToMessage(inMessage, param);
}