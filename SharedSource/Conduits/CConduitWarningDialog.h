#pragma once

#include "CConduitDialog.h"

class LStaticText;
class LCheckBox;

class CConduitWarningDialog : public CConduitDialog
{
public:
							CConduitWarningDialog(const string& title, const string& description);
					virtual ~CConduitWarningDialog();
	
	virtual void	FinishCreateSelf();
	
	bool			WasDisabled() const {return fWasDisabled;}

protected:
	virtual bool	DialogDone();

private:
	void			UpdateWindow();
	
	string			fTitle;
	string			fDesc;
	LStaticText*	fTitleText;
	LStaticText*	fDescText;
	LCheckBox*		fDisable;
	bool			fWasDisabled;
};

