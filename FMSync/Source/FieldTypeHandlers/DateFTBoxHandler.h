#include "FieldTypeBoxHandler.h"

class LRadioButton;

class DateFieldTypeBoxHandler : public FieldTypeBoxHandler {
public:
	DateFieldTypeBoxHandler(CEditDBDialog*);
	
	void FinishCreateSelf();

	UInt32 GetJFileFieldType();
	UInt32 GetExtra() {return 0;}
	void SetupForField(UInt32);

private:
	LRadioButton*		fCreateDateRadio;
	LRadioButton*		fModDateRadio;
	LRadioButton*		fNormalDateRadio;
};
