#include <LRadioButton.h>

#include "CChooseAccessMethod.h"
#include "FMCResources.h"
#include "Utilities.h"

CChooseAccessMethod::CChooseAccessMethod(bool preferUseLocal)
	: CConduitDialog(kWINDChooseAccessMethod), fPreferUseLocal(preferUseLocal), fUseLocal(preferUseLocal)
{
}

CChooseAccessMethod::~CChooseAccessMethod()
{
}

void
CChooseAccessMethod::UpdateControlState()
{
}

void
CChooseAccessMethod::UpdateWindow()
{
}

void
CChooseAccessMethod::RegisterClasses()
{
	CConduitDialog::RegisterClasses();
}

void
CChooseAccessMethod::FinishCreateSelf()
{
	FindPane(fView, 'host', &fExisting);
	FindPane(fView, 'disk', &fLocal);
	if (fPreferUseLocal) {
		fLocal->SetValue(1);
		fExisting->SetValue(0);
	} else {
		fLocal->SetValue(0);
		fExisting->SetValue(1);
	}
	UpdateWindow();
}

bool
CChooseAccessMethod::DialogDone()
{
	fUseLocal = (fLocal->GetValue());
	return true;
}

bool
CChooseAccessMethod::UseLocal() const
{
	return fUseLocal;
}
