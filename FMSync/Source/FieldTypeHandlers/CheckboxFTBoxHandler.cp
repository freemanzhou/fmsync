#include <LRadioButton.h>

#include "CEditDBDialog.h"
#include "CEditPopupDialog.h"
#include "FMAE.h"
#include "JFile5.h"
#include "PaneUtilities.h"
#include "CheckboxFTBoxHandler.h"

CheckboxFieldTypeBoxHandler::CheckboxFieldTypeBoxHandler(CEditDBDialog* d)
	: FieldTypeBoxHandler(d)
{
}

void CheckboxFieldTypeBoxHandler::FinishCreateSelf()
{
}

void CheckboxFieldTypeBoxHandler::SetupForField(UInt32 fieldType)
{
}

UInt32 CheckboxFieldTypeBoxHandler::GetJFileFieldType()
{
	return JFile5::FLDTYPE_FMSYNC_CHECKBOX;
}

void CheckboxFieldTypeBoxHandler::DoEditPopupValues()
{
	if (fDialog->fLastSelected != -1) {
		FMAE::FieldID fieldID(fDialog->GetLastSelectedField());
		CEditPopupDialog editPD(fDialog->fFieldAttributes.fFieldPopups[fieldID]);
		if (!editPD.DoDialog()) {
			fDialog->fFieldAttributes.fFieldPopups[fieldID] = editPD.GetValues();
		}
	}
}
