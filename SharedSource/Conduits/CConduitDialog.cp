#include "CConduitDialog.h"
#include "StAppleEvent.h"
#include "Str255.h"
#include "Str255.h"
#include "syncmgr.h"
#include "CSyncIdler.h"
#include <Threads.h>
#include <UControlRegistry.h>
#include <URegistrar.h>
#include <LPushButton.h>
#include <LSeparatorLine.h>
#include <LScrollerView.h>
#include <LMultiPanelView.h>
#include <LPlacard.h>
#include <UControlRegistry.h>
#include <LMenuController.h>
#include <LGASeparatorImp.h>
#include <LGATextGroupBoxImp.h>
#include <LAMControlImp.h>
#include <LAMTrackActionImp.h>
#include <LTextGroupBox.h>
#include <LTextEditView.h>
#include <UAttachments.h>
#include <UReanimator.h>
#include <LRadioGroupView.h>
#include <LPopupButton.h>
#include <LPicture.h>
#include <LStaticText.h>
#include <LCheckBox.h>
#include <LBevelButton.h>
#include <LPopupGroupBox.h>
#include <LAMPopupGroupBoxImp.h>
#include <LAMTextGroupBoxImp.h>
#include <LAMStaticTextImp.h>
#include <LGAStaticTextImp.h>
#include <LGABevelButtonImp.h>
#include <LAMBevelButtonImp.h>
#include <LAMPopupButtonImp.h>
#include <LGAPopupButtonImp.h>
#include <LAMControlImp.h>
#include <LGACheckBoxImp.h>
#include <LGASliderImp.h>
#include <LTable.h>
#include <LTableView.h>
#include <LActiveScroller.h>
#include <UAttachments.h>
#include <LEditText.h>
#include <LTabsControl.h>
#include <LGAEditTextImp.h>
#include <UGAColorRamp.h>
#include <LAMEditTextImp.h>
#include <map>
#include "CStringTable.h"
#include "CTaskEdit.h"
#include "CFieldNamesEdit.h"

typedef map<WindowPtr, CConduitDialog*> CDialogMap;

CDialogMap fDialogs;
bool CConduitDialog::gBusy = false;
int CConduitDialog::gEditMenuID = 0;
CConduitDialog* CConduitDialog::gCurrentDialog = 0;
static StRegion gUpdateRgn;

const int kSleepTime = 12;

static Boolean CheckForAbort()
{
	EventRecord eventRecord;
	
	if (EventAvail(keyDownMask, &eventRecord)) {
 		return (eventRecord.message & keyCodeMask) == vkey_Escape;	
	}
	return false;
}

pascal Boolean 
CConduitDialog::AEIdleProc(EventRecord *eventRecord, long *sleepTime, RgnHandle *mouseRegion)
{
	static bool firstTime = true;
	if (firstTime) {
		*sleepTime = kSleepTime;
		*mouseRegion = 0;
		firstTime = false;
	}
#ifdef _DEBUG
	if (CheckForAbort())
		return true;
#endif
	return CConduitDialog::HandleEvent(*eventRecord);
}

AEIdleUPP 
CConduitDialog::GetIdleProc()
{
	static	AEIdleUPP idleProc = NewAEIdleUPP(CConduitDialog::AEIdleProc);
	return idleProc;
}

CConduitDialog::CConduitDialog(ResIDT windowID)
	: fWindow(0), fView(0), fQuit(false), fCanceled(false), 
	  fWindowID(windowID), fOldIdleProc(0), fIdler(0), fDefaultButton(0)
{
	fOldIdleProc = StAppleEvent::SetDefaultIdleProc(GetIdleProc());
	gBusy = true;
	AddAttachment(new LUndoer());
	fOldCurrent = gCurrentDialog;
	gCurrentDialog = this;
}

CConduitDialog::~CConduitDialog()
{
	fView->OutOfFocus(0);
	delete fView;
	if (fWindow) {
		CDialogMap::iterator f = fDialogs.find(fWindow);
		if (f != fDialogs.end()) {
			fDialogs.erase(f);
		}
		if (fOldCurrent == 0) {
			PaintBehind(fWindow, gUpdateRgn);
			SetEmptyRgn(gUpdateRgn);
		}
		DisposeWindow(fWindow);
	}
	
	StAppleEvent::SetDefaultIdleProc(fOldIdleProc);
	
	delete fIdler;

	gCurrentDialog = fOldCurrent;
	//UpdateOtherWindows();
}

void
CConduitDialog::UpdateOtherWindows()
{
}

void
CConduitDialog::ListenToMessage(MessageT inMessage, void*)
{
	if (inMessage == msg_OK) {
		fQuit = true;
	} else if (inMessage == msg_Cancel) {
		fQuit = true;
		fCanceled = true;
	}
}

void
CConduitDialog::RegisterClasses()
{
	static Boolean firstRun = true;
	if (firstRun) {
		firstRun = false;
		RegisterClass_(LGrafPortView);
		UControlRegistry::RegisterClassicControls();
		RegisterClass_(LView);
		RegisterClass_(LPicture);
		RegisterClass_(LSeparatorLine);
		RegisterClass_(LTextGroupBox);
		RegisterClass_(LRadioGroupView);
		RegisterClass_(LPopupButton);
		RegisterClass_(LStaticText);
		RegisterClass_(LCheckBox);
		RegisterClass_(CStringTable);
		RegisterClass_(LActiveScroller);
		RegisterClass_(LBevelButton);
		RegisterClass_(CTaskEdit);
		RegisterClass_(CFieldNamesEdit);
		RegisterClass_(LTableView);
		RegisterClass_(LPlacard);
		RegisterClass_(LEditText);
		RegisterClass_(LTabsControl);
		RegisterClass_(LMultiPanelView);
		RegisterClass_(LPopupGroupBox);
		RegisterClass_(LTextEditView);
		RegisterClass_(LColorEraseAttachment);
		RegisterClass_(LTabGroupView);
		RegisterClassID_(LAMStaticTextImp, LStaticText::imp_class_ID);
		RegisterClassID_(LAMPopupButtonImp, LPopupButton::imp_class_ID);
		RegisterClassID_(LAMControlImp, LCheckBox::imp_class_ID);
		RegisterClassID_(LAMControlImp, LSeparatorLine::imp_class_ID);
		RegisterClassID_(LAMBevelButtonImp, LBevelButton::imp_class_ID);		
		RegisterClassID_(LAMPopupGroupBoxImp, LPopupGroupBox::imp_class_ID);
		RegisterClassID_(LAMTextGroupBoxImp, LTextGroupBox::imp_class_ID);
		RegisterClassID_(LAMControlImp, LTabsControl::imp_class_ID);
		RegisterClassID_(LAMEditTextImp, LEditText::imp_class_ID);
		RegisterClassID_(LAMControlImp,	LPlacard::imp_class_ID);
	}
}

typedef struct {
	Rect bounds;
	short wdefID;
} **WINDPokerH;

void
CConduitDialog::SetDefaultButton(LPushButton* thisButton)
{
	if (fDefaultButton)
		fDefaultButton->SetDefaultButton(false);
	
	fDefaultButton = thisButton;
	
	if (fDefaultButton)
		fDefaultButton->SetDefaultButton(true);
}

void
CConduitDialog::ShowWindow()
{
	if (fWindow == 0) {
		SaveAndDisableMenus();
		UCursor::SetWatch();		// Default cursor is the arrow	
		gBusy = true;
		RegisterClasses();
		StResource wind('WIND', fWindowID);
		
		WINDPokerH h = (WINDPokerH)Handle(wind);
		if ((**h).wdefID == movableDBoxProc) {
			(**h).wdefID = kWindowMovableModalDialogProc;
		}
		
		fWindow = GetNewCWindow(fWindowID, 0, kFirstWindowOfClass);
		ThrowIfNil_(fWindow);
		fDialogs[fWindow] = this;
		SetPort(GetWindowPort(fWindow));
		
		fView = LGrafPortView::CreateGrafPortView(fWindowID, this);
		ThrowIfNil_(fView);
		UReanimator::LinkListenerToBroadcasters(this, fView, fWindowID);
		FinishCreateSelf();
		RGBColor bc = UGAColorRamp::GetColor(colorRamp_Gray1);
		RGBColor fc = UGAColorRamp::GetColor(colorRamp_Black);
		fView->SetForeAndBackColors(&fc, &bc);
		fView->Activate();
	
		SDimension16 frameSize;
		fView->GetFrameSize(frameSize);
		fOkayButton = dynamic_cast<LPushButton*>(fView->FindPaneByID('Okay'));
		fCancelButton = dynamic_cast<LControl*>(fView->FindPaneByID('Canc'));
		if (fOkayButton && fOkayButton->IsDefaultButton())
			fDefaultButton = fOkayButton;
		::SizeWindow(fWindow, frameSize.width, frameSize.height, true);
		::ShowWindow(fWindow);
		::SelectWindow(fWindow);
		UpdateMenus();
	}
}
	
void
CConduitDialog::HandleKeyDown(const EventRecord &inMacEvent)
{
	WindowPtr	frontWindowP = ::FrontWindow();
	CConduitDialog* dp = fDialogs[frontWindowP];
	if (dp) {
		char theChar = (inMacEvent.message & charCodeMask);
		bool couldBeMenu = ((inMacEvent.modifiers & cmdKey) != 0);
		int menuSelected = 0;
		if (couldBeMenu)
			menuSelected = MenuKey (theChar);
		if (couldBeMenu && HiWord(menuSelected) != 0) {
			HandleMenuSelected(menuSelected);
		} else if (dp->fDefaultButton && (theChar == char_Return || theChar == char_Enter)) {
			dp->fDefaultButton->SimulateHotSpotClick(kControlButtonPart);
		} else if ( (inMacEvent.message & keyCodeMask) == vkey_Escape ) {
			if (dp->fCancelButton)
				dp->fCancelButton->SimulateHotSpotClick(kControlButtonPart);
			else if (dp->fOkayButton)
				dp->fOkayButton->SimulateHotSpotClick(kControlButtonPart);
		} else {
			dp->fView->DoKeyPress(inMacEvent);
		}
		if (dp->fIdler)
			dp->fIdler->ResetTimeout();
	}
;
}

Boolean
CConduitDialog::HandleMouseDown(const EventRecord &inMacEvent)
{
	Boolean shouldQuit = false;
	WindowPtr	macWindowP;
	CConduitDialog* dp;
	SInt16		thePart = ::FindWindow(inMacEvent.where, &macWindowP);
	
	switch (thePart) {
		case inMenuBar:
			ClickMenuBar(inMacEvent);
			break;
			
		case inSysWindow:
			//::SystemClick(&inMacEvent, macWindowP);
			break;
			
		case inDesk:			// Only happens when a truly modal window
			::SysBeep(1);		//   is in front and the user clicks on
			break;				//   the desktop
			
		case inContent:
			dp = fDialogs[macWindowP];
			if (dp) {
				WindowPtr	frontWindowP = ::FrontWindow();
				if (frontWindowP == macWindowP) {
					Point portMouse = inMacEvent.where;
					dp->fView->GlobalToPortPoint(portMouse);
					dp->fView->AdjustMouse(portMouse, inMacEvent, gMouseRgn);
					dp->fView->ClickInContent(inMacEvent);
					shouldQuit = dp->fQuit;
				} else {
					::SysBeep(1);		//   is in front and the user clicks on
				}
				if (dp->fIdler)
					dp->fIdler->ResetTimeout();
			}
			break;

			// Fall Thru
		case inDrag:
			dp = fDialogs[macWindowP];
			if (dp) {
				WindowPtr	frontWindowP = ::FrontWindow();
				if (frontWindowP == macWindowP) {
					Rect	dragRect;
					GetRegionBounds(GetGrayRgn(), &dragRect);
					::InsetRect(&dragRect, 4, 4);
					::DragWindow(macWindowP, inMacEvent.where, &dragRect);
				} else {
					::SysBeep(1);		//   is in front and the user clicks on
				}
				if (dp->fIdler)
					dp->fIdler->ResetTimeout();
			}
			break;
			
		case inGrow:
		case inGoAway:
		case inZoomIn:
		case inZoomOut:
			break;
	}
	return shouldQuit;
}

StRegion CConduitDialog::gMouseRgn;

void
CConduitDialog::AdjustCursor(const EventRecord&	inMacEvent)
{
									// Find out where the mouse is
	WindowPtr	macWindowP;
	Point		globalMouse = inMacEvent.where;
	::FindWindow(globalMouse, &macWindowP);

	Boolean		useArrow = true;	// Assume cursor will be the Arrow
	
	if (macWindowP != nil) {		// Mouse is inside a Window
		CConduitDialog* dp;
		dp = fDialogs[macWindowP];
		if ((dp != nil) &&
			dp->fView->IsActive() &&
			dp->fView->IsEnabled()) {
			if (gBusy) {
				UCursor::SetWatch();		// Default cursor is the arrow	
			} else {
										// Mouse is inside an active and enabled
										//   PowerPlant Window. Let the Window
										//   adjust the cursor shape.
										
										// Get mouse location in Port coords
				Point	portMouse = globalMouse;
				dp->fView->GlobalToPortPoint(portMouse);
				
				dp->fView->AdjustMouse(portMouse, inMacEvent, gMouseRgn);
			}
			useArrow = false;
		}
	}
	
	if (useArrow) {					// Window didn't set the cursor
		if (gBusy)
			UCursor::SetWatch();		// Default cursor is the watch
		else	
			UCursor::SetArrow();		// Default cursor is the arrow	
	}	
	
		// Rather than trying to calculate an accurate mouse region,
		// we define a region that contains just the one pixel where
		// the mouse is located. This is quick, and handles the common
		// case where this application is in the foreground but the user
		// isn't doing anything. However, any mouse movement will generate
		// a mouse-moved event.
	::SetRectRgn(gMouseRgn, globalMouse.h, globalMouse.v,
						(SInt16) (globalMouse.h + 1),
						(SInt16) (globalMouse.v + 1));
}

Boolean
CConduitDialog::HandleEvent(const EventRecord& theEvent)
{
	EventRecord macEvent;
	if (gBusy) {
		UCursor::SetWatch();		// Default cursor is the arrow	
	} else {
		::EventAvail(0, &macEvent);
		AdjustCursor(macEvent);
	}

	WindowPtr wp;
	Boolean shouldQuit = false;
	CConduitDialog* dp;
	switch(theEvent.what) {
	case keyDown:
		HandleKeyDown(theEvent);
		UpdateMenus();
		break;
		
	case mouseDown:
		shouldQuit = HandleMouseDown(theEvent);
		UpdateMenus();
		break;
		
	case updateEvt:
		wp = (WindowPtr)theEvent.message;
		dp = fDialogs[wp];
		if (dp != 0) {
			GrafPtr		originalPort = UQDGlobals::GetCurrentPort();
			
			::SetPort(GetWindowPort(wp));
			::SetOrigin(0,0);

			dp->fView->ApplyForeAndBackColors();
			dp->fView->UpdatePort();
			
			::SetPort(originalPort);
		} else {
			StRegion rh;
			GetWindowRegion(wp, kWindowUpdateRgn, rh);
			UnionRgn(rh, gUpdateRgn, gUpdateRgn);
			::BeginUpdate(wp);
			::EndUpdate(wp);
		}
		break;

	case activateEvt:
		wp = (WindowPtr)theEvent.message;
		dp = fDialogs[wp];
		if (dp) {
			if (theEvent.modifiers & activeFlag) {
				dp->Activate();
			} else {
				dp->Deactivate();
			}
		}
		UpdateMenus();
		break;

	case osEvt:
		UInt8	osEvtFlag = (UInt8) (((UInt32) theEvent.message) >> 24);
		
		if (osEvtFlag == mouseMovedMessage) {
		} else if (osEvtFlag == suspendResumeMessage) {
			WindowPtr	macWindowP = ::FrontWindow();
			CConduitDialog* dp = fDialogs[macWindowP];
			if (dp) {
				if (theEvent.message & resumeFlag) {
					dp->Activate();
				} else {
					dp->Deactivate();
				}
			}
		}
	}
	return shouldQuit;
}

Boolean
CConduitDialog::DoDialogOnce()
{
	EventRecord theEvent;
	if (WaitNextEvent(everyEvent, &theEvent, kSleepTime, gMouseRgn)) {
		HandleEvent(theEvent);
		SafeUpdateControlState();
	}
	if (fIdler)
		fIdler->SpendTime(theEvent);

	return fCanceled;
}

Boolean
CConduitDialog::DoDialog()
{
	bool wasBusy = gBusy;
	ShowWindow();
	while (!fQuit) {
		DoDialogOnce();
		if (fQuit && !fCanceled) {
			gBusy = true;
			fQuit = DialogDone();
		}
		gBusy = false;
	}
	gBusy = wasBusy;
	return fCanceled;
}

void
CConduitDialog::Activate()
{
	if (fWindow)
		::SelectWindow(fWindow);

	if (fView)
		fView->Activate();
}

void
CConduitDialog::Deactivate()
{
	if (fView)
		fView->Deactivate();

	if (fWindow)
		::HiliteWindow(fWindow, false);
}

void
CConduitDialog::SafeUpdateControlState()
{
	UpdateControlState();
}

void
CConduitDialog::UpdateControlState()
{
}

bool
CConduitDialog::DialogDone()
{
	return true;
}

void
CConduitDialog::FinishCreateSelf()
{
}

void
CConduitDialog::SetWindowTitle(const string& wTitle)
{
	ThrowIfNil_(fWindow);
	SetWTitle(fWindow, AsStr255(wTitle));
}

void
CConduitDialog::SaveAndDisableMenus()
{
#if 0
	Handle h = GetIndResource('MBAR', 1);
	if (h != 0) {
		DetachResource(h);
		StHandleBlock block(h);
		::MoveHHi(h);
		::HLock(h);
		short *p = (short*)*h;
		int menuCount = *p;
		++p;
		for (int i = 0; i < menuCount; i += 1) {
			MenuHandle mh = GetMenu(p[i]);
			if (mh)
				::DisableItem(mh, 0);
			if (i == 2)
				gEditMenuID = p[i];
		}
		::DrawMenuBar();
	}
#endif
}

const int kEditMenuItems = 7;
const CommandT kEditMenuCommands[kEditMenuItems] = {
	cmd_Undo,
	cmd_Nothing,
	cmd_Cut,
	cmd_Copy,
	cmd_Paste,
	cmd_Clear,
	cmd_SelectAll
};

void
CConduitDialog::UpdateMenus()
{
#if 0
	LCommander	*theTarget = LCommander::GetTarget();
	if (theTarget != nil) {
		Boolean		isEnabled;
		Boolean		usesMark;
		UInt16		mark;
		Str255		itemName;	
		MenuHandle mh = GetMenu(gEditMenuID);
		if (mh) {
			::EnableItem(mh, 0);
			for (int i = 0; i < kEditMenuItems; i+=1 ) {
				itemName[0] = 0;
				CommandT command = kEditMenuCommands[i];
				if (command != cmd_Nothing) {
					theTarget->ProcessCommandStatus(command, isEnabled,
											usesMark, mark, itemName);
				} else {
					isEnabled = false;
				}
				if (isEnabled) {
					::EnableItem(mh, i+1);
				} else {
					::DisableItem(mh, i+1);
				}
				if (itemName[0] > 0) {
					::SetMenuItemText(mh, i+1, itemName);
				}
			}
		}
	}
	::InvalMenuBar();
#endif
}

void
CConduitDialog::ClickMenuBar(const EventRecord& inMacEvent)
{
	HandleMenuSelected(::MenuSelect(inMacEvent.where));
}

void
CConduitDialog::HandleMenuSelected(long menuSelected)
{
	LCommander	*theTarget = LCommander::GetTarget();
	int endTicks = TickCount() + 8;
	if (theTarget != nil) {
		int menuID = HiWord(menuSelected);
		int menuItem = LoWord(menuSelected);
		CommandT command = kEditMenuCommands[menuItem - 1];
		if (command != cmd_Nothing)
			theTarget->ProcessCommand(command, 0);
	}
	while ( ::TickCount() < endTicks )
		;
	
	::HiliteMenu(0);
}

void
CConduitDialog::SetIdler(CSyncIdler* idler)
{
	delete fIdler;
	fIdler = idler;
}

LView*
CConduitDialog::GetView()
{
	ThrowIfNil_(fView);
	return fView;
}

CMenuHandler::CMenuHandler(LMenuController* theController, 
		const vector<int>& menuIDs, int numberOfExtras)
	: fNumberOfExtras(numberOfExtras), fMenu(theController->GetMacMenuH())
{
	int idCount = menuIDs.size();
	for (int i = 0; i < idCount; i += 1) {
		fIndexes[menuIDs[i]] = i + fNumberOfExtras + 1;
	}
}
	
void
CMenuHandler::Enable(int fieldID)
{
	int menuIndex = fIndexes[fieldID];
	if (menuIndex) {
		EnableMenuItem(fMenu, menuIndex);
	}
}

void
CMenuHandler::Disable(int fieldID)
{
	int menuIndex = fIndexes[fieldID];
	if (menuIndex) {
		DisableMenuItem(fMenu, menuIndex);
	}
}
