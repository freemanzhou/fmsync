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
#include "PaneUtilities.h"
#include "FolderUtilities.h"
#include "Str255.h"
#include "Stringiness.h"
#include "syncmgr.h"
#include "CFileMakerDatabase.h"
#include "CJFileSynchronizer.h"
#include "FMCResources.h"
#include "JFilePro.h"
#include "DebugOutput.h"
#include "CFieldOverrides.h"
#include "CJFileSynchronizer.h"
#include "CFrontProcess.h"
#include "FMSWarnings.h"
#include "CEditPopupDialog.h"
#include "CFMFieldTyper.h"
#include "ErrorCodes.h"

enum {
	kFieldTypeMenuString = 1, kFieldTypeMenuCheckbox, 
	kFieldTypeMenuDate, kFieldTypeMenuTime, kFieldTypeMenuPopup,
	kFieldTypeMenuInt, kFieldTypeMenuFloat, kFieldTypeMenuBool,
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

CEditDBDialog::CEditDBDialog(CDatabaseFile::Ptr inDB, CWarnings& w)
	: CConduitDialog(kWINDDatabaseSettings), CConduitWarner(w), fWarnings(w), fDatabase(inDB), fFMDatabase(0)
{
	for(int i = 0; i < kFieldTypeMenuCount; i+= 1) {
		fJFileToMenu[gFieldTypeTable[i]] = i+1;
		fMenuToJFile[i+1] = gFieldTypeTable[i];
	}
}

CEditDBDialog::~CEditDBDialog()
{
	delete fFMDatabase;
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
#if 0
	else {
		if (!NameIsOK(remoteName)) {
			string m1(LoadString(kFMJErrorStrings, kNameInUseErrorIndex));
			string m2(LoadString(kFMJErrorStrings, kNameInUseErrorIndex+1));
			Substitute(m2, "%%1", remoteName);
			LErrorMsg::AlertWithMessageAndDetails(kAlertNoteAlert, m1, m2);
			setSucceeded = false;
		}
	}
#endif
	
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
		FMAE::FieldID fieldID(GetLastSelectedField());
		fFieldAttributes.fFieldNames[fieldID] = fDefaultFieldAttributes.fFieldNames[fieldID];
		fFieldAttributes.fFieldPopups[fieldID] = fDefaultFieldAttributes.fFieldPopups[fieldID];
		fFieldAttributes.fFieldTypes[fieldID] = fDefaultFieldAttributes.fFieldTypes[fieldID];
		fFieldAttributes.fFieldWidths[fieldID] = fDefaultFieldAttributes.fFieldWidths[fieldID];
		fFieldAttributes.fFieldReadOnly[fieldID] = fDefaultFieldAttributes.fFieldReadOnly[fieldID];
		fFieldAttributes.fFieldExtra[fieldID] = fDefaultFieldAttributes.fFieldExtra[fieldID];
		fFieldAttributes.fFieldExtra2[fieldID] = fDefaultFieldAttributes.fFieldExtra2[fieldID];
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
			fDatabase->RememberFieldIDs(fFieldIDs);
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
	
	return newOverrides;
}

void
CEditDBDialog::ListenToMessage(MessageT inMessage, void* param)
{
	switch(inMessage) {
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
	FindPane(fView, 'dupl', &fDuplicateOnConflict);
	FindPane(fView, 'popv', &fEditPopups);
	FindPane(fView, 'chkv', &fEditPopupsChk);
	FindPane(fView, 'defa', &fDefaults);
	FindPane(fView, 'dsyn', &fSyncModeDescription);

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

	fView->SetLatentSub(fJFileName);
	SetupForDatabase();
	fFieldNamesList->AddListener(this);

	fEditPopups->AddListener(this);
	fEditPopupsChk->AddListener(this);
	fDefaults->AddListener(this);
	fFieldTypeBox->AddListener(this);
	fLayout->AddListener(this);
	fSyncModeMenu->AddListener(this);

	fLastSelected = -1;

#ifdef DEMO
	if (fFMDatabase->AllRecordsCount() > DEMO_RECORD_LIMIT)
		Throw_(kDemoLimitExceeded);
#endif
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

	int menuValue = fPreSyncScript->GetValue();
	FMAE::ScriptID preScriptID;
	if (menuValue > 2) {
		preScriptID = fScriptIDs[menuValue-3];
	}
	fDatabase->SetPreSyncScriptID(preScriptID);

	menuValue = fPostSyncScript->GetValue();
	FMAE::ScriptID postScriptID;
	if (menuValue > 2) {
		postScriptID = fScriptIDs[menuValue-3];
	}
	fDatabase->SetPostSyncScriptID(postScriptID);

	fDatabase->SetTranslateText(fTranslateText->GetValue());
	fDatabase->SetUseFoundSet(fUseFoundSet->GetValue());
	fDatabase->SetJFileVersion(CDatabaseFile::JFile5);
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
	wTitle += fDatabase->DescribeDatabase();
	wTitle += 'Ó';
	SetWindowTitle(wTitle);
}

void
CEditDBDialog::PrepareDatabase()
{
	FSSpec spec;
	fDatabase->GetFSSpec(&spec);
	try {
		fFMDatabase = new CFileMakerDatabase(spec, false);
	} catch (LException err) {
		AnnounceFailToOpenDatabase(spec, err.GetErrorCode());
		Throw_(kSyncQuietAbortErr);
	} catch(...) {
		AnnounceFailToOpenDatabase(spec, kUnknownError);
		throw;
	}

	if (fFMDatabase->MajorVersion() < 4) {
		Warn(FMSWarnings::kFMP3OrderWarning);
		Warn(FMSWarnings::kFMP3MutliUserWarning);
	}

	if (fFMDatabase->IsMultiUser()) {
		string m1(LoadString(kFMJErrorStrings, kMultiUserNoHotSyncIndex));
		LErrorMsg::AlertWithMessage(kAlertNoteAlert, m1);
	}
	
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
	fFieldNamesList->SetStrings(MakeOrderedList(fFieldAttributes.fFieldNames, fFieldIDs));
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
	SetupSyncDescription();
	
	fTranslateText->SetValue(fDatabase->GetTranslateText());
	fUseFoundSet->SetValue(fDatabase->GetUseFoundSet());
	fDuplicateOnConflict->SetValue(fDatabase->GetDuplicateOnConflict());
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

#if 0
bool
CEditDBDialog::NameIsOK(const string& remoteName)
{
	return fNamer.NameIsOK(remoteName);
}

string
CEditDBDialog::MakeValidName(const string& remoteName)
{
	return fNamer.MakeValidName(remoteName);
}
#endif

FMAE::FieldID
CEditDBDialog::GetLastSelectedField()
{
	return fFieldIDs[fLastSelected];
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
	CFMFieldTyper typer(*fFMDatabase);
	CDatabaseInfo info(typer.GetDatabaseInfo());

	fDefaultFieldAttributes.fFieldNames = info.fFieldNames;
	fDefaultFieldAttributes.fFieldPopups = info.fPopupValues;
	fDefaultFieldAttributes.fFieldWidths = info.fColumnWidths;
	fDefaultFieldAttributes.fFieldTypes = info.fFieldTypes;
	fDefaultFieldAttributes.fFieldReadOnly = info.fFieldAccess;
	fDefaultFieldAttributes.fFieldExtra = info.fFieldExtraData;
	fDefaultFieldAttributes.fFieldExtra2 = info.fFieldExtraData2;
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
		fCreateDateRadio->SetValue(fieldType == FLDTYPE_AUTODATE);
		fModDateRadio->SetValue(fieldType == FLDTYPE_MODDATE);
		fNormalDateRadio->SetValue(fieldType == FLDTYPE_DATE);
		break;

	case FLDTYPE_TIME:
	case FLDTYPE_AUTOTIME:
	case FLDTYPE_MODTIME:
		fFieldTypeBox->SetValue(kFieldTypeMenuTime);
		fCreateTimeRadio->SetValue(fieldType == FLDTYPE_AUTOTIME);
		fModTimeRadio->SetValue(fieldType == FLDTYPE_MODTIME);
		fNormalTimeRadio->SetValue(fieldType == FLDTYPE_TIME);
	}
}

int
CEditDBDialog::ExtractFieldType(int menuValue)
{
	switch(menuValue) {
	case kFieldTypeMenuBool:
		return FLDTYPE_BOOLEAN;
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

void
CEditDBDialog::AnnounceFailToOpenDatabase(const FSSpec& f, ExceptionCode err)
{
	if (!IsQuietError(err)) {
		string errorString(LoadString(kFMJErrorStrings, kCantOpenFileMakerIndex));
		Substitute(errorString, "%%1", AsString(f.name));
		UInt32 fileType = Folders::GetFileType(f);
		string theType((char*)&fileType, 4);
		Substitute(errorString, "%%2", theType);
		Substitute(errorString, "%%3", ConvertErrorToString(err));
		LErrorMsg::AlertWithMessage(kAlertStopAlert, errorString);
	}
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
}

}
