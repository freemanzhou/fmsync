#include <LPushButton.h>
#include <LListBox.h>
#include <LRadioButton.h>
#include <LTextGroupBox.h>
#include <LStaticText.h>
#include <LPeriodical.h>
#include <LPopupButton.h>

#include <iostream>
#include <sstream>

#include "CConfigDialog.h"
#include "CStringTable.h"
#include "CEditDBDialog.h"
#include "CEditNamesDialog.h"
#include "CDataTasksDialog.h"
#include "CRegisterDialog.h"
#include "CUploadDBDialog.h"
#include "CDataTask.h"
#include "FMDatabase.h"
#include "CFileMaker.h"
#include "CCreditsDialog.h"
#include "CConduitSettings.h"
#include "CJFileSynchronizer.h"
#include "CDatabaseFile.h"
#include "FMCResources.h"
#include "JFileConduit.h"
#include "LErrorMsg.h"
#include "Utilities.h"
#include "SaveToFile.h"
#include "Str255.h"
#include "Stringiness.h"
#include "syncmgr.h"
#include "ErrorStrings.h"
#include "OtherStrings.h"
#include "LErrorMsg.h"
#include "ErrorCodes.h"
#include "CFrontProcess.h"
#include "CChooseAccessMethod.h"
#include "CSelectFileMakerDatabase.h"

#if SELF_TESTS
#include "SelfTestConduitWindow.h"
#endif

#if __profile__
#include "Profiler.h"
#endif

#if 0
static bool gNeedsRegister = false;
static bool gRegFailed = false;

class CRegisterIdler : public LPeriodical
{
public:
					CRegisterIdler(CConfigDialog* theDialog);
	virtual			~CRegisterIdler();
	
	virtual	void	SpendTime(
							const EventRecord&		inMacEvent);
	
private:
	CConfigDialog*	fDialog;
	long			fCheckRegister;
};

class CRegFailedIdler : public LPeriodical
{
public:
					CRegFailedIdler(CConfigDialog* theDialog);
	virtual			~CRegFailedIdler();
	
	virtual	void	SpendTime(
							const EventRecord&		inMacEvent);
	
private:
	CConfigDialog*	fDialog;
	long			fCheckFailed;
};

CRegisterIdler::CRegisterIdler(CConfigDialog* theDialog)
	: fDialog(theDialog)
{
	fCheckRegister = TickCount() + 15;
}

CRegisterIdler::~CRegisterIdler()
{
}

void
CRegisterIdler::SpendTime(const EventRecord&)
{
	if (gNeedsRegister && TickCount() > fCheckRegister) {
		fCheckRegister = TickCount() + 15;
		fDialog->DoCheckRegistration();
		//fDialog->SetIdler(new CRegFailedIdler(fDialog));
		gNeedsRegister = false;
	}
}


CRegFailedIdler::CRegFailedIdler(CConfigDialog* theDialog)
	: fDialog(theDialog)
{
	fCheckFailed = TickCount() + 15;
}

CRegFailedIdler::~CRegFailedIdler()
{
}

void
CRegFailedIdler::SpendTime(const EventRecord&)
{
	if (gRegFailed) {
		fDialog->AnnounceRegFailed();
		//fDialog->SetIdler(new CRegisterIdler(fDialog));
		gRegFailed = false;
	}
}
#endif

CConfigDialog::CConfigDialog(CSyncPreference& inSyncProperties, const FSSpec& settingsSpec)
	: CConduitDialog(kWINDConduitSettings), fSyncProperties(inSyncProperties), fSettings(0)
{
	fUserName = AsString(fSyncProperties.u.m_UserDirFSSpec.name);
	fSettings = new CConduitSettings(&settingsSpec);

	//SetIdler(new CRegisterIdler(this));
}

CConfigDialog::~CConfigDialog()
{
	try {
		if (fSettings) {
			fSettings->SetQuitMode(fQuitModeMenu->GetValue());
			fSettings->UpdateFile();
		}
	} catch(...) {
		string cantUpdateSettings(LoadString(kFMJErrorStrings, kErrorUpdatingSettingsIndex));
		LErrorMsg::AlertWithMessage(kAlertStopAlert, cantUpdateSettings);
	}
	delete fSettings;
}

static void
SafeSetDescriptor(LPushButton* theButton, ConstStringPtr newDescriptor)
{
	Str255 buttonDesc;
	theButton->GetDescriptor(buttonDesc);
	if (CompareString(newDescriptor, buttonDesc, 0) != 0) {
		theButton->SetDescriptor(newDescriptor);
	}
}

void
CConfigDialog::UpdateControlState()
{
	int index;
	if (fList->SelectedIndex(index)) {
		fEditButton->Enable();
		fDeleteButton->Enable();
		fDisableButton->Enable();
		CDatabaseFile::Ptr file = fSettings->GetDatabase(index);
		if (file->GetDisabled())
			SafeSetDescriptor(fDisableButton, "\pEnable");
		else
			SafeSetDescriptor(fDisableButton, "\pDisable");
	} else {
		fEditButton->Disable();
		fDeleteButton->Disable();
		fDisableButton->Disable();
		SafeSetDescriptor(fDisableButton, "\pDisable");
	}
}

void
CConfigDialog::ListenToMessage(MessageT inMessage, void* param)
{
	try {
		if (inMessage == 'Cred') {
			DoCredits();
		} else if (inMessage == 'AddD') {
			DoAdd();
		} else if (inMessage == 'Edit') {
			DoEditDatabase();
		} else if (inMessage == 'Disa') {
			DoDisableDatabase();
		} else if (inMessage == 'Remo') {
			DoRemoveDatabase();
		} else if (inMessage == 'Uplo') {
			DoUploadDatabase();
#if SELF_TESTS
		} else if (inMessage == 'Test') {
			DoTest();
#endif
#ifdef REGISTRATION
		} else if (inMessage == 'Regi') {
			DoRegister();
#endif
		} else if (inMessage == 'Sync' && *(SInt32*)param) {
			DoSetSyncAction(CConduitSettings::kSyncDatabases);
		} else if (inMessage == 'Dont' && *(SInt32*)param) {
			DoSetSyncAction(CConduitSettings::kDoNothing);
		} else if (inMessage == 'Defa') {
			DoSetDefaultSyncAction(fSettings->SyncAction());
		} else
			CConduitDialog::ListenToMessage(inMessage, param);
	} catch (const LException& inErr) {
		if (inErr.GetErrorCode() == kCannotFindFileMakerErr) {
			string couldNotComplete(LoadString(kFMJErrorStrings, kCouldNotCompleteIndex));
			couldNotComplete.append(ConvertErrorToString(inErr.GetErrorCode()));
			string rebuildDesktop(LoadString(kFMJErrorStrings, kRebuildDesktopIndex));
			LErrorMsg::AlertWithMessageAndDetails(kAlertStopAlert, couldNotComplete, rebuildDesktop);
		} else if (!IsQuietError(inErr.GetErrorCode())) {
			string couldNotComplete(LoadString(kFMJErrorStrings, kCouldNotCompleteIndex));
			couldNotComplete.append(ConvertErrorToString(inErr.GetErrorCode()));
			LErrorMsg::AlertWithMessage(kAlertStopAlert, couldNotComplete);
		}
	}
}

void
CConfigDialog::DoSetSyncAction(int syncAction)
{
	fSettings->SetSyncAction(syncAction);
	UpdateWindow();
}

void
CConfigDialog::DoSetDefaultSyncAction(int syncAction)
{
	fSettings->SetDefaultSyncAction(syncAction);
	UpdateWindow();
}

void
CConfigDialog::DoCredits()
{
	CCreditsDialog dialog(fSettings);
	dialog.DoDialog();
}

void
CConfigDialog::HandleLoadSettingsFile(const FSSpec& theSpec)
{
	CDatabaseFile::Ptr file = CDatabaseFile::ReadSettingsFile(theSpec);
	PrepareAndEditNewDatabase(file);
}

void
CConfigDialog::DoAddDiskDatabase()
{
	FSSpec spec;
	OSType fileType;
	if (CFileMaker::AskForFileMakerDatabasePlus('pref', spec, fileType)) {
		if (fileType == kFMP21Creator) {
			string m1(LoadString(kOtherStrings, kFileMaker211Index));
			string m2(LoadString(kOtherStrings, kFileMaker212Index));
			LErrorMsg::AlertWithMessageAndDetails(kAlertNoteAlert, m1, m2);
		} else if (fileType == 'pref') {
			HandleLoadSettingsFile(spec);
		} else {
			CDatabaseFile::Ptr f(new CDatabaseFile(spec));
			PrepareNewDatabaseFile(f);
			PrepareAndEditNewDatabase(f);
		}
	}
}

void
CConfigDialog::PrepareNewDatabaseFile(CDatabaseFile::Ptr)
{
}

void
CConfigDialog::PrepareAndEditNewDatabase(CDatabaseFile::Ptr f)
{
	f->SetNewInstall(true);
	CWarnings warnings(fSettings->GetWarnings());
	CEditDBDialog dialog(f, warnings);
	if (!dialog.DoDialog()) {
		fSettings->AddDatabase(f);
		UpdateWindow();
	}
	fSettings->SetWarnings(warnings);
}

void
CConfigDialog::ComplainAboutNoDatabases()
{
	string m1(LoadString(kFMJErrorStrings, kNoDatabasesOpenIndex));
	string m2(LoadString(kFMJErrorStrings, kNoDatabasesOpenIndex+1));
	LErrorMsg::AlertWithMessageAndDetails(kAlertNoteAlert, m1, m2);
}

void
CConfigDialog::ComplainAboutDatabaseNotOpen(const string& theName)
{
	string m1(LoadString(kFMJErrorStrings, kNamedDatabaseNotOpenIndex));
	Substitute(m1, "%%1", theName);
	string m2(LoadString(kFMJErrorStrings, kNamedDatabaseNotOpenIndex+1));
	LErrorMsg::AlertWithMessageAndDetails(kAlertNoteAlert, m1, m2);
}

void
CConfigDialog::DoAddExistingDatabase()
{
	vector<CDatabaseName> names(FileMaker::GetAllDatabaseNamesFromAllVersions());
	
	if (names.size() == 0) {
		ComplainAboutNoDatabases();
		return;
	}
		
	CSelectFileMakerDatabase select(names);
	if (!select.DoDialog()) {
		CDatabaseFile::Ptr f(new CDatabaseFile(select.DatabaseName()));
		PrepareNewDatabaseFile(f);
		PrepareAndEditNewDatabase(f);
	}
}

pair<bool,bool>
CConfigDialog::DoChooseMethod(bool chooseLocal)
{
	CChooseAccessMethod method(chooseLocal);
	bool cancelled = method.DoDialog();
	bool useLocal = method.UseLocal();
	return make_pair(cancelled, useLocal);
}

void
CConfigDialog::DoAddDatabase()
{
	FSSpec spec;
	OSType fileType;
	if (CFileMaker::AskForFileMakerDatabase(spec, fileType)) {
		if (fileType == kFMP21Creator) {
			string m1(LoadString(kOtherStrings, kFileMaker211Index));
			string m2(LoadString(kOtherStrings, kFileMaker212Index));
			LErrorMsg::AlertWithMessageAndDetails(kAlertNoteAlert, m1, m2);
		} else {
			CDatabaseFile::Ptr f(new CDatabaseFile(spec));
			f->SetNewInstall(true);
			CWarnings warnings(fSettings->GetWarnings());
			CEditDBDialog dialog(f, warnings);
			if (!dialog.DoDialog()) {
				fSettings->AddDatabase(f);
				UpdateWindow();
			}
			fSettings->SetWarnings(warnings);
		}
	}
}

void
CConfigDialog::DoAdd()
{
#ifdef _NETWORK
	bool chooseLocal = (fSettings->LastAccessMode() == CConduitSettings::kAccessByFile);
	pair<bool,bool> results = DoChooseMethod(chooseLocal);
	if (results.first)
		return;
	fSettings->SetLastAccessMode(results.second?CConduitSettings::kAccessByFile:CConduitSettings::kAccessByName);
	if (results.second)
		DoAddDiskDatabase();
	else
		DoAddExistingDatabase();
#else
	DoAddDiskDatabase();
#endif
}

void
CConfigDialog::DoRemoveDatabase()
{
	int index;
	if (fList->SelectedIndex(index)) {
		string m1(LoadString(kOtherStrings, kSureToRemoveIndex));
		string m2(LoadString(kOtherStrings, kUseDisableInsteadIndex));
		int itemHit = LErrorMsg::AlertWithMessageAndDetails(kAlertNoteAlert, m1, m2, true);
		if (itemHit == kAlertStdAlertOKButton) {
			fSettings->RemoveDatabase(fSettings->GetDatabase(index));
			UpdateWindow();
		}
	}
}

void
CConfigDialog::DoUploadDatabase()
{
	gBusy = true;
	CUploadDBDialog dialog(fSettings->GetLastSyncTime(), fSettings->GetJFileNames(), fSettings->GetUploadRequests());
	if (!dialog.DoDialog())
		fSettings->SetUploadRequests(dialog.GetUploadRequests());
	UpdateWindow();
}

#if 0
void
CConfigDialog::DoRegister()
{
	CRegisterDialog dialog(fUserName, fSettings->RegCode());
	gNeedsRegister = !dialog.DoDialog();
	fPotentialRegCode = dialog.RegCode();
}

void
CConfigDialog::DoCheckRegistration()
{
	try {
		CheckRegistration(fUserName + kProductName, fPotentialRegCode);
		fSettings->SetRegCode(fPotentialRegCode);
	} catch (...) {
		gRegFailed = true;
	}
}
#endif

void
CConfigDialog::AnnounceRegFailed()
{
	string m1("The registration code supplied is not correct. Please check your registration code and try again.");
	LErrorMsg::AnnounceError(m1);
}

#if SELF_TESTS
void
CConfigDialog::DoTest()
{
	SelfTestConduitWindow window;
	window.DoDialog();
}
#endif

void
CConfigDialog::DoEditDatabase()
{
	CFrontProcess fp;
	int index;
	if (fList->SelectedIndex(index)) {
		CDatabaseFile::Ptr file = fSettings->GetDatabase(index);
		if (!file->HasFile()) {
			string m1(LoadString(kFMJErrorStrings, kCannotFindFilemakerFile));
			string m2(LoadString(kFMJErrorStrings, kCannotFindFilemakerFile+1));
			FSSpec spec;
			file->GetFSSpec(&spec);
			Substitute(m2, "%%1", AsString(spec.name));
			if (LErrorMsg::AlertWithMessageAndDetails(kAlertNoteAlert, m1, m2, true) == 1) {
				OSType fileType;
				if (CFileMaker::AskForFileMakerDatabase(spec, fileType)) {
					if (fileType == kFMP21Creator) {
						string m1(LoadString(kOtherStrings, kFileMaker211Index));
						string m2(LoadString(kOtherStrings, kFileMaker212Index));
						LErrorMsg::AlertWithMessageAndDetails(kAlertNoteAlert, m1, m2);
					} else {
						file->SetFSSpec(spec);
						DoEditDatabase();
					}
				}
			}
		} else {
			gBusy = true;
			CDatabaseFile::Ptr theFile = fSettings->GetDatabase(index);
			CWarnings warnings(fSettings->GetWarnings());
#if __profile__
			ProfilerClear();
#endif
			CEditDBDialog dialog(theFile, warnings);
			dialog.DoDialog();
#if __profile__
			ProfilerDump("\pEditDB.prof");
#endif
			fSettings->SetWarnings(warnings);
			UpdateWindow();
		}
	}
}

void
CConfigDialog::DoDisableDatabase()
{
	int index;
	if (fList->SelectedIndex(index)) {
		CDatabaseFile::Ptr file = fSettings->GetDatabase(index);
		file->SetDisabled(!file->GetDisabled());
		UpdateWindow();
		fList->SelectIndex(index);
	}
}

void
CConfigDialog::UpdateWindow()
{
	fList->SelectNone();
	
	vector<string> theFileNames;
	int dbCount = fSettings->DatabaseCount();
	for (int i = 0; i < dbCount; i += 1) {
		CDatabaseFile::Ptr file = fSettings->GetDatabase(i);
		string databaseString("  ");
		if (!file->GetDisabled()) {
			databaseString = string("Ã ");
		}
		CDatabaseName databaseName(file->GetFileMakerDatabaseName());
		databaseString.append(databaseName.GetName());
		string jfileName(file->GetPilotDatabaseName());
		if (databaseName.GetName() != jfileName) {
			databaseString.append(" (");
			databaseString.append(jfileName);
			databaseString.append(")");
		}
		theFileNames.push_back(databaseString);
	}
	fList->SetStrings(theFileNames);
	fList->Refresh();
	UpdateDefaultAction();
}

void
CConfigDialog::UpdateDefaultAction()
{
	int defaultAction = fSettings->DefaultSyncAction();
	LStr255 actionpString(kSTRSyncActionStrings, defaultAction+1);
	LStr255 actionLabelpString(kSTRSyncActionStrings, 3);
	string actionString(AsString(actionLabelpString));
	actionString += AsString(actionpString);
	fDefaultAction->SetDescriptor(AsStr255(actionString));
}

void
CConfigDialog::FinishCreateSelf()
{
	FindPane(fView, 'AddD', &fAddButton);
	FindPane(fView, 'Edit', &fEditButton);
	FindPane(fView, 'Remo', &fDeleteButton);
	FindPane(fView, 'List', &fList);
	FindPane(fView, 'Sync', &fSync);
	FindPane(fView, 'Dont', &fDoNothing);
	FindPane(fView, 'data', &fDatabaseBox);
	FindPane(fView, 'Defa', &fDefaultAction);
	FindPane(fView, 'Disa', &fDisableButton);
	FindPane(fView, 'quit', &fQuitModeMenu);

#if SELF_TESTS
	FindPane(fView, 'Test', &fTestButton);
	fTestButton->Enable();
	fTestButton->Show();
#endif

	fList->SetDoubleClickCommand('Edit');
	
	Str255 newTitle;
	fDatabaseBox->GetDescriptor(newTitle);
	LString::AppendPStr(newTitle, fSyncProperties.u.m_UserDirFSSpec.name);
	LString::AppendPStr(newTitle, "\pÓ");
	fDatabaseBox->SetDescriptor(newTitle);

	fList->AddListener(this);
	fDefaultAction->AddListener(this);

	if (fSettings->SyncAction() == CConduitSettings::kSyncDatabases) {
		fSync->SetValue(1);
	} else {
		fDoNothing->SetValue(1);
	}
	
	fQuitModeMenu->SetValue(fSettings->QuitMode());

	UpdateWindow();
}

