#pragma once

#include <LCommander.h>
#include <LListener.h>
#include <vector>

#include "PaneUtilities.h"

class LPeriodical;
class LPushButton;
class CSyncIdler;

class CConduitDialog : public LCommander, public LListener
{
public:
							CConduitDialog(ResIDT windowID);
					virtual ~CConduitDialog();

static AEIdleUPP	GetIdleProc();

	void			SetIdler(CSyncIdler* idler);
	
	void			ShowWindow();
	Boolean			DoDialog();
	Boolean			DoDialogOnce();
	virtual void	ListenToMessage(
							MessageT		inMessage,
							void			*ioParam);

	virtual void	RegisterClasses();
	void			SetWindowTitle(const string& windowTitle);
	static void		AdjustCursor(const EventRecord&	inMacEvent);
	static void		UpdateOtherWindows();
	static pascal Boolean	AEIdleProc(EventRecord *eventRecord, 
								long *sleepTime, RgnHandle *mouseRegion);
	
	void			SetDefaultButton(LPushButton* thisButton);
	static Boolean			HandleEvent(const EventRecord&);
	
	LView*			GetView();

protected:

	virtual void	FinishCreateSelf();
	virtual bool	DialogDone();
		// returns true if the dialog is ready to be closed
	virtual void	UpdateControlState();

	CConduitDialog* fOldCurrent;
	ResIDT			fWindowID;
	WindowPtr		fWindow;
	LGrafPortView*	fView;
	LPushButton*	fOkayButton;
	LControl*		fCancelButton;
	LPushButton*	fDefaultButton;
	Boolean			fQuit;
	Boolean			fCanceled;
	static bool		gBusy;
	static StRegion	gMouseRgn;
	static CConduitDialog* gCurrentDialog;

private:

	void					SafeUpdateControlState();
	void					SaveAndDisableMenus();
	void					Activate();
	void					Deactivate();
	static Boolean			HandleMouseDown(const EventRecord &inMacEvent);
	static void				HandleKeyDown(const EventRecord &inMacEvent);
	static void				UpdateMenus();
	static void				HandleMenuSelected(long menuSelected);
	static void				ClickMenuBar(const EventRecord& inMacEvent);
	
	AEIdleUPP		fOldIdleProc;
	CSyncIdler*		fIdler;
	static int		gEditMenuID;
};

class LMenuController;

class CMenuHandler {
public:
	CMenuHandler(LMenuController* theController, 
		const vector<int>& menuIDs, int numberOfExtras = 0);
	
	
	void			Enable(int fieldID);
	void			Disable(int fieldID);

private:
	int				fNumberOfExtras;
	map<int, int>	fIndexes;
	MenuHandle		fMenu;
};

