#include "FieldTypeBoxHandler.h"

class LRadioButton;

class IntFieldTypeBoxHandler : public FieldTypeBoxHandler, public LListener {
public:
	IntFieldTypeBoxHandler(CEditDBDialog*);
	
	void FinishCreateSelf();

	UInt32 GetJFileFieldType();
	UInt32 GetExtra();
	UInt32 GetExtra2();
	void SetupForField(UInt32);

protected:
	void ListenToMessage( MessageT inMessage, void* ioParam);

private:
	void EnableAutoIncrementEdit(bool);

	LRadioButton*		fAutoIncRadio;
	LRadioButton*		fNormalIntRadio;
	LEditText*			fIncStartEdit;
	LEditText*			fIncValueEdit;
	LView*				fAutoIncView;
};
