#include <LRadioButton.h>

#include "JFile5.h"
#include "PaneUtilities.h"
#include "TimeFTBoxHandler.h"

TimeFieldTypeBoxHandler::TimeFieldTypeBoxHandler(CEditDBDialog* d)
	: FieldTypeBoxHandler(d)
{
}

void TimeFieldTypeBoxHandler::FinishCreateSelf()
{
	FindPane(fView, 'cret', &fCreateTimeRadio);
	FindPane(fView, 'modt', &fModTimeRadio);
	FindPane(fView, 'nort', &fNormalTimeRadio);
}

UInt32 TimeFieldTypeBoxHandler::GetJFileFieldType()
{
	if (fModTimeRadio->GetValue())
		return JFile5::FLDTYPE_MODTIME;
	if (fCreateTimeRadio->GetValue())
		return JFile5::FLDTYPE_AUTOTIME;
	return JFile5::FLDTYPE_TIME;
}

void TimeFieldTypeBoxHandler::SetupForField(UInt32 fieldType)
{
	fCreateTimeRadio->SetValue(fieldType == JFile5::FLDTYPE_AUTOTIME);
	fModTimeRadio->SetValue(fieldType == JFile5::FLDTYPE_MODTIME);
	fNormalTimeRadio->SetValue(fieldType == JFile5::FLDTYPE_TIME);
}
