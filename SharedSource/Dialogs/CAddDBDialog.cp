#include <LListBox.h>
#include <LPopupButton.h>
#include <LStaticText.h>
#include <LAMStaticTextImp.h>
#include <LGAStaticTextImp.h>
#include <LAMPopupButtonImp.h>
#include <LGAPopupButtonImp.h>
#include <LPushButton.h>

#include "CAddDBDialog.h"
#include "CDatabaseFile.h"
#include "Utilities.h"
#include "Str255.h"
#include "Stringiness.h"
#include "syncmgr.h"
#include "CFileMakerDatabase.h"
#include "Utilities.h"

CAddDBDialog::CAddDBDialog(CDatabaseFile* inDB)
	: CConduitDialog(kWindowID), fDatabase(inDB), fArchiveID(0), 
			fPalmMenuH(0), fKeyFieldMenuH(0), fArchiveMenuH(0)
{
}

CAddDBDialog::~CAddDBDialog()
{
	delete fPalmMenuH;
	delete fKeyFieldMenuH;
	delete fArchiveMenuH;
}

static void
Enable(LPushButton *foo, Boolean enable)
{
	if (enable) {
		foo->Enable();
	} else {
		foo->Disable();
	}
}

static void
SetButtonTitle(LPushButton *button, const LStr255& theTitle)
{
	LStr255 currentTitle;
	button->GetDescriptor(currentTitle);
	if (currentTitle != theTitle)
		button->SetDescriptor(theTitle);
}

Boolean
CAddDBDialog::FieldCanBeMoved(int fieldID)
{
	if (fieldID == fArchiveID)
		return false;
	return find(fFieldsToMove.begin(), fFieldsToMove.end(), fieldID) == fFieldsToMove.end();
}

Boolean
CAddDBDialog::AnyFieldCanBeMoved()
{
	for (int i = 0; i < fFieldIDs.size(); i += 1) {
		if (FieldCanBeMoved(fFieldIDs[i]))
			return true;
	}
	return false;
}

Boolean
CAddDBDialog::AnyFieldCanBeCleared()
{
	for (int i = 0; i < fFieldsToMove.size(); i += 1) {
		if (FieldCanBeCleared(fFieldsToMove[i]))
			return true;
	}
	return false;
}

Boolean
CAddDBDialog::FieldCanBeCleared(int fieldID)
{
	if (fFieldsToMove.size() < 2)
		return false;
	return find(fFieldsToMove.begin(), fFieldsToMove.end(), fieldID) != fFieldsToMove.end();
}

const LStr255 kMoveTitle("\pMove");
const LStr255 kClearTitle("\pClear");

void
CAddDBDialog::UpdateControlState()
{
	Cell cell;
	UpdateMenus();
	if (fAllFieldListBox->IsTarget() && fAllFieldListBox->GetLastSelectedCell(cell)) {
		fPalmFieldListBox->UnselectAllCells();
		SetButtonTitle(fMoveButton, kMoveTitle);
		Enable(fMoveButton, FieldCanBeMoved(fFieldIDs[cell.v]));
		fUpButton->Disable();
		fDownButton->Disable();
	} else if (fPalmFieldListBox->IsTarget() && fPalmFieldListBox->GetLastSelectedCell(cell)) {
		fAllFieldListBox->UnselectAllCells();
		SetButtonTitle(fMoveButton, kClearTitle);
		Enable(fMoveButton, FieldCanBeCleared(fFieldsToMove[cell.v]));
		Enable(fUpButton, cell.v > 0);
		Enable(fDownButton, cell.v < fFieldsToMove.size()-1);
	} else {
		fMoveButton->Disable();
		fUpButton->Disable();
		fDownButton->Disable();
	}
	Enable(fMoveAllButton, AnyFieldCanBeMoved());
	Enable(fClearAllButton, false);
}

void
CAddDBDialog::ListenToMessage(MessageT inMessage, void* param)
{
	switch(inMessage) {
	case 'Move':
		if (fAllFieldListBox->IsTarget()) {
			HandleMove();
		} else {
			HandleClear();
		}
		break;
	case 'ClAl':
		HandleClearAll();
		break;
	case 'MoAl':
		HandleMoveAll();
		break;
	case 'Up  ':
		HandleUp();
		break;
	case 'Down':
		HandleDown();
		break;
	default:
		CConduitDialog::ListenToMessage(inMessage, param);
		break;
	}
}

static void
SetupListBox(LListBox* listB)
{
	ListHandle lh = listB->GetMacListH();
	::LAddColumn(1, 0, lh);
	(**lh).selFlags |= lOnlyOne;
}

void
CAddDBDialog::FinishCreateSelf()
{
#if 0
	FindPane(fView, 'KeyF', &fKeyFieldMenu);
	fKeyFieldMenu->AddListener(this);
	FindPane(fView, 'PIDF', &fPalmIDMenu);
	fPalmIDMenu->AddListener(this);
	FindPane(fView, 'ArcF', &fArchiveMenu);
	fArchiveMenu->AddListener(this);
#endif
	FindPane(fView, 'FM  ', &fAllFieldListBox);
	fAllFieldListBox->AddListener(this);
	SetupListBox(fAllFieldListBox);
	FindPane(fView, 'Palm', &fPalmFieldListBox);
	fPalmFieldListBox->AddListener(this);
	SetupListBox(fPalmFieldListBox);
	FindPane(fView, 'Name', &fDatabaseName);
	FindPane(fView, 'Move', &fMoveButton);
	FindPane(fView, 'MoAl', &fMoveAllButton);
	FindPane(fView, 'ClAl', &fClearAllButton);
	FindPane(fView, 'Up  ', &fUpButton);
	FindPane(fView, 'Down', &fDownButton);
	//ProcessSerialNumber myPSN;
	//ThrowIfOSErr_(GetFrontProcess(&myPSN));
	SetupForDatabase();
	//ThrowIfOSErr_(SetFrontProcess(&myPSN));
	UpdateMenus();
	UpdateLists();
}

void
CAddDBDialog::RegisterClasses()
{
	CConduitDialog::RegisterClasses();
	static Boolean firstRun = true;
	if (firstRun) {
		firstRun = false;
		RegisterClass_(LPopupButton);
		RegisterClass_(LStaticText);
		if (UEnvironment::HasFeature (env_HasAppearance)) {
			RegisterClassID_(LAMStaticTextImp, LStaticText::imp_class_ID);
			RegisterClassID_(LAMPopupButtonImp, LPopupButton::imp_class_ID);
		} else {
			RegisterClassID_( LGAStaticTextImp,			LStaticText::imp_class_ID );
			RegisterClassID_( LGAPopupButtonImp,			LPopupButton::imp_class_ID );
		}
	}
}

void
CAddDBDialog::SetupForDatabase()
{
	FSSpec spec;
	fDatabase->GetFSSpec(&spec);
	fDatabaseName->SetDescriptor(spec.name);
	CFileMakerDatabase db(spec);
	fFieldNames = db.GetFieldNames();
	if (fFieldNames.size() == 0)
		return;
	fFieldIDs = db.GetFieldIDs();
	fFieldsToMove = fFieldIDs;
	
	vector<string>::iterator i = fFieldNames.begin();
	int itemIndex = 1;
	int selectedIndex = 1;
	while (i != fFieldNames.end()) {
		string fieldName = *i;
		int fieldID = fFieldIDs[itemIndex - 1];
		fFieldNameMap[fieldID] = fieldName;
		//LStr255 fieldNameStr(AsStr255(fieldName));
		//fArchiveMenu->AppendMenu(fieldNameStr);
		i++;
		itemIndex += 1;
	}
	FillListBox(fAllFieldListBox, fFieldNames);
	UpdateLists();
}

void
CAddDBDialog::UpdateMenus()
{
}

void
CAddDBDialog::UpdateLists()
{
	if (fArchiveID) {
		vector<int>::iterator i = find(fFieldsToMove.begin(), fFieldsToMove.end(), fArchiveID);
		if (i != fFieldsToMove.end()) {
			fFieldsToMove.erase(i);
		}
	}
	vector<string> movedFieldNames;
	for (vector<int>::iterator i = fFieldsToMove.begin(); i != fFieldsToMove.end(); ++i) {
		string fieldName = fFieldNameMap[*i];
		movedFieldNames.push_back(fieldName);
	}
	FillListBox(fPalmFieldListBox, movedFieldNames);
}

void
CAddDBDialog::HandleMove()
{
	Cell cell;
	if (fAllFieldListBox->GetLastSelectedCell(cell)) {
		fAllFieldListBox->UnselectAllCells();
		int fieldID = fFieldIDs[cell.v];
		fFieldsToMove.push_back(fieldID);
		UpdateLists();
	}
}

void
CAddDBDialog::HandleClear()
{
	Cell cell;
	if (fPalmFieldListBox->GetLastSelectedCell(cell)) {
		fPalmFieldListBox->UnselectAllCells();
		vector<int>::iterator i = fFieldsToMove.begin() + cell.v;
		fFieldsToMove.erase(i);
		if (cell.v == fFieldsToMove.size())
			cell.v -= 1;
		fPalmFieldListBox->SelectOneCell(cell);
		UpdateLists();
	}
}

void
CAddDBDialog::HandleClearAll()
{
	fPalmFieldListBox->UnselectAllCells();
	fFieldsToMove.clear();
	UpdateLists();
}

void
CAddDBDialog::HandleMoveAll()
{
	fPalmFieldListBox->UnselectAllCells();
	fFieldsToMove = fFieldIDs;
	UpdateLists();
}

void
CAddDBDialog::HandleUpOrDown(int delta)
{
	Cell cell;
	if (fPalmFieldListBox->GetLastSelectedCell(cell)) {
		fPalmFieldListBox->UnselectAllCells();
		vector<int>::iterator i = fFieldsToMove.begin() + cell.v;
		vector<int>::iterator j = i + delta;
		int fieldID = *i;
		*i = *j;
		*j = fieldID;
		cell.v += delta;
		fPalmFieldListBox->SelectOneCell(cell);
		UpdateLists();
	}
}

void
CAddDBDialog::HandleUp()
{
	HandleUpOrDown(-1);
}

void
CAddDBDialog::HandleDown()
{
	HandleUpOrDown(1);
}

void
CAddDBDialog::DialogDone()
{
	if (!fCanceled) {
		CFieldList fieldList;
		vector<int>::iterator i = fFieldsToMove.begin();
		int index = 0;
		while (i != fFieldsToMove.end()) {
			string fieldName(fFieldNames[index]);
			CField aField(fieldName, fieldName);
			aField.fFMID = *i;
			fieldList.push_back(aField);
			++i;
			++index;
		}
		fDatabase->SetFieldList(fieldList);
	}
}

