#pragma once

#include "CConduitDialog.h"

#include <vector>

class CFieldNamesEdit;

class CEditNamesDialog : public CConduitDialog
{
public:
							CEditNamesDialog(const string& databaseName, const vector<string>& pilotNames, 
								const vector<string>& fmNames);
					virtual ~CEditNamesDialog();
	
	virtual void		FinishCreateSelf();
	
	virtual void		ListenToMessage(
							MessageT		inMessage,
							void			*ioParam);

	virtual void	RegisterClasses();
	
	vector<int>		GetSelection();
	vector<int>		GetOrder();

protected:
	virtual void	UpdateControlState();
	virtual bool	DialogDone();

private:
	void			SetupForDatabase();
	void			ShowTask();
	
	CFieldNamesEdit*	fFieldNamesEditor;
	string				fDatabaseName;
	vector<string>		fPilotNames;
	vector<string>		fFMNames;
};

