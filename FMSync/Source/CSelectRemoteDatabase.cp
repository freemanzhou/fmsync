#include <LStaticText.h>
#include <LGAStaticTextImp.h>
#include <LAMStaticTextImp.h>
#include <LPushButton.h>

#include "CSelectRemoteDatabase.h"
#include "Utilities.h"
#include "FMCResources.h"
#include "Stringiness.h"
#include "Str255.h"
#include "CStringTable.h"

CSelectRemoteDatabase::CSelectRemoteDatabase(SInt32 lastTime, const vector<string>& names)
	: CConduitDialog(kWINDSelectRemotedDatabase), fTime(lastTime), fNames(names)
{
	sort(fNames.begin(), fNames.end());
}

CSelectRemoteDatabase::~CSelectRemoteDatabase()
{
}


bool
CSelectRemoteDatabase::HasSelection()
{
	return fList->HasSelection();
}

bool
CSelectRemoteDatabase::SelectedIndex(int& index)
{
	return fList->SelectedIndex(index);
}
	
void
CSelectRemoteDatabase::UpdateControlState()
{
	if (HasSelection()) {
		fOkayButton->Enable();
	} else {
		fOkayButton->Disable();
	}
}

void
CSelectRemoteDatabase::UpdateWindow()
{
}

void
CSelectRemoteDatabase::UpdateTime()
{
	string updateString;
	if (fTime == 0) {
		updateString = LoadString(kSTRUploadStrings, 1);
	} else if (fNames.size() == 0) {
		updateString = LoadString(kSTRUploadStrings, 2);
	} else {
		updateString = LoadString(kSTRUploadStrings, 3);
	}
	Substitute(updateString, "%%1", TimeAsString(fTime));
	Substitute(updateString, "%%2", DateAsString(fTime));
	fUpdatedNote->SetDescriptor(AsStr255(updateString));
}

void
CSelectRemoteDatabase::RegisterClasses()
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
CSelectRemoteDatabase::FinishCreateSelf()
{
	FindPane(fView, 'Date', &fUpdatedNote);
	FindPane(fView, 'List', &fList);
	fList->SetStrings(fNames);
	UpdateTime();
	UpdateWindow();
}

bool
CSelectRemoteDatabase::DialogDone()
{
	int index;
	if (SelectedIndex(index)) {
		fSelectedName = fNames[index];
	}
	return true;
}

string
CSelectRemoteDatabase::DatabaseName() const
{
	return fSelectedName;
}