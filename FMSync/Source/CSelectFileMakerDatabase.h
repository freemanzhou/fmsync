#pragma once

#include "CConduitDialog.h"
#include "CDatabaseName.h"

class LStaticText;
class CStringTable;
class LCheckBox;

class CSelectFileMakerDatabase : public CConduitDialog
{
public:
							CSelectFileMakerDatabase(vector<CDatabaseName>& names);
					virtual ~CSelectFileMakerDatabase();
	
	virtual void	RegisterClasses();
	
	CDatabaseName	DatabaseName() const;
	bool			WasSelected() const;

protected:
	virtual void	UpdateControlState();
	virtual void	FinishCreateSelf();
	virtual bool	DialogDone();

private:
	void			UpdateWindow();
	bool			HasSelection();
	bool			SelectedIndex(int& index);
	vector<string>	DatabaseNamesAsStrings() const;
	
	vector<CDatabaseName> fNames;
	CDatabaseName fSelectedName;
		
	CStringTable*	fList;
};

