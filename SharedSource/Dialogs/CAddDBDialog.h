#pragma once

#include "CConduitDialog.h"

class CDatabaseFile;
class LPopupButton;
class LStaticText;
class LListBox;
class LPushButton;

class CAddDBDialog : public CConduitDialog
{
public:
							CAddDBDialog(CDatabaseFile* inDB);
					virtual ~CAddDBDialog();
	
	virtual void		FinishCreateSelf();
	
	virtual void		ListenToMessage(
							MessageT		inMessage,
							void			*ioParam);

	virtual void	UpdateControlState();
	virtual bool	DialogDone();

	virtual void	RegisterClasses();

private:

	enum			{kWindowID = 131};
	void			SetupForDatabase();
	void			UpdateMenus();
	void			UpdateLists();

	void			HandleMove();
	void			HandleMoveAll();
	void			HandleClear();
	void			HandleClearAll();
	void			HandleUp();
	void			HandleDown();
	void			HandleUpOrDown(int delta);
	Boolean			FieldCanBeMoved(int);
	Boolean			FieldCanBeCleared(int);
	Boolean			AnyFieldCanBeMoved();
	Boolean			AnyFieldCanBeCleared();
	
	vector<string>		fFieldNames;
	vector<int>			fFieldIDs;
	vector<int>			fFieldsToMove;
	map<int, string>	fFieldNameMap;
	//int					fKeyFieldID;
	//int					fPalmID;
	int					fArchiveID;
	CMenuHandler*		fKeyFieldMenuH;
	CMenuHandler*		fPalmMenuH;
	CMenuHandler*		fArchiveMenuH;
	CDatabaseFile* 		fDatabase;
	LStaticText*		fDatabaseName;
	LListBox*			fAllFieldListBox;
	LListBox*			fPalmFieldListBox;
	LPushButton*		fMoveButton;
	LPushButton*		fMoveAllButton;
	LPushButton*		fClearAllButton;
	LPushButton*		fUpButton;
	LPushButton*		fDownButton;
};

