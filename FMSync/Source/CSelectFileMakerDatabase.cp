#include <LStaticText.h>
#include <LGAStaticTextImp.h>
#include <LAMStaticTextImp.h>
#include <LPushButton.h>
#include <iterator>

#include "CSelectFileMakerDatabase.h"
#include "Utilities.h"
#include "FMCResources.h"
#include "Stringiness.h"
#include "Str255.h"
#include "CStringTable.h"

CSelectFileMakerDatabase::CSelectFileMakerDatabase(vector<CDatabaseName>& names)
	: CConduitDialog(kWINDSelectFileMakerDatabase), fNames(names)
{
}

CSelectFileMakerDatabase::~CSelectFileMakerDatabase()
{
}


bool
CSelectFileMakerDatabase::HasSelection()
{
	return fList->HasSelection();
}

bool
CSelectFileMakerDatabase::SelectedIndex(int& index)
{
	return fList->SelectedIndex(index);
}
	
void
CSelectFileMakerDatabase::UpdateControlState()
{
	if (HasSelection()) {
		fOkayButton->Enable();
	} else {
		fOkayButton->Disable();
	}
}

void
CSelectFileMakerDatabase::UpdateWindow()
{
}

void
CSelectFileMakerDatabase::RegisterClasses()
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
CSelectFileMakerDatabase::FinishCreateSelf()
{
	FindPane(fView, 'List', &fList);
	fList->SetStrings(DatabaseNamesAsStrings());
	UpdateWindow();
}

bool
CSelectFileMakerDatabase::DialogDone()
{
	int index;
	if (SelectedIndex(index)) {
		fSelectedName = fNames[index];
	}
	return true;
}

CDatabaseName
CSelectFileMakerDatabase::DatabaseName() const
{
	return fSelectedName;
}

vector<string>
CSelectFileMakerDatabase::DatabaseNamesAsStrings() const
{
	vector<string> results;
	vector<CDatabaseName> theNames(fNames);
	
	transform(theNames.begin(), theNames.end(), back_inserter(results), mem_fun_ref(&CDatabaseName::GetName));
	return results;
}
