#pragma once

#include "CConduitDialog.h"
#include "CUploadRequest.h"

class LStaticText;
class CStringTable;
class LCheckBox;
class LTextGroupBox;

class CUploadDBDialog : public CConduitDialog
{
public:
							CUploadDBDialog(SInt32 lastTime, const vector<string>& names,
								const vector<CUploadRequest>& req);
					virtual ~CUploadDBDialog();
	
	virtual void	RegisterClasses();
	
	virtual void	ListenToMessage(
							MessageT		inMessage,
							void			*ioParam);
	vector<CUploadRequest> GetUploadRequests() const {return fReqs;}

protected:
	virtual void	UpdateControlState();
	virtual void	FinishCreateSelf();
	virtual bool	DialogDone();

private:
	string			MakeLocalName(const string&);
	void			UpdateWindow();
	bool			HasSelection();
	bool			SelectedIndex(int& index);
	void			SetSelectedIndex(int index);
	void			SelectNone();
	void			SaveSelectedValues();
	void			DoAdd();
	void			DoRemove();
	bool			DoSetLocation(CUploadRequest&);
	void			DoSetChanges();
	void			UpdateControlForSelection(int index, bool hasSelection);
	
	vector<string>	fNames;
	vector<CUploadRequest> fReqs;
	SInt32			fTime;
	int				fIndex;
	bool			fChanged;
		
	CStringTable*	fList;
	LCheckBox*		fChangedBox;
	LPushButton*	fAddButton;
	LPushButton*	fRemoveButton;
	LPushButton*	fSetLocationButton;
	LTextGroupBox*	fOptionsBox;
	LStaticText*	fLocationText;
};

