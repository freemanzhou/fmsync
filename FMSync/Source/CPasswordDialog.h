#pragma once

#include "CConduitDialog.h"

class LStaticText;
class LEditText;
class LPushButton;

class CPasswordDialog : public CConduitDialog
{
public:
							CPasswordDialog(const string& databaseName);
					virtual ~CPasswordDialog();
	
	virtual void	FinishCreateSelf();
	
	string GetPassword();

	virtual void	UpdateControlState();

private:
	void			UpdateWindow();
	
	string			fName;
	LEditText*		fPassword;
	LStaticText*	fDescription;
	LPushButton*	fOKButton;
};

