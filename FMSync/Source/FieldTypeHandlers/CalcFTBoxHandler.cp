#include <LPopupButton.h>
#include <LEditText.h>
#include <LStaticText.h>

#include "CalcFTBoxHandler.h"
#include "LErrorMsg.h"
#include "CEditDBDialog.h"
#include "ErrorStrings.h"
#include "CEnterValueDialog.h"
#include "WriteJFile5.h"
#include "JFile5.h"
#include "PaneUtilities.h"
#include "Str255.h"
#include "Utilities.h"

#ifdef JFILE_CALCULATIONS
const UInt32 kCalcValue = -1;

CalcFieldTypeBoxHandler::CalcFieldTypeBoxHandler(CEditDBDialog* d)
	: FieldTypeBoxHandler(d)
{
}

void CalcFieldTypeBoxHandler::FinishCreateSelf()
{
	FindPane(fView, 'fie1', &fField1Menu);
	fField1Menu->AddListener(this);
	FindPane(fView, 'fie2', &fField2Menu);
	fField2Menu->AddListener(this);
	FindPane(fView, 'oper', &fCalcOperationMenu);
	
	FindPane(fView, 'val1', &fValue1Field);
	FindPane(fView, 'vla1', &fValue1Label);
	FindPane(fView, 'val2', &fValue2Field);
	FindPane(fView, 'vla2', &fValue2Label);
}

int CalcFieldTypeBoxHandler::JFileFieldIndexFromMenu(LPopupButton* pb)
{
	UInt32 v = pb->GetValue();
	if (v <= fDialog->fFieldIDs.size())
		return v - 1;
	return kCalcValue;
}

string CalcFieldTypeBoxHandler::JFileFieldValueFromMenu(LPopupButton* pb, LEditText* eb)
{
	if (pb->GetValue() < pb->GetMaxValue())
		return "";
	
	return GetTextFromEditBox(eb);
}

UInt32 CalcFieldTypeBoxHandler::GetJFileFieldType()
{
	if (JFileFieldIndexFromMenu(fField1Menu) == kCalcValue)
		return JFile5::FLDTYPE_CALC_V1;
	
	if (JFileFieldIndexFromMenu(fField2Menu) == kCalcValue)
		return JFile5::FLDTYPE_CALC_V2;
	
	return JFile5::FLDTYPE_CALC;
}

UInt32 CalcFieldTypeBoxHandler::GetExtra()
{
	UInt32 f1 = JFileFieldIndexFromMenu(fField1Menu);
	UInt32 f2 = JFileFieldIndexFromMenu(fField2Menu);
	ThrowIf_(f1 == kCalcValue && f2 == kCalcValue);
	UInt32 oper = fCalcOperationMenu->GetValue();
	return JFile5::CWriteJFile5::ExtraDataForCalculatedFields(f1, f2, oper);
}

string CalcFieldTypeBoxHandler::GetValue1()
{
	return JFileFieldValueFromMenu(fField1Menu, fValue1Field);
}

string CalcFieldTypeBoxHandler::GetValue2()
{
	return JFileFieldValueFromMenu(fField2Menu, fValue2Field);
}

inline UInt32 GetCalcField1(UInt32 extraValue)
{
	return extraValue >> 24;
}

inline UInt32 GetCalcField2(UInt32 extraValue)
{
	return (extraValue >> 16) & 0xff;
}

inline UInt32 GetCalcOperation(UInt32 extraValue)
{
	return extraValue & 0x0000ffff;
}

static int JFileToFieldMenuValue(int fieldType, int specialCase, int fieldNumber, int fieldCount)
{
	if (fieldType == specialCase)
		return fieldCount + 2;
	return fieldNumber+1; 
}

static void AddOther(LPopupButton* button)
{
	button->AppendMenu("\p-", false);
	button->AppendMenu("\pValue", true);
}

void CalcFieldTypeBoxHandler::SetupForField(UInt32 fieldType)
{
	FMAE::FieldID fieldID(fDialog->GetLastSelectedField());
	UInt32 extra(fDialog->fFieldAttributes.fFieldExtra[fieldID]);
	fCalcOperationMenu->SetValue(GetCalcOperation(extra));
	vector<string> fieldNames(MakeOrderedList(fDialog->fFieldAttributes.fFieldNames, fDialog->fFieldIDs));
	DisableMessage<LPopupButton> df1(fField1Menu);
	DisableMessage<LPopupButton> df2(fField2Menu);
	SetPopupButton(fField1Menu, fieldNames);
	AddOther(fField1Menu);
	SetPopupButton(fField2Menu, fieldNames);
	AddOther(fField2Menu);
	fLastLegalF1Menu = JFileToFieldMenuValue(fieldType, JFile5::FLDTYPE_CALC_V1, GetCalcField1(extra), fDialog->fFieldIDs.size());
	fField1Menu->SetValue(fLastLegalF1Menu);
	fLastLegalF2Menu = JFileToFieldMenuValue(fieldType, JFile5::FLDTYPE_CALC_V2, GetCalcField2(extra), fDialog->fFieldIDs.size());
	fField2Menu->SetValue(fLastLegalF2Menu);

	UpdateValueFieldState(fDialog->fFieldAttributes.fFieldValue1, fField1Menu, fValue1Field, fValue1Label, fLastLegalF1Menu);
	UpdateValueFieldState(fDialog->fFieldAttributes.fFieldValue2, fField2Menu, fValue2Field, fValue2Label, fLastLegalF2Menu);
}

void CalcFieldTypeBoxHandler::ListenToMessage(MessageT inMessage, void* param)
{
	if (inMessage == fField1Menu->GetValueMessage()) {
		if (IsSetToValue(fField2Menu)) {
			AnnounceInvalidMode(fField1Menu, fLastLegalF1Menu);
		} else {
			UpdateValueFieldState(fDialog->fFieldAttributes.fFieldValue1, fField1Menu, fValue1Field, fValue1Label, fLastLegalF1Menu);
		}
	} else if (inMessage == fField2Menu->GetValueMessage()) {
		if (IsSetToValue(fField1Menu)) {
			AnnounceInvalidMode(fField2Menu, fLastLegalF2Menu);
		} else {
			UpdateValueFieldState(fDialog->fFieldAttributes.fFieldValue2, fField2Menu, fValue2Field, fValue2Label, fLastLegalF2Menu);
		}
	}
}

void CalcFieldTypeBoxHandler::UpdateValueFieldState(map<FMAE::FieldID,string>& theMap, LPopupButton* fieldMenu, LEditText* e, LStaticText* st, int& lastLegal)
{
	FMAE::FieldID fieldID(fDialog->GetLastSelectedField());
	FillEditBox(e, theMap[fieldID]);
	if (IsSetToValue(fieldMenu)) {
		e->Enable();
		st->Enable();
	} else {
		e->Disable();
		st->Disable();
	}
	lastLegal = fieldMenu->GetValue();
}

bool CalcFieldTypeBoxHandler::IsSetToValue(LPopupButton* fieldMenu)
{
	return fieldMenu->GetValue() == fieldMenu->GetMaxValue();
}

void CalcFieldTypeBoxHandler::AnnounceInvalidMode(LPopupButton* pb, int lastLegalValue)
{
	LErrorMsg::AlertWithMessage(kAlertStopAlert, LoadString(kFMJErrorStrings, kCantHaveTwoValues));
	DisableMessage<LPopupButton> df1(pb);
	pb->SetValue(lastLegalValue);
}

#endif