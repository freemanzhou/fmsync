#include <LPopupButton.h>
#include <LStaticText.h>
#include <LCheckBox.h>
#include <LEditText.h>
#include <LMultiPanelView.h>
#include <LPushButton.h>
#include <LRadioButton.h>
#include <LPopupGroupBox.h>

#include "OtherStrings.h"
#include "LErrorMsg.h"
#include "CEditDBDialog.h"
#include "ErrorStrings.h"
#include "CDatabaseFile.h"
#include "CStringTable.h"
#include "Utilities.h"
#include "Str255.h"
#include "Stringiness.h"
#include "syncmgr.h"
#include "FMDatabase.h"
#include "CJFileSynchronizer.h"
#include "FMCResources.h"
#include "JFile5.h"
#include "WriteJFile5.h"
#include "DebugOutput.h"
#include "CFieldOverrides.h"
#include "CJFileSynchronizer.h"
#include "CFrontProcess.h"
#include "FMSWarnings.h"
#include "CEditPopupDialog.h"
#include "CFMFieldTyper.h"
#include "PaneUtilities.h"

#include "AllFieldTypeHandlers.h"

#include "FMDatabase.h"

using namespace JFile5;

static void
AnnounceStrings(const string& string1, const string& string2)
{
	if (string1.length() > 0) {
		if (string2.length() > 0) {
			LErrorMsg::AlertWithMessageAndDetails(kAlertStopAlert, string1, string2);
		} else {
			LErrorMsg::AlertWithMessage(kAlertStopAlert, string1);
		}
	}
}

CEditDBDialog::CEditDBDialog(CDatabaseFile::Ptr inDB,
	CWarnings& w)
	: CConduitDialog(kWINDDatabaseSettings), CConduitWarner(w), fWarnings(w), 
	fDatabase(inDB), fFMDatabase(0), fLastFieldTypeMenuValue(0), 
	fHandlingFieldTypeChange(false),fLastSelected(-1),
	fOverrideSchedule(false)
{
	SetupJFileFieldTypeMap();
}

CEditDBDialog::~CEditDBDialog()
{
}

void CEditDBDialog::UpdateSchedule()
{
	if (fSchedule->GetValue() > 1) {
		fNextTimeButton->Enable();
	} else {
		fNextTimeButton->Disable();
	}
	
	int resIndex = kEveryHotSyncIndex;
	UInt32 nextSyncTime = fDatabase->GetNextSyncTimeForSchedule(fSchedule->GetValue()-1, fOverrideSchedule);
	if (fSchedule->GetValue() > 1) {
		UInt32 secs;
		GetDateTime(&secs);
		if (nextSyncTime < secs) {
			resIndex = kNextHotSyncIndex;
		} else {
			resIndex = kNextAfterDateIndex;
		}
	}
	string syncString(LoadString(kOtherStrings, resIndex));
	Substitute(syncString, "%%1", DateAsString(nextSyncTime) + " at " + TimeAsString(nextSyncTime));
	if (fSyncScheduleString != syncString) {
		FillStaticTextBox(fNextSyncTimeDescription, syncString);
		fSyncScheduleString = syncString;
	}
}

void
CEditDBDialog::UpdateControlState()
{
	UpdateSchedule();
}

bool
CEditDBDialog::DoSetRemoteDatabaseName()
{
	bool setSucceeded = true;
	Str255 remoteNameStr255;
	fJFileName->GetDescriptor(remoteNameStr255);
	string remoteName(AsString(remoteNameStr255));
	string originalRemoteName(fDatabase->GetPilotDatabaseName());
	
	if (!fDatabase->NewInstall() && remoteName != originalRemoteName) {
			string m1(LoadString(kFMJErrorStrings, kCannotRenameAfterSyncIndex));
			string m2(LoadString(kFMJErrorStrings, kCannotRenameAfterSyncIndex+1));
			LErrorMsg::AlertWithMessageAndDetails(kAlertNoteAlert, m1, m2);
			remoteName = originalRemoteName;
	}
	
	if (remoteName.length() < 1) {
			string m1(LoadString(kFMJErrorStrings, kRemoteNameBlankIndex));
			string m2(LoadString(kFMJErrorStrings, kRemoteNameBlankIndex+1));
			LErrorMsg::AlertWithMessageAndDetails(kAlertNoteAlert, m1, m2);
			setSucceeded = false;
	}
	
	if (setSucceeded) {
		fDatabase->SetPilotDatabaseName(remoteName);
		fRemoteDatabaseName = remoteName;
	} else {
		fJFileName->SetDescriptor(AsStr255(fRemoteDatabaseName));
	}
	return setSucceeded;
}

bool
CEditDBDialog::DefaultLayoutID(int *layoutIDP)
{
	vector<int>::const_iterator i = fLayoutIDs.begin();
	while (i != fLayoutIDs.end()) {
		int layoutID = *i++;
		string string1, string2;
		if (CJFileSynchronizer::LayoutIsOK(fFMDatabase, layoutID, string1, string2)) {
			if (layoutIDP)
				*layoutIDP = layoutID;
			return true;
		}
	}
	return false;
}

#ifdef NETWORK

bool
CEditDBDialog::AskForExportFile(FSSpec& outFile)
{
	LStr255 prompt(kOtherStrings, kSavePromptIndex);
	StandardFileReply reply;
	StandardPutFile(prompt, AsStr255(fRemoteDatabaseName + " Settings"), &reply);
	if (reply.sfGood) {
		outFile = reply.sfFile;
	}
	return reply.sfGood;
}

void
CEditDBDialog::DoExport()
{
	FSSpec outputFile;
	if (AskForExportFile(outputFile)) {
		CDatabaseFile::CreateSettingsFile(outputFile);
		CDatabaseFile::WriteSettingsFile(outputFile, fDatabase, false);
	}
}
#endif

template <class T, class U>
void
EraseEntry(map<T, U> &m, const T& thisOne)
{
	map<T,U>::iterator f = m.find(thisOne);
	if (f != m.end())
		m.erase(f);
}

void
CEditDBDialog::SetToDefaults()
{
	if (fLastSelected != -1) {
		FMAE::FieldID fieldID(GetLastSelectedField());
		fFieldAttributes.fFieldNames[fieldID] = fDefaultFieldAttributes.fFieldNames[fieldID];
		fFieldAttributes.fFieldPopups[fieldID] = fDefaultFieldAttributes.fFieldPopups[fieldID];
		fFieldAttributes.fFieldTypes[fieldID] = fDefaultFieldAttributes.fFieldTypes[fieldID];
		fFieldAttributes.fFieldWidths[fieldID] = fDefaultFieldAttributes.fFieldWidths[fieldID];
		fFieldAttributes.fFieldReadOnly[fieldID] = fDefaultFieldAttributes.fFieldReadOnly[fieldID];
		fFieldAttributes.fFieldExtra[fieldID] = fDefaultFieldAttributes.fFieldExtra[fieldID];
		fFieldAttributes.fFieldExtra2[fieldID] = fDefaultFieldAttributes.fFieldExtra2[fieldID];
		fFieldAttributes.fFieldValue1[fieldID] = fDefaultFieldAttributes.fFieldValue1[fieldID];
		fFieldAttributes.fFieldValue2[fieldID] = fDefaultFieldAttributes.fFieldValue2[fieldID];
		UpdateFieldDisplay(fieldID);
	}
}

int
CEditDBDialog::SelectedLayoutID()
{
	int menuValue = fLayout->GetValue();
	ThrowIf_(menuValue < 1);
	int layoutIndex = menuValue-1;
	return fLayoutIDs[layoutIndex];
}

bool
CEditDBDialog::RememberLayoutChanges()
{
	bool setSucceeded = true;

	int layoutID = SelectedLayoutID();
	string string1, string2;
	if (!CJFileSynchronizer::LayoutIsOK(fFMDatabase, layoutID, string1, string2)) {
		AnnounceStrings(string1, string2);
		setSucceeded = false;
	}

	if (setSucceeded) {
		RememberChanges();
		fDatabase->SetLayoutID(layoutID);
		if (fDatabase->NewInstall()) {
			fDatabase->RememberFieldInfo(fFieldIDs, fFMDatabase->GetFieldRepeats());
		}
	}

	return setSucceeded;
}

void
CEditDBDialog::RememberChanges()
{
	fDatabase->SetFieldOverrides(MakeOverrides());
}

CFieldOverrides
CEditDBDialog::MakeOverrides()
{
	CFieldOverrides newOverrides;
	newOverrides.fName = MakeDiffMap(fDefaultFieldAttributes.fFieldNames, fFieldAttributes.fFieldNames);
	newOverrides.fPopups = MakeDiffMap(fDefaultFieldAttributes.fFieldPopups, fFieldAttributes.fFieldPopups);
	newOverrides.fType = MakeDiffMap(fDefaultFieldAttributes.fFieldTypes, fFieldAttributes.fFieldTypes);
	newOverrides.fWidth = MakeDiffMap(fDefaultFieldAttributes.fFieldWidths, fFieldAttributes.fFieldWidths);
	newOverrides.fReadOnly = MakeDiffMap(fDefaultFieldAttributes.fFieldReadOnly, fFieldAttributes.fFieldReadOnly);
	newOverrides.fExtra = MakeDiffMap(fDefaultFieldAttributes.fFieldExtra, fFieldAttributes.fFieldExtra);
	newOverrides.fExtra2 = MakeDiffMap(fDefaultFieldAttributes.fFieldExtra2, fFieldAttributes.fFieldExtra2);
	newOverrides.fFieldCalcValue1 = MakeDiffMap(fDefaultFieldAttributes.fFieldValue1, fFieldAttributes.fFieldValue1);
	newOverrides.fFieldCalcValue2 = MakeDiffMap(fDefaultFieldAttributes.fFieldValue2, fFieldAttributes.fFieldValue2);
	
	return newOverrides;
}

void
CEditDBDialog::ListenToMessage(MessageT inMessage, void* param)
{
	switch(inMessage) {
#ifdef NETWORK
	case 'Expo':
		DoExport();
		break;
#endif
	case CStringTable::kSelectionChanged:
		HandleSelectedFieldMessage();
		break;
	case 'defa':
		SetToDefaults();
		break;
	case 'NeTi':
		fOverrideSchedule = true;
		break;
	case 'Sche':
		fOverrideSchedule = false;
		break;
	case 'type':
		HandleFieldTypeChangedMessage();
		break;
	case 'LayO':
		gBusy = true;
		UnselectField();
		UseLayout(SelectedLayoutID());
		break;
	case 'sync':
		SetupSyncDescription();
		break;
	default:
		CConduitDialog::ListenToMessage(inMessage, param);
		break;
	}
}

void
CEditDBDialog::FindPanes()
{
	FindPane(fView, 'mpvw', &fMPV);
	fMPV->CreateAllPanels();
	FindPane(fView, 'ftyp', &fFieldTypeMPV);
	fFieldTypeMPV->CreateAllPanels();
	FindPane(fView, 'Expo', &fExportButton);
	FindPane(fView, 'LayO', &fLayout);
	FindPane(fView, 'PreS', &fPreSyncScript);
	FindPane(fView, 'Post', &fPostSyncScript);
	FindPane(fView, 'Tran', &fTranslateText);
	FindPane(fView, 'FSet', &fUseFoundSet);
	FindPane(fView, 'read', &fReadOnly);
	FindPane(fView, 'JNam', &fJFileName);
	FindPane(fView, 'flds', &fFieldNamesList);
	FindPane(fView, 'widt', &fWidthEdit);
	FindPane(fView, 'name', &fFieldNameEdit);
	FindPane(fView, 'type', &fFieldTypeBox);
	FindPane(fView, 'sync', &fSyncModeMenu);
	FindPane(fView, 'JVer', &fJFileVersionMenu);
	FindPane(fView, 'dupl', &fDuplicateOnConflict);
	FindPane(fView, 'defa', &fDefaults);
	FindPane(fView, 'dsyn', &fSyncModeDescription);
	FindPane(fView, 'UNam', &fUserNameField);
	FindPane(fView, 'Sche', &fSchedule);
	FindPane(fView, 'NeTi', &fNextTimeButton);
	FindPane(fView, 'SyDe', &fNextSyncTimeDescription);
}

void
CEditDBDialog::SetupListeners()
{
	fFieldNamesList->AddListener(this);
	fDefaults->AddListener(this);
	fFieldTypeBox->AddListener(this);
	fLayout->AddListener(this);
	fSyncModeMenu->AddListener(this);
	fNextTimeButton->AddListener(this);
	fSchedule->AddListener(this);
}

#ifdef JFILE_CALCULATIONS
enum {
	kFieldTypeMenuString = 1, kFieldTypeMenuCheckbox, 
	kFieldTypeMenuDate, kFieldTypeMenuTime, kFieldTypeMenuPopup,
	kFieldTypeMenuInt, kFieldTypeMenuFloat, kFieldTypeMenuBool, 
	kFieldTypeMenuCalc, kFieldTypeMenuCount};
#else
enum {
	kFieldTypeMenuString = 1, kFieldTypeMenuCheckbox, 
	kFieldTypeMenuDate, kFieldTypeMenuTime, kFieldTypeMenuPopup,
	kFieldTypeMenuInt, kFieldTypeMenuFloat, kFieldTypeMenuBool, kFieldTypeMenuCount};
#endif

static void CallFinishCreateSelfOp(const map<DialogFieldType,FieldTypeBoxHandler::Ptr>::value_type& v)
{
	v.second->FinishCreateSelf();
}

void
CEditDBDialog::SetupHandlers()
{
	AddHandler(kFieldTypeMenuString, new SimpleFieldTypeBoxHandler(this, FLDTYPE_STRING));
	AddHandler(kFieldTypeMenuCheckbox, new CheckboxFieldTypeBoxHandler(this));
	AddHandler(kFieldTypeMenuDate, new DateFieldTypeBoxHandler(this));
	AddHandler(kFieldTypeMenuTime, new TimeFieldTypeBoxHandler(this));
	AddHandler(kFieldTypeMenuPopup, new PopupFieldTypeBoxHandler(this));
	AddHandler(kFieldTypeMenuInt, new IntFieldTypeBoxHandler(this));
	AddHandler(kFieldTypeMenuFloat, new SimpleFieldTypeBoxHandler(this, FLDTYPE_FLOAT));
	AddHandler(kFieldTypeMenuBool, new SimpleFieldTypeBoxHandler(this, FLDTYPE_BOOLEAN));
#ifdef JFILE_CALCULATIONS
	AddHandler(kFieldTypeMenuCalc, new CalcFieldTypeBoxHandler(this));
#endif
	
	for_each(fHandlers.begin(), fHandlers.end(), CallFinishCreateSelfOp);
}

void
CEditDBDialog::SetupNetworkOnlyFeatures()
{
#ifdef NETWORK
	fUserNameField->Show();
	fUserNameField->AddListener(this);

	fExportButton->Show();
	fExportButton->AddListener(this);
#endif
}

void
CEditDBDialog::FinishCreateSelf()
{
	FindPanes();
	fView->SetLatentSub(fJFileName);
	SetupForDatabase();
	SetupListeners();
	SetupNetworkOnlyFeatures();
	SetupHandlers();

	fLastSelected = -1;
}

void 
CEditDBDialog::RememberPreSyncScript()
{
	int menuValue = fPreSyncScript->GetValue();
	FMAE::ScriptID preScriptID;
	if (menuValue > 2) {
		preScriptID = fScriptIDs[menuValue-3];
	}
	fDatabase->SetPreSyncScriptID(preScriptID);
}

void 
CEditDBDialog::RememberPostSyncScript()
{
	int menuValue = fPostSyncScript->GetValue();
	FMAE::ScriptID postScriptID;
	if (menuValue > 2) {
		postScriptID = fScriptIDs[menuValue-3];
	}
	fDatabase->SetPostSyncScriptID(postScriptID);

}

void 
CEditDBDialog::RememberUserNameField()
{
	int menuValue = fUserNameField->GetValue();
	FMAE::FieldID userNameField;
	if (menuValue > 2) {
		userNameField = fGlobalFieldIDs[menuValue-3];
	}
	fDatabase->SetUserNameField(userNameField);
}

bool
CEditDBDialog::DialogDone()
{
	SaveSelectedFieldInfo();
	
	//StCursor aCursor;
	if (!DoSetRemoteDatabaseName())
		return false;

	if (!RememberLayoutChanges())
		return false;

	RememberPreSyncScript();
	RememberPostSyncScript();
	RememberUserNameField();
	
	fDatabase->SetTranslateText(fTranslateText->GetValue());
	fDatabase->SetUseFoundSet(fUseFoundSet->GetValue());
	fDatabase->SetJFileVersion(JFileVersionFromMenu(fJFileVersionMenu));
	fDatabase->SetSyncMode(fSyncModeMenu->GetValue());
	fDatabase->SetDuplicateOnConflict(fDuplicateOnConflict->GetValue());
	fDatabase->SetSchedule(fSchedule->GetValue() - 1);
	fDatabase->SetOverrideSchedule(fOverrideSchedule);

	return true;	
}

void
CEditDBDialog::RegisterClasses()
{
	CConduitDialog::RegisterClasses();
}

void
CEditDBDialog::SetupWindowTitle()
{
	string wTitle(LoadString(kOtherStrings, kDatabaseOptionsIndex));
	wTitle += fDatabase->GetFileMakerDatabaseName().GetName();
	wTitle += 'Ó';
	SetWindowTitle(wTitle);
}

void
CEditDBDialog::OpenDatabaseByName()
{
	fFMDatabase.reset(new FileMaker::Database(fDatabase->GetFileMakerDatabaseName(), false));
}

void
CEditDBDialog::OpenDatabaseBySpec()
{
	FSSpec spec;
	fDatabase->GetFSSpec(&spec);
	fFMDatabase.reset(new FileMaker::Database(spec, false));
}

void
CEditDBDialog::OpenDatabase()
{
	if (fDatabase->GetByName())
		OpenDatabaseByName();
	else
		OpenDatabaseBySpec();
}

void
CEditDBDialog::PrepareDatabase()
{
	OpenDatabase();
	
	CFileMaker::Get(fDatabase->GetFileMakerDatabaseName().GetCreator()).CheckVersion();

	if (fFMDatabase->MajorVersion() < 4) {
		Warn(FMSWarnings::kFMP3OrderWarning);
		Warn(FMSWarnings::kFMP3MutliUserWarning);
	}

#ifndef NETWORK
	if (fFMDatabase->IsMultiUser()) {
		string m1(LoadString(kFMJErrorStrings, kMultiUserNoHotSyncIndex));
		LErrorMsg::AlertWithMessage(kAlertNoteAlert, m1);
	}
#endif
	
	fFMDatabase->EnterBrowseMode();
}

void
CEditDBDialog::PrepareLayouts()
{
	fLayoutNames = fFMDatabase->GetLayoutNames();
	if (fLayoutNames.size() == 0)
		return;
	
	fLayoutIDs = fFMDatabase->GetLayoutIDs();
	int layoutID = fDatabase->GetLayoutID();
	string s1, s2;
	if (layoutID == 0 || !CJFileSynchronizer::LayoutIsOK(fFMDatabase, layoutID, s1, s2)) {
		AnnounceStrings(s1, s2);
		 if (!DefaultLayoutID(&layoutID)) {
			string m1(LoadString(kFMJErrorStrings, kAllRelatedFieldsErrorIndex));
			string m2(LoadString(kFMJErrorStrings, kAllRelatedFieldsErrorIndex+1));
			LErrorMsg::AlertWithMessageAndDetails(kAlertNoteAlert, m1, m2);
			Throw_(userCanceledErr);
		}
	}
	AppendToPopupButton(fLayout, false, fLayoutNames);
	SetupOneMenu(fLayoutIDs, layoutID, 0, fLayout);
	UseLayout(layoutID);
}

void
CEditDBDialog::UnselectField()
{
	SaveSelectedFieldInfo();
	fFieldNamesList->SelectNone();
	fLastSelected = -1;
}

static void AddOther(LPopupButton* button)
{
	button->AppendMenu("\p--", false);
	button->AppendMenu("\pValue...", true);
}

void
CEditDBDialog::UseLayout(int layoutID)
{
	fFMDatabase->UseLayout(layoutID);
	fFieldIDs = fFMDatabase->GetFieldIDs();
	vector<int> fmReadOnly(fFMDatabase->GetFieldReadOnly());
	fFieldFMReadOnly.clear();
	int fieldCount = fFieldIDs.size();
	for (int i = 0; i < fieldCount; i+=1) {
		if (fmReadOnly[i])
			fFieldFMReadOnly[fFieldIDs[i]] = true;
	}
	UpdateFieldAttributes();
	vector<string> fieldNames(MakeOrderedList(fFieldAttributes.fFieldNames, fFieldIDs));
	fFieldNamesList->SetStrings(fieldNames);
	HandleSelectedFieldMessage();
}

void
CEditDBDialog::UpdateFieldAttributes()
{
	fOverrides = MakeOverrides();
	SetupFieldMaps();
}

void
CEditDBDialog::PrepareScripts()
{
	fScriptNames = fFMDatabase->GetScriptNames();
	fScriptIDs = fFMDatabase->GetScriptIDs();
	AppendToPopupButton(fPreSyncScript, true, fScriptNames);
	AppendToPopupButton(fPostSyncScript, true, fScriptNames);
	FMAE::ScriptID scriptID = fDatabase->GetPreSyncScriptID();
	if (!CJFileSynchronizer::ScriptIsOK(fFMDatabase, scriptID)) {
		string m1(LoadString(kFMJErrorStrings, kPreSyncNLEErrorIndex));
		LErrorMsg::AlertWithMessage(kAlertNoteAlert, m1);
		scriptID = FMAE::ScriptID();
	}
	SetupOneMenu(fScriptIDs, scriptID, 2, fPreSyncScript);
	scriptID = fDatabase->GetPostSyncScriptID();
	if (!CJFileSynchronizer::ScriptIsOK(fFMDatabase, scriptID)) {
		string m1(LoadString(kFMJErrorStrings, kPostSyncNLEErrorIndex));
		LErrorMsg::AlertWithMessage(kAlertNoteAlert, m1);
		scriptID = FMAE::ScriptID();
	}
	SetupOneMenu(fScriptIDs, scriptID, 2, fPostSyncScript);
}

void
CEditDBDialog::PrepareOptions()
{
	fRemoteDatabaseName = fDatabase->GetPilotDatabaseName();
	fJFileName->SetDescriptor(AsStr255(fRemoteDatabaseName));
	
	fSyncModeMenu->SetValue(fDatabase->GetSyncMode());
	fJFileVersionMenu->SetValue(MenuFromJFileVersion(fDatabase->GetJFileVersion()));
	SetupSyncDescription();
	
	fTranslateText->SetValue(fDatabase->GetTranslateText());
	fUseFoundSet->SetValue(fDatabase->GetUseFoundSet());
	fDuplicateOnConflict->SetValue(fDatabase->GetDuplicateOnConflict());
	
	PrepareUserNameFieldMenu();
}

void
CEditDBDialog::PrepareUserNameFieldMenu()
{
#if 0
	fGlobalFieldIDs = fFMDatabase->GetGlobalFieldIDs();
	AppendToPopupButton(fUserNameField, false, fFMDatabase->GetGlobalFieldNames());
	FMAE::FieldID userNameField(fDatabase->GetUserNameField());
	if (userNameField.IsValid()) {
		CFieldIDList::const_iterator f = find(fGlobalFieldIDs.begin(), fGlobalFieldIDs.end(), userNameField);
		if (f != fGlobalFieldIDs.end()) {
			CFieldIDList::size_type index = f - fGlobalFieldIDs.begin();
			fUserNameField->SetValue(index + 3);
		} else {
			static string userNameFieldMissing(LoadString(kFMJErrorStrings, kUserNameFieldMissingDlg));
			static string userNameFieldMissing2(LoadString(kFMJErrorStrings, kUserNameFieldMissingDlg2));
			LErrorMsg::AlertWithMessageAndDetails(kAlertStopAlert, userNameFieldMissing, userNameFieldMissing2);
		}
	}
#endif
}

void
CEditDBDialog::PrepareAdvanced()
{
	fSchedule->SetValue(fDatabase->GetSchedule() + 1);
	fOverrideSchedule = fDatabase->GetOverrideSchedule();
}

void
CEditDBDialog::SetupSyncDescription()
{
	fSyncModeDescription->SetDescriptor(
		AsStr255(LoadString(kOtherStrings, kTwoWayDescriptionIndex + fSyncModeMenu->GetValue() - 1)));
}

void
CEditDBDialog::SetupForDatabase()
{
	CJFileSynchronizer::SetupDefaults(fDatabase->GetDefaultToDate(), fDatabase->GetDefaultToInt());

	CFrontProcess fp;

	SetupWindowTitle();

	PrepareDatabase();
	PrepareLayouts();
	PrepareScripts();
	PrepareOptions();
	PrepareAdvanced();
	
	fOverrides = fDatabase->FieldOverrides();

	SetupFieldMaps();

	fFieldNamesList->SetStrings(MakeOrderedList(fFieldAttributes.fFieldNames, fFieldIDs));
}

FMAE::FieldID
CEditDBDialog::GetLastSelectedField()
{
	return fFieldIDs[fLastSelected];
}

void
CEditDBDialog::SaveSelectedExtraData()
{
	FieldTypeBoxHandler::Ptr handler(GetHandler(fLastFieldTypeMenuValue));
	FMAE::FieldID field(GetLastSelectedField());
	fFieldAttributes.fFieldExtra[field] = handler->GetExtra();
	fFieldAttributes.fFieldExtra2[field] = handler->GetExtra2();
	fFieldAttributes.fFieldValue1[field] = handler->GetValue1();
	fFieldAttributes.fFieldValue2[field] = handler->GetValue2();
}

void
CEditDBDialog::SaveSelectedFieldInfo()
{
	if (fLastSelected >= 0) {
		FMAE::FieldID field(GetLastSelectedField());
		fFieldAttributes.fFieldWidths[field] = fWidthEdit->GetValue();
		fFieldAttributes.fFieldTypes[field] = GetHandler(fLastFieldTypeMenuValue)->GetJFileFieldType();
		if (!fFieldFMReadOnly[field])
			fFieldAttributes.fFieldReadOnly[field] = fReadOnly->GetValue();
		Str255 fieldName;
		fFieldNameEdit->GetDescriptor(fieldName);
		fFieldAttributes.fFieldNames[field] = AsString(fieldName);
		SaveSelectedExtraData();
	}
}

void
CEditDBDialog::UpdateFieldDisplay(FMAE::FieldID field)
{
	fWidthEdit->SetValue(fFieldAttributes.fFieldWidths[field]);
	SetupFieldInfoBox(fFieldAttributes.fFieldTypes[field]);
	if (fFieldFMReadOnly[field]) {
		fReadOnly->SetValue(true);
		fReadOnly->Disable();
	}
	else {
		fReadOnly->SetValue(fFieldAttributes.fFieldReadOnly[field]);
		fReadOnly->Enable();
	}
	fFieldNameEdit->SetDescriptor(AsStr255(fFieldAttributes.fFieldNames[field]));
}

void CEditDBDialog::SetupJFileFieldTypeMap()
{
	fJFileFieldTypeMap[FLDTYPE_STRING] = DialogFieldType(kFieldTypeMenuString);
	fJFileFieldTypeMap[FLDTYPE_BOOLEAN] = DialogFieldType(kFieldTypeMenuBool);
#ifdef JFILE_CALCULATIONS
	fJFileFieldTypeMap[FLDTYPE_CALC] = DialogFieldType(kFieldTypeMenuCalc);
	fJFileFieldTypeMap[FLDTYPE_CALC_V1] = DialogFieldType(kFieldTypeMenuCalc);
	fJFileFieldTypeMap[FLDTYPE_CALC_V2] = DialogFieldType(kFieldTypeMenuCalc);
#endif
	fJFileFieldTypeMap[FLDTYPE_FMSYNC_CHECKBOX] = DialogFieldType(kFieldTypeMenuCheckbox);
	fJFileFieldTypeMap[FLDTYPE_LIST] = DialogFieldType(kFieldTypeMenuPopup);
	fJFileFieldTypeMap[FLDTYPE_MULTLIST] = DialogFieldType(kFieldTypeMenuPopup);
	fJFileFieldTypeMap[FLDTYPE_AUTOINC] = DialogFieldType(kFieldTypeMenuInt);
	fJFileFieldTypeMap[FLDTYPE_INT] = DialogFieldType(kFieldTypeMenuInt);
	fJFileFieldTypeMap[FLDTYPE_FLOAT] = DialogFieldType(kFieldTypeMenuFloat);
	fJFileFieldTypeMap[FLDTYPE_DATE] = DialogFieldType(kFieldTypeMenuDate);
	fJFileFieldTypeMap[FLDTYPE_AUTODATE] = DialogFieldType(kFieldTypeMenuDate);
	fJFileFieldTypeMap[FLDTYPE_MODDATE] = DialogFieldType(kFieldTypeMenuDate);
	fJFileFieldTypeMap[FLDTYPE_TIME] = DialogFieldType(kFieldTypeMenuTime);
	fJFileFieldTypeMap[FLDTYPE_AUTOTIME] = DialogFieldType(kFieldTypeMenuTime);
	fJFileFieldTypeMap[FLDTYPE_MODTIME] = DialogFieldType(kFieldTypeMenuTime);
}

void CEditDBDialog::HandleSelectedField(int index)
{
	SaveSelectedFieldInfo();
	fLastSelected = index;
	FMAE::FieldID field(GetLastSelectedField());
	fWidthEdit->Enable();
	fFieldTypeBox->Enable();
	fLastFieldTypeMenuValue = fJFileFieldTypeMap[fFieldAttributes.fFieldTypes[field]];
	SetDialogFieldType(fLastFieldTypeMenuValue);
	fDefaults->Enable();
	fFieldNameEdit->Enable();
	UpdateFieldDisplay(field);
	LCommander::SwitchTarget(fFieldNameEdit);
}

void CEditDBDialog::HandleNoSelectedField()
{
	fLastSelected = -1;
	fWidthEdit->SetDescriptor(Str_Empty);
	fFieldNameEdit->SetDescriptor(Str_Empty);
	fFieldTypeBox->SetValue(0);
	fReadOnly->SetValue(0);
	fWidthEdit->Disable();
	fFieldTypeBox->Disable();
	fReadOnly->Disable();
	fFieldNameEdit->Disable();
	fDefaults->Disable();
}

void CEditDBDialog::HandleSelectedFieldMessage()
{
	int index;
	if (fFieldNamesList->SelectedIndex(index)) {
		HandleSelectedField(index);
	} else {
		HandleNoSelectedField();
	}
}

void CEditDBDialog::HandleFieldTypeChangedMessage()
{
	if (fHandlingFieldTypeChange)
		return;
	
	StValueChanger<bool> vc(fHandlingFieldTypeChange, true);
	int index;
	if (fFieldNamesList->SelectedIndex(index)) {
		FMAE::FieldID field(fFieldIDs.at(index));
		fLastFieldTypeMenuValue = GetDialogFieldType();
		fFieldAttributes.fFieldTypes[field] = GetHandler(fLastFieldTypeMenuValue)->GetJFileFieldType();
		fWidthEdit->Enable();
		fFieldTypeBox->Enable();
		fDefaults->Enable();
		fFieldNameEdit->Enable();
		UpdateFieldDisplay(field);
		LCommander::SwitchTarget(fFieldNameEdit);
	}
}

template <class T> map<T,int>
MapMap(const map<int, int>& theMap, const map<T,int>& targetMap)
{
	map<T,int> theResult;
	map<T,int>::const_iterator i = targetMap.begin();
	while (i != targetMap.end()) {
		map<int, int>::const_iterator f = theMap.find(i->second);
		theResult[i->first] = f->second;
		++i;
	}
	return theResult;
}

void
CEditDBDialog::SetupDefaultFieldMaps()
{
	fAllRecordIDs = fFMDatabase->GetAllRecordIDs();
	CFMFieldTyper typer(fFMDatabase);
	CDatabaseInfo info(typer.GetDatabaseInfo());

	fDefaultFieldAttributes.fFieldNames = info.fFieldNames;
	fDefaultFieldAttributes.fFieldPopups = info.fPopupValues;
	fDefaultFieldAttributes.fFieldWidths = info.fColumnWidths;
	fDefaultFieldAttributes.fFieldTypes = info.fFieldTypes;
	fDefaultFieldAttributes.fFieldReadOnly = info.fFieldAccess;
	fDefaultFieldAttributes.fFieldExtra = info.fFieldExtraData;
	fDefaultFieldAttributes.fFieldExtra2 = info.fFieldExtraData2;
	fDefaultFieldAttributes.fFieldValue1 = info.fFieldCalcValue1;
	fDefaultFieldAttributes.fFieldValue2 = info.fFieldCalcValue2;
}

void
CEditDBDialog::SetupFieldMaps()
{
	SetupDefaultFieldMaps();
	fFieldAttributes = fDefaultFieldAttributes;
	ApplyDiffMap(fFieldAttributes.fFieldNames, fOverrides.fName);
	ApplyDiffMap(fFieldAttributes.fFieldPopups, fOverrides.fPopups);
	ApplyDiffMap(fFieldAttributes.fFieldTypes, fOverrides.fType);
	ApplyDiffMap(fFieldAttributes.fFieldWidths, fOverrides.fWidth);
	ApplyDiffMap(fFieldAttributes.fFieldReadOnly, fOverrides.fReadOnly);
	ApplyDiffMap(fFieldAttributes.fFieldExtra, fOverrides.fExtra);
	ApplyDiffMap(fFieldAttributes.fFieldExtra2, fOverrides.fExtra2);
	ApplyDiffMap(fFieldAttributes.fFieldValue1, fOverrides.fFieldCalcValue1);
	ApplyDiffMap(fFieldAttributes.fFieldValue2, fOverrides.fFieldCalcValue2);
}

void
CEditDBDialog::SetupFieldInfoBox(int fieldType)
{
	GetHandler(GetDialogFieldType())->SetupForField(fieldType);
}

int	CEditDBDialog::JFileVersionFromMenu(LPopupButton* b)
{
	if (b->GetValue() == 1)
		return CDatabaseFile::JFilePro;
	
	return CDatabaseFile::JFile5;
}

int	CEditDBDialog::MenuFromJFileVersion(int version)
{
	if (version == CDatabaseFile::JFilePro)
		return 1;
	
	return 2;
}

FieldTypeBoxHandler::Ptr CEditDBDialog::GetHandler(DialogFieldType fieldTypeMenuValue)
{
	map<DialogFieldType,FieldTypeBoxHandler::Ptr>::iterator f = fHandlers.find(fieldTypeMenuValue);
	ThrowIf_(f == fHandlers.end());
	return f->second;
}

void CEditDBDialog::AddHandler(int i, FieldTypeBoxHandler* p)
{
	fHandlers[DialogFieldType(i)] = FieldTypeBoxHandler::Ptr(p);
}

DialogFieldType CEditDBDialog::GetDialogFieldType()
{
	return DialogFieldType(fFieldTypeBox->GetValue());
}

void CEditDBDialog::SetDialogFieldType(const DialogFieldType& f)
{
	DisableMessage<LPopupGroupBox> dm(fFieldTypeBox);
	fFieldTypeBox->SetValue(f.GetValue());
	fFieldTypeMPV->SwitchToPanel(f.GetValue());
}


namespace DebugOutput {

void DoOutput(const CFieldAttributes& inAttr)
{
	Output("fFieldNames", inAttr.fFieldNames);
	Output("fFieldPopups", inAttr.fFieldPopups);
	Output("fFieldTypes", inAttr.fFieldTypes);
	Output("fFieldWidths", inAttr.fFieldWidths);
	Output("fFieldReadOnly", inAttr.fFieldReadOnly);
	Output("fFieldExtra", inAttr.fFieldExtra);
	Output("fFieldExtra2", inAttr.fFieldExtra2);
	Output("fFieldValue1", inAttr.fFieldValue1);
	Output("fFieldValue2", inAttr.fFieldValue2);
}

}
