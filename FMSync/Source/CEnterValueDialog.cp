#include <LStaticText.h>
#include <LEditText.h>
#include <LPushButton.h>

#include "CEnterValueDialog.h"
#include "PaneUtilities.h"
#include "FMCResources.h"

CEnterValueDialog::CEnterValueDialog(const string& v)
	: CConduitDialog(kWINDEnterValue), fValue(v)
{
}

CEnterValueDialog::~CEnterValueDialog()
{
}

void
CEnterValueDialog::FinishCreateSelf()
{
	FindPane(fView, 'valu', &fValueField);
	fView->SetLatentSub(fValueField);
	FindPane(fView, 'OK  ', &fOKButton);
	SetDefaultButton(fOKButton);
}

string CEnterValueDialog::GetValue() const
{
	return fValue;
}

void CEnterValueDialog::UpdateControlState()
{
	if (GetTextFromEditBox(fValueField).length()) {
		fOKButton->Enable();
	} else {
		fOKButton->Disable();
	}
}

bool CEnterValueDialog::DialogDone()
{
	fValue = GetTextFromEditBox(fValueField);
	return true;
}
