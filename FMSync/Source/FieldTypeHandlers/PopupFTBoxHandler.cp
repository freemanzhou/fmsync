#include <LPushButton.h>
#include <LRadioButton.h>

#include "CEditDBDialog.h"
#include "CEditPopupDialog.h"
#include "FMAE.h"
#include "JFile5.h"
#include "PaneUtilities.h"
#include "PopupFTBoxHandler.h"

PopupFieldTypeBoxHandler::PopupFieldTypeBoxHandler(CEditDBDialog* d)
	: FieldTypeBoxHandler(d)
{
}

void PopupFieldTypeBoxHandler::FinishCreateSelf()
{
	FindPane(fView, 'npop', &fNormalListRadio);
	FindPane(fView, 'mpop', &fMultiListRadio);
}

void PopupFieldTypeBoxHandler::SetupForField(UInt32 fieldType)
{
	fNormalListRadio->SetValue(fieldType == JFile5::FLDTYPE_LIST);
	fMultiListRadio->SetValue(fieldType == JFile5::FLDTYPE_MULTLIST);
	
	LPushButton* button;
	FindPane(fView, 'popv', &button);
	button->AddListener(this);
}

UInt32 PopupFieldTypeBoxHandler::GetJFileFieldType()
{
	if (fMultiListRadio->GetValue())
		return JFile5::FLDTYPE_MULTLIST;
	return JFile5::FLDTYPE_LIST;
}

void PopupFieldTypeBoxHandler::DoEditPopupValues()
{
	if (fDialog->fLastSelected != -1) {
		FMAE::FieldID fieldID(fDialog->GetLastSelectedField());
		CEditPopupDialog editPD(fDialog->fFieldAttributes.fFieldPopups[fieldID]);
		if (!editPD.DoDialog()) {
			fDialog->fFieldAttributes.fFieldPopups[fieldID] = editPD.GetValues();
		}
	}
}

void PopupFieldTypeBoxHandler::ListenToMessage(MessageT inMessage, void* param)
{
	if(inMessage == 'popv') {
		DoEditPopupValues();
	}
}
