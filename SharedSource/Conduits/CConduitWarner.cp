/* 
	CConduitWarner.cp

	Author:			Rob Tsuk
	Description:	<describe the CConduitWarner class here>
*/

#include "CConduitWarner.h"
#include "CConduitWarningDialog.h"
#include "WarningsStrings.h"
#include "Str255.h"

CConduitWarner::CConduitWarner(CWarnings& w)
	: fWarnings(w)
{
}


CConduitWarner::~CConduitWarner()
{
}

bool
CConduitWarner::Warn(Warnings::WarningID warnID)
{
	bool disabled = fWarnings.Disabled(warnID);
	if (!disabled) {
		CConduitWarningDialog theDialog(LoadString(kWarningsTitleStrings, warnID), 
			LoadString(kWarningsDescStrings, warnID));
		theDialog.DoDialog();
		disabled = theDialog.WasDisabled();
		fWarnings.SetDisabled(warnID, disabled);
	}
	return disabled;
}
