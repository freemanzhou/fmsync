#include "FieldTypeBoxHandler.h"

class CheckboxFieldTypeBoxHandler : public FieldTypeBoxHandler {
public:
	CheckboxFieldTypeBoxHandler(CEditDBDialog*);
	
	virtual void FinishCreateSelf();
	void SetupForField(UInt32);

	virtual UInt32 GetJFileFieldType();

private:
	void DoEditPopupValues();
};
