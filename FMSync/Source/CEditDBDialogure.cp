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

#include "FMDatabase.h"

using namespace JFile5;

const UInt32 kCalcValue = -1;

enum {
	kFieldTypeMenuString = 1, kFieldTypeMenuCheckbox, 
	kFieldTypeMenuDate, kFieldTypeMenuTime, kFieldTypeMenuPopup,
	kFieldTypeMenuInt, kFieldTypeMenuFloat, kFieldTypeMenuBool, 
	kFieldTypeMenuCalc,
	kFieldTypeMenuAutoDate, kFieldTypeMenuAutoTime, kFieldTypeMenuAutoInc,
	kFieldTypeMenuModDate, kFieldTypeMenuModTime,
	kFieldTypeMenuCount};
	
static int gFieldTypeTable[kFieldTypeMenuCount] =
{
	FLDTYPE_STRING,
	FLDTYPE_FMSYNC_CHECKBOX,
	FLDTYPE_AUTODATE,
	FLDTYPE_AUTOTIME,
	FLDTYPE_LIST,
	FLDTYPE_INT,
	FLDTYPE_FLOAT,
	FLDTYPE_BOOLEAN,
	FLDTYPE_CALC,
	FLDTYPE_AUTODATE,
	FLDTYPE_AUTOTIME,
	FLDTYPE_AUTOINC,
	FLDTYPE_MODDATE,
	FLDTYPE_MODTIME
};

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
	: CConduitDialog(kWINDDatabaseSettings), CConduitWarner(w), fWarnings(w), fDatabase(inDB), fFMDatabase(0)
{
	for(int i = 0; i < kFieldTypeMenuCount; i+= 1) {
		fJFileToMenu[gFieldTypeTable[i]] = i+1;
		fMenuToJFile[i+1] = gFieldTypeTable[i];
	}
}

CEditDBDialog::~CEditDBDialog()
{
}

void
CEditDBDialog::UpdateControlState()
{
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

void
CEditDBDialog::DoEditPopupValues()
{
	if (fLastSelected != -1) {
		FMAE::FieldID fieldID(GetLastSelectedField());
		CEditPopupDialog editPD(fFieldAttributes.fFieldPopups[fieldID]);
		if (!editPD.DoDialog()) {
			fFieldAttributes.fFieldPopups[fieldID] = editPD.GetValues();
		}
	}
}

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
		DebugOutput::Output("fFieldAttributes: ", fFieldAttributes);
		DebugOutput::Output("fDefaultFieldAttributes: ", fDefaultFieldAttributes);
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
	newOverrides.fFieldCalcValue1 = MakeDiffMap(fDefaultFieldAttributes.fFieldValue2, fFieldAttributes.fFieldValue2);
	
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
		HandleSelectedField();
		break;
	case 'popv':
		DoEditPopupValues();
		break;
	case 'auto':
		EnableAutoIncrementEdit(true);
		break;
	case 'nori':
		EnableAutoIncrementEdit(false);
		break;
	case 'defa':
		SetToDefaults();
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
CEditDBDialog::FinishCreateSelf()
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
	FindPane(fView, 'popv', &fEditPopups);
	FindPane(fView, 'defa', &fDefaults);
	FindPane(fView, 'dsyn', &fSyncModeDescription);
	FindPane(fView, 'UNam', &fUserNameField);

	FindPane(fView, 'crea', &fCreateDateRadio);
	FindPane(fView, 'modi', &fModDateRadio);
	FindPane(fView, 'norm', &fNormalDateRadio);
	
	FindPane(fView, 'cret', &fCreateTimeRadio);
	FindPane(fView, 'modt', &fModTimeRadio);
	FindPane(fView, 'nort', &fNormalTimeRadio);
	
	FindPane(fView, 'auto', &fAutoIncRadio);
	fAutoIncRadio->AddListener(this);
	FindPane(fView, 'nori', &fNormalIntRadio);
	fNormalIntRadio->AddListener(this);

	FindPane(fView, 'npop', &fNormalListRadio);
	FindPane(fView, 'mpop', &fMultiListRadio);

	FindPane(fView, 'star', &fIncStartEdit);
	FindPane(fView, 'incr', &fIncValueEdit);
	FindPane(fView, 'autv', &fAutoIncView);

	FindPane(fView, 'fie1', &fField1Menu);
	FindPane(fView, 'fie2', &fField2Menu);
	FindPane(fView, 'oper', &fCalcOperationMenu);


	fView->SetLatentSub(fJFileName);
	SetupForDatabase();
	fFieldNamesList->AddListener(this);

	fEditPopups->AddListener(this);
	fDefaults->AddListener(this);
	fFieldTypeBox->AddListener(this);
	fLayout->AddListener(this);
	fSyncModeMenu->AddListener(this);

#ifdef NETWORK
	fUserNameField->Show();
	fUserNameField->AddListener(this);

	fExportButton->Show();
	fExportButton->AddListener(this);
#endif

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
	
	StCursor aCursor;
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
	SetPopupButton(fField1Menu, fieldNames);
	AddOther(fField1Menu);
	SetPopupButton(fField2Menu, fieldNames);
	AddOther(fField2Menu);
	fFieldNamesList->SetStrings(fieldNames);
	HandleSelectedField();
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
	
	fOverrides = fDatabase->FieldOverrides();

	SetupFieldMaps();

	fFieldNamesList->SetStrings(MakeOrderedList(fFieldAttributes.fFieldNames, fFieldIDs));
}

FMAE::FieldID
CEditDBDialog::GetLastSelectedField()
{
	return fFieldIDs[fLastSelected];
}

int CEditDBDialog::JFileFieldIndexFromMenu(LPopupButton* pb)
{
	UInt32 v = pb->GetValue();
	if (v <= fFieldIDs.size())
		return v - 1;
	return kCalcValue;
}

string CEditDBDialog::JFileFieldValueFromMenu(LPopupButton* pb)
{
	UInt32 v = pb->GetValue();
	if (v <= fFieldIDs.size())
		return "";
	
	Str255 menuText;
	pb->GetMenuItemText(v, menuText);
	return AsString(menuText);
}

void
CEditDBDialog::SaveExtraDataForCalculatedFields(int fieldType)
{
	UInt32 f1 = JFileFieldIndexFromMenu(fField1Menu);
	UInt32 f2 = JFileFieldIndexFromMenu(fField2Menu);
	ThrowIf_(f1 == kCalcValue && f2 == kCalcValue);
	FMAE::FieldID field(GetLastSelectedField());
	UInt32 oper = fCalcOperationMenu->GetValue();
	fFieldAttributes.fFieldExtra[field] = CWriteJFile5::ExtraDataForCalculatedFields(f1, f2, oper);
	fFieldAttributes.fFieldExtra2[field] = 0;
	fFieldAttributes.fFieldValue1[field] = JFileFieldValueFromMenu(fField1Menu);
	fFieldAttributes.fFieldValue2[field] = JFileFieldValueFromMenu(fField2Menu);
}

void
CEditDBDialog::SaveSelectedExtraData(int fieldType)
{
	FMAE::FieldID field(GetLastSelectedField());
	if(fieldType == FLDTYPE_MULTLIST) {
		fFieldAttributes.fFieldExtra[field] = 2;
		fFieldAttributes.fFieldExtra2[field] = 0;
	} else if (fieldType == FLDTYPE_AUTOINC) {
		fFieldAttributes.fFieldExtra[field] = fIncStartEdit->GetValue();
		fFieldAttributes.fFieldExtra2[field] = fIncValueEdit->GetValue();
	} else if (fieldType >= FLDTYPE_CALC && fieldType <= FLDTYPE_CALC_V2) {
		SaveExtraDataForCalculatedFields(fieldType);
	} else {
		fFieldAttributes.fFieldExtra[field] = 0;
		fFieldAttributes.fFieldExtra2[field] = 0;
	}
}

void
CEditDBDialog::SaveSelectedFieldInfo()
{
	if (fLastSelected >= 0) {
		FMAE::FieldID field(GetLastSelectedField());
		fFieldAttributes.fFieldWidths[field] = fWidthEdit->GetValue();
		int fieldType = ExtractFieldType(fFieldTypeBox->GetValue());
		fFieldAttributes.fFieldTypes[field] = fieldType;
		if (!fFieldFMReadOnly[field])
			fFieldAttributes.fFieldReadOnly[field] = fReadOnly->GetValue();
		Str255 fieldName;
		fFieldNameEdit->GetDescriptor(fieldName);
		fFieldAttributes.fFieldNames[field] = AsString(fieldName);
		SaveSelectedExtraData(fieldType);
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

void
CEditDBDialog::HandleSelectedField()
{
	int index;
	if (fFieldNamesList->SelectedIndex(index)) {
		SaveSelectedFieldInfo();
		fLastSelected = index;
		FMAE::FieldID field(GetLastSelectedField());
		fWidthEdit->Enable();
		fFieldTypeBox->Enable();
		fDefaults->Enable();
		fFieldNameEdit->Enable();
		UpdateFieldDisplay(field);
		LCommander::SwitchTarget(fFieldNameEdit);
	} else {
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

map<FMAE::FieldID,int>
CEditDBDialog::MenuToJFile(const map<FMAE::FieldID,int>&theMap)
{
	return MapMap(fMenuToJFile, theMap);
}

map<FMAE::FieldID,int>
CEditDBDialog::JFileToMenu(const map<FMAE::FieldID,int>&theMap)
{
	return MapMap(fJFileToMenu, theMap);
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
	FMAE::FieldID fieldID(GetLastSelectedField());
	switch(fieldType) {
	case FLDTYPE_STRING:
		fFieldTypeBox->SetValue(kFieldTypeMenuString);
		break;

	case FLDTYPE_BOOLEAN:
		fFieldTypeBox->SetValue(kFieldTypeMenuBool);
		break;

	case FLDTYPE_CALC:
	case FLDTYPE_CALC_V1:
	case FLDTYPE_CALC_V2:
		fFieldTypeBox->SetValue(kFieldTypeMenuCalc);
		SetupCalculatedField();
		break;

	case FLDTYPE_FMSYNC_CHECKBOX:
		fFieldTypeBox->SetValue(kFieldTypeMenuCheckbox);
		break;

	case FLDTYPE_LIST:
	case FLDTYPE_MULTLIST:
		fFieldTypeBox->SetValue(kFieldTypeMenuPopup);
		fNormalListRadio->SetValue(fieldType == FLDTYPE_LIST);
		fMultiListRadio->SetValue(fieldType == FLDTYPE_MULTLIST);		
		break;

	case FLDTYPE_AUTOINC:
	case FLDTYPE_INT:
		fAutoIncRadio->SetValue(fieldType == FLDTYPE_AUTOINC);
		fNormalIntRadio->SetValue(fieldType == FLDTYPE_INT);		
		EnableAutoIncrementEdit(fieldType == FLDTYPE_AUTOINC);
		fFieldTypeBox->SetValue(kFieldTypeMenuInt);
		break;
		
	case FLDTYPE_FLOAT:
		fFieldTypeBox->SetValue(kFieldTypeMenuFloat);
		break;

	case FLDTYPE_DATE:
	case FLDTYPE_AUTODATE:
	case FLDTYPE_MODDATE:
		fFieldTypeBox->SetValue(kFieldTypeMenuDate);
		break;

	case FLDTYPE_TIME:
	case FLDTYPE_AUTOTIME:
	case FLDTYPE_MODTIME:
		fFieldTypeBox->SetValue(kFieldTypeMenuTime);
	}
}

int
CEditDBDialog::CalculatedFieldType()
{
	if (JFileFieldIndexFromMenu(fField1Menu) == kCalcValue)
		return FLDTYPE_CALC_V1;
	
	if (JFileFieldIndexFromMenu(fField2Menu) == kCalcValue)
		return FLDTYPE_CALC_V2;
	
	return FLDTYPE_CALC;
}

int
CEditDBDialog::ExtractFieldType(int menuValue)
{
	switch(menuValue) {
	case kFieldTypeMenuBool:
		return FLDTYPE_BOOLEAN;
	case kFieldTypeMenuCalc:
		return CalculatedFieldType();
	case kFieldTypeMenuCheckbox:
		return FLDTYPE_FMSYNC_CHECKBOX;
	case kFieldTypeMenuDate:
		if (fModDateRadio->GetValue())
			return FLDTYPE_MODDATE;
		if (fCreateDateRadio->GetValue())
			return FLDTYPE_AUTODATE;
		return FLDTYPE_DATE;
	case kFieldTypeMenuTime:
		if (fModTimeRadio->GetValue())
			return FLDTYPE_MODTIME;
		if (fCreateTimeRadio->GetValue())
			return FLDTYPE_AUTOTIME;
		return FLDTYPE_TIME;
	case kFieldTypeMenuPopup:
		if (fMultiListRadio->GetValue())
			return FLDTYPE_MULTLIST;
		return FLDTYPE_LIST;
	case kFieldTypeMenuInt:
		if (fAutoIncRadio->GetValue())
			return FLDTYPE_AUTOINC;
		return FLDTYPE_INT;
	case kFieldTypeMenuFloat:
		return FLDTYPE_FLOAT;
	}
	return FLDTYPE_STRING;
}

void
CEditDBDialog::EnableAutoIncrementEdit(bool enable)
{
	FMAE::FieldID fieldID(GetLastSelectedField());

	if (enable) {
		fAutoIncView->Enable();
		fIncStartEdit->SetValue(fFieldAttributes.fFieldExtra[fieldID]);
		fIncValueEdit->SetValue(fFieldAttributes.fFieldExtra2[fieldID]);
		LCommander::SwitchTarget(fIncStartEdit);
	} else {
		fAutoIncView->Disable();
		fIncStartEdit->SetValue(0);
		fIncValueEdit->SetValue(0);
	}
}

inline UInt32 GetCalcField1(UInt32 extraValue)
{
	return extraValue >> 24;
}

inline UInt32 GetCalcField2(UInt32 extraValue)
{
	return (extraValue >> 16) & 0xff;
}

inline UInt32 GetCalcOperation(UInt32 extraValue)
{
	return extraValue & 0x0000ffff;
}

static int JFileToFieldMenuValue(int fieldType, int specialCase, int fieldNumber, int fieldCount)
{
	if (fieldType == specialCase)
		return fieldCount + 1;
	return fieldNumber; 
}

void
CEditDBDialog::SetupCalculatedField()
{
	FMAE::FieldID fieldID(GetLastSelectedField());
	UInt32 extra(fFieldAttributes.fFieldExtra[fieldID]);
	UInt32 fieldType(fFieldAttributes.fFieldTypes[fieldID]);
	fField1Menu->SetValue(JFileToFieldMenuValue(fieldType, FLDTYPE_CALC_V1, GetCalcField1(extra), fFieldIDs.size()));
	fField2Menu->SetValue(JFileToFieldMenuValue(fieldType, FLDTYPE_CALC_V2, GetCalcField2(extra), fFieldIDs.size()));
	fCalcOperationMenu->SetValue(GetCalcOperation(extra));
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
