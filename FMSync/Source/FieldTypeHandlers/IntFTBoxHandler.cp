#include <LEditText.h>
#include <LRadioButton.h>

#include "CEditDBDialog.h"
#include "FMAE.h"
#include "JFile5.h"
#include "PaneUtilities.h"
#include "IntFTBoxHandler.h"

IntFieldTypeBoxHandler::IntFieldTypeBoxHandler(CEditDBDialog* d)
	: FieldTypeBoxHandler(d)
{
}

void IntFieldTypeBoxHandler::FinishCreateSelf()
{
	FindPane(fView, 'auto', &fAutoIncRadio);
	FindPane(fView, 'nori', &fNormalIntRadio);
	FindPane(fView, 'star', &fIncStartEdit);
	FindPane(fView, 'incr', &fIncValueEdit);
	FindPane(fView, 'autv', &fAutoIncView);

	fAutoIncRadio->AddListener(this);
	fNormalIntRadio->AddListener(this);
}

UInt32 IntFieldTypeBoxHandler::GetJFileFieldType()
{
	if (fAutoIncRadio->GetValue())
		return JFile5::FLDTYPE_AUTOINC;
	return JFile5::FLDTYPE_INT;
}

void IntFieldTypeBoxHandler::ListenToMessage( MessageT inMessage, void*)
{
	switch(inMessage) {
	case 'auto':
		EnableAutoIncrementEdit(true);
		break;
	case 'nori':
		EnableAutoIncrementEdit(false);
		break;
	}
}

void IntFieldTypeBoxHandler::EnableAutoIncrementEdit(bool enable)
{
	FMAE::FieldID fieldID(fDialog->GetLastSelectedField());

	if (enable) {
		fAutoIncView->Enable();
		fIncStartEdit->SetValue(fDialog->fFieldAttributes.fFieldExtra[fieldID]);
		fIncValueEdit->SetValue(fDialog->fFieldAttributes.fFieldExtra2[fieldID]);
		LCommander::SwitchTarget(fIncStartEdit);
	} else {
		fAutoIncView->Disable();
		fIncStartEdit->SetValue(0);
		fIncValueEdit->SetValue(0);
	}
}

UInt32 IntFieldTypeBoxHandler::GetExtra()
{
	if (GetJFileFieldType() == JFile5::FLDTYPE_AUTOINC)
		return fIncStartEdit->GetValue();
	return 0;
}

UInt32 IntFieldTypeBoxHandler::GetExtra2()
{
	if (GetJFileFieldType() == JFile5::FLDTYPE_AUTOINC)
		return fIncValueEdit->GetValue();
	return 0;
}

void IntFieldTypeBoxHandler::SetupForField(UInt32 fieldType)
{
	fAutoIncRadio->SetValue(fieldType == JFile5::FLDTYPE_AUTOINC);
	fNormalIntRadio->SetValue(fieldType == JFile5::FLDTYPE_INT);		
	EnableAutoIncrementEdit(fieldType == JFile5::FLDTYPE_AUTOINC);
}
