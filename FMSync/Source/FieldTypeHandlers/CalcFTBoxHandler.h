#include "FieldTypeBoxHandler.h"
#include "FMAE.h"


class LPopupButton;
class LEditText;
class LStaticText;

class CalcFieldTypeBoxHandler : public FieldTypeBoxHandler, public LListener {
public:
	CalcFieldTypeBoxHandler(CEditDBDialog*);
	
	virtual void FinishCreateSelf();
	void SetupForField(UInt32);

	UInt32 GetJFileFieldType();
	virtual UInt32 GetExtra();
	virtual string GetValue1();
	virtual string GetValue2();

protected:
	virtual void		ListenToMessage(
							MessageT		inMessage,
							void			*ioParam);
private:
	int JFileFieldIndexFromMenu(LPopupButton* pb);
	string JFileFieldValueFromMenu(LPopupButton* pb, LEditText* eb);
	static bool IsSetToValue(LPopupButton*);
	void AnnounceInvalidMode(LPopupButton*, int);
	
	void UpdateValueFieldState(map<FMAE::FieldID,string>& theMap, LPopupButton* fieldMenu, LEditText* e, LStaticText* st, int&);
	
	int fLastLegalF1Menu;
	int fLastLegalF2Menu;
	LPopupButton* fField1Menu;
	LPopupButton* fField2Menu;
	LEditText* fValue1Field;
	LEditText* fValue2Field;
	LStaticText* fValue1Label;
	LStaticText* fValue2Label;
	LPopupButton* fCalcOperationMenu;
};
