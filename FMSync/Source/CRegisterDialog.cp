#include <LStaticText.h>
#include <LEditText.h>

#include "CRegisterDialog.h"
#include "Utilities.h"
#include "FMCResources.h"
#include "Str255.h"

#if 0
CRegisterDialog::CRegisterDialog(const string& userName, const string& regCode)
	: CConduitDialog(kWINDRegister), fUserName(userName), fRegCode(regCode)
{
}

CRegisterDialog::~CRegisterDialog()
{
}

bool
CRegisterDialog::DialogDone()
{
	Str255 regCode;
	fRegistrationCodeField->GetDescriptor(regCode);
	fRegCode = AsString(regCode);
	return true;
}

void
CRegisterDialog::FinishCreateSelf()
{
	FindPane(fView, 'User', &fUserNameField);
	FindPane(fView, 'RegC', &fRegistrationCodeField);
	
	fUserNameField->SetDescriptor(AsStr255(fUserName));
	fRegistrationCodeField->SetDescriptor(AsStr255(fRegCode));

	fView->SetLatentSub(fRegistrationCodeField);
}

string
CRegisterDialog::RegCode() const
{
	return fRegCode;
}
#endif