#pragma once

#include "CConduitDialog.h"
#include "CDataTask.h"
#include "CDatabaseFilePtr.h"
#include "CFieldIDList.h"
#include "FMDatabasePtr.h"

#include <vector>

class CDatabaseFile;
class LPopupButton;
class LPopupGroupBox;
class LMultiPanelView;
class LCheckBox;
class LStaticText;
class CStringTable;
class LBevelButton;
class LRadioGroupView;
class CTaskEdit;
class LSlider;

class CDataTasksDialog : public CConduitDialog
{
public:
							CDataTasksDialog(CDatabaseFilePtr inDB, 
								FileMaker::DatabasePtr fmDB, vector<CDataTask> &theTasks,
								const CFieldIDList& existingLocalFields,
								const vector<int>& existingLocalRepeats);
					virtual ~CDataTasksDialog();
	
	virtual void		FinishCreateSelf();
	
	virtual void		ListenToMessage(
							MessageT		inMessage,
							void			*ioParam);

	virtual void	RegisterClasses();

protected:
	virtual void	UpdateControlState();
	virtual bool	DialogDone();

private:
	void			SetupForDatabase();
	void			NextTask(int delta);
	void			ShowTask();
	void			UpdateCurrentTask();
	void			DoUseForAll();
	
	CDataTask&	CurrentTask() {return fTasks[fConflicts[fCurrentTaskIndex]];}
	
	vector<CDataTask>&	fTasks;
	vector<string>		fFieldNames;
	vector<int>			fConflicts;
	CFieldIDList		fExistingLocalFields;
	vector<int>			fExistingLocalRepeats;
	CDatabaseFilePtr	fDatabase;
	FileMaker::DatabasePtr	fFileMakerDatabase;
	LMultiPanelView*	fMPV;
	CTaskEdit*			fTasksEditor;
	LBevelButton*		fPrevious;
	LBevelButton*		fNext;
	LStaticText*		fTaskCountLabel;
	LPopupGroupBox*		fActionBox;
	int					fCurrentTaskIndex;
	map<int,bool>		fViewedMap;
};

