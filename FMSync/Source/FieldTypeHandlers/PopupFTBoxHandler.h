#include <LListener.h>

#include "FieldTypeBoxHandler.h"

class LRadioButton;

class PopupFieldTypeBoxHandler : public FieldTypeBoxHandler, public LListener {
public:
	PopupFieldTypeBoxHandler(CEditDBDialog* d);
	
	virtual void FinishCreateSelf();
	void SetupForField(UInt32);

	UInt32 GetJFileFieldType();

protected:
	void ListenToMessage( MessageT inMessage, void* ioParam);

private:
	void DoEditPopupValues();

	LRadioButton* fNormalListRadio;
	LRadioButton* fMultiListRadio;
};
