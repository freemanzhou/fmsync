#include <LTextEditView.h>

#include "CEditPopupDialog.h"
#include "Utilities.h"
#include "FMCResources.h"
#include "Str255.h"
#include "Stringiness.h"

CEditPopupDialog::CEditPopupDialog(const vector<string>& v)
	: CConduitDialog(kWINDEditPopup), fValues(v)
{
}

CEditPopupDialog::~CEditPopupDialog()
{
}

void
CEditPopupDialog::FinishCreateSelf()
{
	FindPane(fView, 'popv', &fText);
	fView->SetLatentSub(fText);
	LCommander::SwitchTarget(fText);
	
	string valueString;
	for (vector<string>::const_iterator i = fValues.begin(); i != fValues.end(); ++i) {
		valueString += *i;
		valueString += "\r";
	}
	fText->SetTextPtr(const_cast<Ptr>(valueString.data()), valueString.length());
}

bool
CEditPopupDialog::DialogDone()
{
	return true;
}

vector<string>
CEditPopupDialog::GetValues()
{
	vector<string> v(SplitString(GetText(fText), "\r"));
	return v;
}
