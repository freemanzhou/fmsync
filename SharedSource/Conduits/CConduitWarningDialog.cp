#include <LStaticText.h>
#include <LCheckBox.h>

#include "CConduitWarningDialog.h"
#include "PaneUtilities.h"
#include "FMCResources.h"
#include "Str255.h"

CConduitWarningDialog::CConduitWarningDialog(const string& title, const string& description)
	: CConduitDialog(kWINDWarning), fTitle(title), fDesc(description)
{
}

CConduitWarningDialog::~CConduitWarningDialog()
{
}

void
CConduitWarningDialog::FinishCreateSelf()
{
	FindPane(fView, 'titl', &fTitleText);
	fTitleText->SetDescriptor(AsStr255(fTitle));

	FindPane(fView, 'desc', &fDescText);
	fDescText->SetDescriptor(AsStr255(fDesc));

	FindPane(fView, 'show', &fDisable);
}

bool
CConduitWarningDialog::DialogDone()
{
	fWasDisabled = fDisable->GetValue();
	return true;
}
