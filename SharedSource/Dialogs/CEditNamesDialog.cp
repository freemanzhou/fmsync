#include <LStaticText.h>
#include <LListBox.h>
#include <LBevelButton.h>
#include <LRadioGroupView.h>
#include <LSlider.h>

#include "CEditNamesDialog.h"
#include "CDatabaseFile.h"
#include "Utilities.h"
#include "Str255.h"
#include "Stringiness.h"
#include "StringList.h"
#include "FMDatabase.h"
#include "CStringTable.h"
#include "CFieldNamesEdit.h"
#include "FMCResources.h"
#include "OtherStrings.h"

#if 0
CEditNamesDialog::CEditNamesDialog(const string& databaseName, const vector<string>& pilotNames, 
								const vector<string>& fmNames)
	: CConduitDialog(kWINDSpecifyFields), fDatabaseName(databaseName), fPilotNames(pilotNames), fFMNames(fmNames)
{
}

CEditNamesDialog::~CEditNamesDialog()
{
}

void
CEditNamesDialog::UpdateControlState()
{
}

void
CEditNamesDialog::ListenToMessage(MessageT inMessage, void* param)
{
	switch(inMessage) {
	default:
		CConduitDialog::ListenToMessage(inMessage, param);
		break;
	}
}

void
CEditNamesDialog::FinishCreateSelf()
{
	FindPane(fView, 'FNEd', &fFieldNamesEditor);
	SetupForDatabase();
}

bool
CEditNamesDialog::DialogDone()
{
	return true;
}

vector<int>
CEditNamesDialog::GetSelection()
{
	return fFieldNamesEditor->GetSelection();
}

vector<int>
CEditNamesDialog::GetOrder()
{
	return fFieldNamesEditor->GetOrder();
}

void
CEditNamesDialog::RegisterClasses()
{
	CConduitDialog::RegisterClasses();
}

void
CEditNamesDialog::SetupForDatabase()
{
	string wTitle(LoadString(kOtherStrings, kSpecifyFieldNamesIndex));
	
	wTitle += fDatabaseName;
	wTitle += 'Ó';
	SetWindowTitle(wTitle);

	fFieldNamesEditor->Setup(fPilotNames, fFMNames);
}
#endif