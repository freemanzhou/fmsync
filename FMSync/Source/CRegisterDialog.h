#pragma once

#include "CConduitDialog.h"

class LStaticText;
class LEditText;

class CRegisterDialog : public CConduitDialog
{
public:
							CRegisterDialog(const string& userName, const string& regCode);
					virtual ~CRegisterDialog();
	
	virtual void	FinishCreateSelf();
	
	string			RegCode() const;
	virtual bool	DialogDone();

private:
	string			fUserName;
	string			fRegCode;
	LStaticText*	fUserNameField;
	LEditText*		fRegistrationCodeField;
};

