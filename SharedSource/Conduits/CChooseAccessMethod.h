#pragma once

#include "CConduitDialog.h"

class LRadioButton;

class CChooseAccessMethod : public CConduitDialog
{
public:
							CChooseAccessMethod(bool preferUseLocal);
					virtual ~CChooseAccessMethod();
	
	virtual void	RegisterClasses();
	
	bool UseLocal() const;
	
protected:
	virtual void	UpdateControlState();
	virtual void	FinishCreateSelf();
	virtual bool	DialogDone();

private:
	void			UpdateWindow();
	
	LRadioButton*	fLocal;
	LRadioButton*	fExisting;
	bool			fUseLocal;
	bool			fPreferUseLocal;
};

