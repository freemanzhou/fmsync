#pragma once

#include "CConduitDialog.h"

class LStaticText;
class LCheckBox;

class CEditPopupDialog : public CConduitDialog
{
public:
							CEditPopupDialog(const vector<string>&);
					virtual ~CEditPopupDialog();
	
	virtual void	FinishCreateSelf();
	
	vector<string>	GetValues();

protected:
	virtual bool	DialogDone();
	
	
	LTextEditView*	fText;
	vector<string>	fValues;
};

