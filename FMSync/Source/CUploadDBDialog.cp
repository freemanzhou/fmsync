#include <LStaticText.h>
#include <LPushButton.h>
#include <LCheckBox.h>
#include <LTextGroupBox.h>
#include <UNavServicesDialogs.h>

#include "CUploadDBDialog.h"
#include "CSelectRemoteDatabase.h"
#include "Utilities.h"
#include "FMCResources.h"
#include "Stringiness.h"
#include "Str255.h"
#include "CStringTable.h"
#include "DescribeFiles.h"
#include "DebugOutput.h"

CUploadDBDialog::CUploadDBDialog(SInt32 lastTime, const vector<string>& names,
								const vector<CUploadRequest>& req)
	: CConduitDialog(kWINDUploadDatabase), fTime(lastTime), fNames(names), fReqs(req),
	fIndex(-1)
{
	sort(fNames.begin(), fNames.end());
}

CUploadDBDialog::~CUploadDBDialog()
{
}


bool
CUploadDBDialog::HasSelection()
{
	return fList->HasSelection();
}

bool
CUploadDBDialog::SelectedIndex(int& index)
{
	return fList->SelectedIndex(index);
}
	
void
CUploadDBDialog::SetSelectedIndex(int index)
{
	SaveSelectedValues();
	fIndex = index;
	if (index != -1) {
		fList->SelectIndex(index);
	} else {
		fList->SelectNone();
	}
	fList->Refresh();
	UpdateControlForSelection(index, index != -1);
}
	
void
CUploadDBDialog::SelectNone()
{
	SetSelectedIndex(-1);
}
	
void
CUploadDBDialog::UpdateControlState()
{
}

void
CUploadDBDialog::SaveSelectedValues()
{
	if (fIndex != -1) {
		fReqs[fIndex].fChanges = fChangedBox->GetValue();
	}
}

void
CUploadDBDialog::UpdateControlForSelection(int index, bool hasSelection)
{
	if (hasSelection) {
		fRemoveButton->Enable();
		fOptionsBox->Enable();
		fChangedBox->SetValue(fReqs[index].fChanges);
		FSSpec spec;
		fReqs[index].GetLocation(spec);
		string fileLoc(DescribeFile(spec));
		Str255 locStr;
		fLocationText->GetDescriptor(locStr);
		string oldFileLoc(AsString(locStr));
		if (oldFileLoc != fileLoc) {
			fLocationText->SetDescriptor(AsStr255(fileLoc));
		}
	} else {
		fRemoveButton->Disable();
		fOptionsBox->Disable();
		fLocationText->SetDescriptor("\p");
	}
}

void
CUploadDBDialog::UpdateWindow()
{
	vector<string> upNames;
	for (vector<CUploadRequest>::const_iterator i = fReqs.begin();
		i != fReqs.end(); ++i) {
		upNames.push_back(i->fName);
	}
	fList->SetStrings(upNames);
	fList->Refresh();
}

void
CUploadDBDialog::RegisterClasses()
{
	CConduitDialog::RegisterClasses();
	static Boolean firstRun = true;
	if (firstRun) {
		firstRun = false;
	}
}

void
CUploadDBDialog::FinishCreateSelf()
{
	FindPane(fView, 'List', &fList);
	FindPane(fView, 'UplO', &fOptionsBox);
	FindPane(fView, 'AddD', &fAddButton);
	FindPane(fView, 'Remo', &fRemoveButton);
	FindPane(fView, 'SetL', &fSetLocationButton);
	FindPane(fView, 'Chan', &fChangedBox);
	FindPane(fView, 'Loca', &fLocationText);
	fList->AddListener(this);
	UpdateWindow();
	int index;
	bool hasSelection = SelectedIndex(index);
	UpdateControlForSelection(index, hasSelection);
}

bool
CUploadDBDialog::DialogDone()
{
	SaveSelectedValues();
	return true;
}

void
CUploadDBDialog::ListenToMessage(MessageT inMessage, void* param)
{
	if (inMessage == 'AddD') {
		DoAdd();
	} else if (inMessage == 'Remo') {
		DoRemove();
	} else if (inMessage == 'SetL') {
		if (fIndex != -1) {
			DoSetLocation(fReqs[fIndex]);
			int index;
			bool hasSelection = SelectedIndex(index);
			UpdateControlForSelection(index, hasSelection);
		}
	} else if (inMessage == 'Chan') {
		DoSetChanges();
	} else if (inMessage == CStringTable::kSelectionChanged) {
		int index;
		bool hasSelection = SelectedIndex(index);
		UpdateControlForSelection(index, hasSelection);
		fIndex = index;	
	} else
		CConduitDialog::ListenToMessage(inMessage, param);
}

void
CUploadDBDialog::DoRemove()
{
	if (fIndex != -1) {
		int index = fIndex;
		SelectNone();
		fReqs.erase(fReqs.begin() + index);
		UpdateWindow();
	}
}

bool
CUploadDBDialog::DoSetLocation(CUploadRequest& thisOne)
{
	FSSpec location = {};
	if (!thisOne.GetLocation(location) && location.name[0] == 0) {
		string localName(MakeLocalName(thisOne.fName));
		LString::CopyPStr(AsStr255(localName), location.name);
	}
	UNavServicesDialogs::StNavReplyRecord reply;
	NavDialogOptions options;
	::NavGetDefaultDialogOptions(&options);
	OSErr err = ::NavPutFile(
						0,
						reply,
						&options,
						0,
						'TEXT',
						0,
						0L);
	
	if (reply.IsValid()) {
		FSSpec endLocation;
		reply.GetFileSpec(endLocation);
		thisOne.SetLocation(endLocation);
	}

	return reply.IsValid();
}

const string kLocalNameExtension = ".txt";

string
CUploadDBDialog::MakeLocalName(const string& baseName)
{
	string localName(baseName.substr(0, 31 - kLocalNameExtension.length()));
	localName += kLocalNameExtension;
	return localName;
}

void
CUploadDBDialog::DoSetChanges()
{
}

void
CUploadDBDialog::DoAdd()
{
	SelectNone();
	
	string theName;
	{
		CSelectRemoteDatabase selectRemote(fTime, fNames);
	
		if (selectRemote.DoDialog())
			return;
		theName = selectRemote.DatabaseName();
	}

	CUploadRequest req(theName);
	if (DoSetLocation(req)) {
		fReqs.push_back(req);
		int newIndex = fReqs.size() - 1;
		UpdateWindow();
		SetSelectedIndex(newIndex);
		TableCellT cell;
		fList->GetSelectedCell(cell);
	}
}