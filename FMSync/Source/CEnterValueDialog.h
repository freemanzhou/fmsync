#pragma once

#include "CConduitDialog.h"

class LEditText;
class LPushButton;

class CEnterValueDialog : public CConduitDialog
{
public:
							CEnterValueDialog(const string& initalValue);
					virtual ~CEnterValueDialog();
	
	virtual void	FinishCreateSelf();
	
	string GetValue() const;

	virtual void	UpdateControlState();

private:
	void			UpdateWindow();
	virtual bool	DialogDone();
	
	string			fValue;
	LEditText*		fValueField;
	LPushButton*	fOKButton;
};

