#include <LStaticText.h>
#include <LListBox.h>
#include <LBevelButton.h>
#include <LRadioGroupView.h>
#include <LPopupButton.h>
#include <LPopupGroupBox.h>
#include <LMultiPanelView.h>

#include "CDataTasksDialog.h"
#include "CDatabaseFile.h"
#include "PaneUtilities.h"
#include "CSyncIdler.h"
#include "Str255.h"
#include "Stringiness.h"
#include "StringList.h"
#include "FMDatabase.h"
#include "CStringTable.h"
#include "CTaskEdit.h"
#include "FMCResources.h"
#include "OtherStrings.h"
#include "LErrorMsg.h"

enum {kMenuItemMerge = 1, kMenuItemKeepBoth};

CStringList gStateStrings(kSTRDataTaskStateStrings);
CStringList gActionStrings(kSTRDataTaskActionStrings);

CDataTasksDialog::CDataTasksDialog(CDatabaseFile::Ptr inDB, 
								FileMaker::DatabasePtr fmDB, vector<CDataTask> &theTasks,
								const CFieldIDList& existingLocalFields,
								const vector<int>& existingLocalRepeats)
	: CConduitDialog(kWINDSynchronizationActions), fDatabase(inDB), 
	fTasks(theTasks), fFileMakerDatabase(fmDB), 
	fExistingLocalFields(existingLocalFields),
	fExistingLocalRepeats(existingLocalRepeats)
{
	ThrowIfNil_(fDatabase.get());
	ThrowIfNil_(fFileMakerDatabase.get());
	fFieldNames = fFileMakerDatabase->GetFieldNames();
	int totalCount = fTasks.size();
	for (int i = 0; i < totalCount; i += 1) {
		if (fTasks[i].IsConflict())
			fConflicts.push_back(i);
	}
}

CDataTasksDialog::~CDataTasksDialog()
{
}

void
CDataTasksDialog::UpdateControlState()
{
}

void
CDataTasksDialog::UpdateCurrentTask()
{
	CDataTask& currentTask = CurrentTask();
	fTasksEditor->GetTask(currentTask);
	int item = fActionBox->GetCurrentMenuItem();
	if (item == kMenuItemMerge) {
		currentTask.SetDuplicate(false);
	} else {
		currentTask.SetDuplicate(true);
	}
}

void
CDataTasksDialog::ListenToMessage(MessageT inMessage, void* param)
{
	switch(inMessage) {
	case 'next':
		if (fCurrentTaskIndex < fConflicts.size() - 1) {
			UpdateCurrentTask();
			NextTask(1);
			ShowTask();
		}
		break;
	case 'prev':
		if (fCurrentTaskIndex > 0) {
			UpdateCurrentTask();
			NextTask(-1);
			ShowTask();
		}
		break;
	case 'uall':
		DoUseForAll();
		break;
	default:
		CConduitDialog::ListenToMessage(inMessage, param);
		break;
	}
}

void
CDataTasksDialog::FinishCreateSelf()
{
	FindPane(fView, 'mult', &fMPV);
	fMPV->CreateAllPanels();
	FindPane(fView, 'Task', &fTasksEditor);
	FindPane(fView, 'prev', &fPrevious);
	FindPane(fView, 'next', &fNext);
	FindPane(fView, 'tcnt', &fTaskCountLabel);
	FindPane(fView, 'reso', &fActionBox);
	SetIdler(new CSyncIdler());
	SetupForDatabase();
}

bool
CDataTasksDialog::DialogDone()
{
	bool goAhead = true;
	UpdateCurrentTask();
	if (fViewedMap.size() < fConflicts.size()) {
		string sure(LoadString(kOtherStrings, kSureSynchronizeIndex));
		string hint(LoadString(kOtherStrings, kUseSmallButtonsIndex));
		short itemHit = LErrorMsg::AlertWithMessageAndDetails(kAlertCautionAlert, sure, hint, true);
		if (itemHit == kAlertStdAlertCancelButton)
			goAhead = false;
	}
	return goAhead;
}

void
CDataTasksDialog::RegisterClasses()
{
	CConduitDialog::RegisterClasses();
}

static void
SetupListBox(LListBox* listB)
{
	ListHandle lh = listB->GetMacListH();
	::LAddColumn(3, 0, lh);
	(**lh).selFlags |= lOnlyOne;
	Point cSize = (**lh).cellSize;
	cSize.h /= 3;
	::LCellSize(cSize, lh);
}

void
CDataTasksDialog::SetupForDatabase()
{
	FSSpec spec;
	fDatabase->GetFSSpec(&spec);
	string wTitle(LoadString(kOtherStrings, kSyncTasksIndex));
	wTitle += AsString(spec.name);
	wTitle += 'Ó';
	SetWindowTitle(wTitle);

	fCurrentTaskIndex = 0;
	fTasksEditor->SetFMDatabase(fFileMakerDatabase);
	fTasksEditor->SetDatabaseFile(fDatabase);
	ShowTask();
}

void
CDataTasksDialog::DoUseForAll()
{
	int item = fActionBox->GetCurrentMenuItem();
	bool dup = (item == kMenuItemKeepBoth);
	for (int i = 0; i < fConflicts.size(); i++) {
		fTasks[fConflicts[i]].SetDuplicate(dup);
	}
	ShowTask();
}

void
CDataTasksDialog::NextTask(int delta)
{
	fCurrentTaskIndex += delta;
}

void
CDataTasksDialog::ShowTask()
{
	fViewedMap[fCurrentTaskIndex] = true;
	fTasksEditor->SetTask(CurrentTask(), fExistingLocalFields, fExistingLocalRepeats);
	string taskLabel(AsString(fCurrentTaskIndex+1));
	taskLabel += "/";
	taskLabel += AsString(fConflicts.size());
	fTaskCountLabel->SetDescriptor(AsStr255(taskLabel));
	if (CurrentTask().GetDuplicate()) {
		fActionBox->SetCurrentMenuItem(kMenuItemKeepBoth);
	} else {
		fActionBox->SetCurrentMenuItem(kMenuItemMerge);
	}
}

