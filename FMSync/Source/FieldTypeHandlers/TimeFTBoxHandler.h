#include "FieldTypeBoxHandler.h"

class LRadioButton;

class TimeFieldTypeBoxHandler : public FieldTypeBoxHandler {
public:
	TimeFieldTypeBoxHandler(CEditDBDialog*);
	
	void FinishCreateSelf();
	void SetupForField(UInt32);

	UInt32 GetJFileFieldType();

private:
	LRadioButton*		fCreateTimeRadio;
	LRadioButton*		fModTimeRadio;
	LRadioButton*		fNormalTimeRadio;
};
