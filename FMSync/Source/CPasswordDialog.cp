#include <LStaticText.h>
#include <LEditText.h>
#include <LPushButton.h>

#include "CPasswordDialog.h"
#include "Utilities.h"
#include "FMCResources.h"
#include "Str255.h"

CPasswordDialog::CPasswordDialog(const string& databaseName)
	: CConduitDialog(kWINDPassword), fName(databaseName)
{
}

CPasswordDialog::~CPasswordDialog()
{
}

void
CPasswordDialog::FinishCreateSelf()
{
	FindPane(fView, 'pass', &fPassword);
	FindPane(fView, 'desc', &fDescription);
	fView->SetLatentSub(fPassword);
	FindPane(fView, 'OK  ', &fOKButton);
	SetDefaultButton(fOKButton);
}

string CPasswordDialog::GetPassword()
{
	Str255 password255;
	fPassword->GetDescriptor(password255);
	return (AsString(password255));
}

void CPasswordDialog::UpdateControlState()
{
	if (GetPassword().length()) {
		fOKButton->Enable();
	} else {
		fOKButton->Disable();
	}
}
