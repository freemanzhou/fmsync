#include <LRadioButton.h>

#include "JFile5.h"
#include "PaneUtilities.h"
#include "DateFTBoxHandler.h"

DateFieldTypeBoxHandler::DateFieldTypeBoxHandler(CEditDBDialog* d)
	: FieldTypeBoxHandler(d)
{
}

void DateFieldTypeBoxHandler::FinishCreateSelf()
{
	FindPane(fView, 'crea', &fCreateDateRadio);
	FindPane(fView, 'modi', &fModDateRadio);
	FindPane(fView, 'norm', &fNormalDateRadio);
}

UInt32 DateFieldTypeBoxHandler::GetJFileFieldType()
{
	if (fModDateRadio->GetValue())
		return JFile5::FLDTYPE_MODDATE;
	if (fCreateDateRadio->GetValue())
		return JFile5::FLDTYPE_AUTODATE;
	return JFile5::FLDTYPE_DATE;
}

void DateFieldTypeBoxHandler::SetupForField(UInt32 fieldType)
{
	fCreateDateRadio->SetValue(fieldType == JFile5::FLDTYPE_AUTODATE);
	fModDateRadio->SetValue(fieldType == JFile5::FLDTYPE_MODDATE);
	fNormalDateRadio->SetValue(fieldType == JFile5::FLDTYPE_DATE);
}
