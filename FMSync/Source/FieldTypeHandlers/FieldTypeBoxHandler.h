#pragma once

#include <smart_ptr.hpp>

class CEditDBDialog;

class FieldTypeBoxHandler {
public:
	FieldTypeBoxHandler(CEditDBDialog*);
	
	virtual void FinishCreateSelf() {};
	virtual void SetupForField(UInt32) = 0;
	virtual void SaveSelectedExtraData() {};
	
	typedef boost::shared_ptr<FieldTypeBoxHandler> Ptr;
	virtual UInt32 GetJFileFieldType() = 0;
	virtual UInt32 GetExtra() {return 0;}
	virtual UInt32 GetExtra2() {return 0;};
	virtual string GetValue1() {return "";}
	virtual string GetValue2() {return "";}

protected:
	CEditDBDialog* fDialog;
	LView* fView;
};
