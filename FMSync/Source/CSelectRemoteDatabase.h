#pragma once

#include "CConduitDialog.h"

class LStaticText;
class CStringTable;
class LCheckBox;

class CSelectRemoteDatabase : public CConduitDialog
{
public:
							CSelectRemoteDatabase(SInt32 lastTime, const vector<string>& names);
					virtual ~CSelectRemoteDatabase();
	
	virtual void	RegisterClasses();
	
	string			DatabaseName() const;
	bool			WasSelected() const;

protected:
	virtual void	UpdateControlState();
	virtual void	FinishCreateSelf();
	virtual bool	DialogDone();

private:
	void			UpdateWindow();
	void			UpdateTime();
	bool			HasSelection();
	bool			SelectedIndex(int& index);
	
	vector<string>	fNames;
	SInt32			fTime;
	string			fSelectedName;
		
	LStaticText*	fUpdatedNote;
	CStringTable*	fList;
	LCheckBox*		fChangedBox;
};

