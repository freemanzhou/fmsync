/*
	File:		CGADialogBox.cp

	Contains:	xxx put contents here xxx

	Written by:	Chris LeCroy, Jeff Pritchard

	Copyright:	© 1997 by Now Software, Inc., all rights reserved.

	Change History (most recent first):

		<12>	10/27/97	csl		Added CGADialogBox::ShowAlertDialog

	To Do:
*/

// CGADialogBox.cp			jap				8/27/97
// a subclass of LGADialogBox that handles some common features of HotSync Modal Dialogs
// borrows heavily from powerplants stdialoghandler class


#include "CGADialogBox.h"
#include "HotSyncDefines.h"
#include "UMulticaster.h"
#include <LGAEditField.h>
#include <LEditField.h>

CGADialogBox::CGADialogBox()
{
	mDone = false;
	mResult = false;
	mAboutBoxMode = false;
	mPauseSeconds = 0;
	::GetDateTime(&mStartSeconds);
}

CGADialogBox::CGADialogBox(LStream *inStream) : LGADialogBox(inStream)
{
	mDone = false;
	mResult = false;
	mAboutBoxMode = false;
	mPauseSeconds = 0;
	mNoEditMenu = true;
	mNoOtherMenus = true;
	::GetDateTime(&mStartSeconds);
}

CGADialogBox::~CGADialogBox()
{
//	RestoreTarget();
}




void CGADialogBox::FinishCreateSelf()
{
	LGADialogBox::FinishCreateSelf();

	UMulticaster::GetMulticaster()->AddListener(this);

	LGAEditField *edit = (LGAEditField *)FindPaneByID(kEditField);
	if(edit)	// if there is a text edit field in this one, make it the target
	{
		SwitchTarget(edit);
		mNoEditMenu = false;
	}
	else
	{
		SwitchTarget(this);
	}
	
	LButton* button;
	button = (LButton*)FindPaneByID(kCon1);
	if(button)
		button->AddListener(this);
	button = (LButton*)FindPaneByID(kCon2);
	if(button)
		button->AddListener(this);

}


void
CGADialogBox::FindCommandStatus(
	CommandT	inCommand,
	Boolean		&outEnabled,
	Boolean		&outUsesMark,
	Char16		&outMark,
	Str255		outName)
{
	switch (inCommand) {
	
		case cmd_Undo:
		case cmd_Cut:
		case cmd_Copy:
		case cmd_Paste:
		case cmd_Clear:
		case cmd_SelectAll:
			if(mNoEditMenu)	// no text edit stuff
			{
				outEnabled = false;
			}
			else
			{
				LCommander::FindCommandStatus (inCommand,outEnabled,outUsesMark,outMark,outName);
			}
			break;
			
		default:
			if(mNoOtherMenus)
			{
				outEnabled = false;
			}
			else
			{
				LCommander::FindCommandStatus(inCommand, outEnabled,outUsesMark, outMark, outName);
			}
			break;
	}
}


void CGADialogBox::ListenToMessage	(MessageT inMessage,void* ioParam )
{
	switch(inMessage)
	{
		case msg_Cancel:
			mDone = true;
			mResult = 0;
			break;
			
		case msg_OK:
			mDone = true;
			mResult = kOKBut;
			break;
			
		case kCon1:
			mDone = true;
			mResult = kControl1;
			break;
			
		case kCon2:
			mDone = true;
			mResult = kControl2;
			break;
		
		case msg_HotSyncEnded:
			mDone = true;
			mResult = kUpdateEverything;
			break;
		
		default:
			LGADialogBox::ListenToMessage(inMessage,ioParam);
			break;
	}
}

	
	
void CGADialogBox::SetDialogTitle(StringPtr title)
{
	::SetWTitle(mMacWindowP,title);
}


void CGADialogBox::SetDialogCaption(StringPtr caption)
{
	LCaption *cap = (LCaption *)FindPaneByID(kCaptionID);
	ThrowIfNil_(cap);
	cap->SetDescriptor(caption);
}


void CGADialogBox::GetEditText(StringPtr editText)
{
	LPane *edit = FindPaneByID(kEditField);
	ThrowIfNil_(edit);
	edit->GetDescriptor(editText);	
}


void CGADialogBox::SetEditText(StringPtr editText)
{
	LPane *edit = FindPaneByID(kEditField);
	ThrowIfNil_(edit);
	edit->SetDescriptor(editText);	
	((LEditField *)edit)->SelectAll();
}

void CGADialogBox::SetAboutBoxMode(Boolean about,UInt32 seconds)
{
	mAboutBoxMode = about;
	mPauseSeconds = seconds;
}

void CGADialogBox::ClickInContent(const EventRecord	&inMacEvent)
{
	if(mAboutBoxMode)
	{
		mDone = true;
		mResult = kOKBut;
	}
	else
	{
		LWindow::ClickInContent(inMacEvent);
	}
}


void CGADialogBox::ShowAlertDialog(StringPtr alertString)
{
	CGADialogBox *box = (CGADialogBox *)LWindow::CreateWindow ( kAlertDialog,LCommander::GetTopCommander());
	ThrowIfNil_ ( box );
	box->SetDialogCaption(alertString);
	box->Show();
	SysBeep(0);
	Boolean result = box->WaitForResult();
	delete box;
}


short CGADialogBox::WaitForResult()
{
	// unabashedly stolen from UDialogs.cp
	EventRecord macEvent;
	LCommander* theTarget = GetTarget();
	
	SetUpdateCommandStatus(true);

	while(!mDone)
	{
		
		if (LCommander::IsOnDuty()) 
		{
			::OSEventAvail(0, &macEvent);
			LEventDispatcher::AdjustCursor(macEvent);
		}

//		LCommander::SetUpdateCommandStatus(false);

		Boolean gotEvent = ::WaitNextEvent(everyEvent, &macEvent,
											1, nil);
		
			// Let Attachments process the event. Continue with normal
			// event dispatching unless suppressed by an Attachment.
		
		if (LEventDispatcher::ExecuteAttachments(msg_Event, &macEvent)) 
		{
			if (gotEvent) 
			{
				if(mAboutBoxMode &&  macEvent.what == keyDown && 
					(((macEvent.message & keyCodeMask) == 0x2400) ||	// return key
					((macEvent.message & keyCodeMask) == 0x4C00)))		// enter key
				{
					mDone = true;
					mResult = kOKBut;
					return mResult;
				}
				
				if(macEvent.what == keyDown)
				{
					if(!HandleKeyPress(macEvent))
						DispatchEvent(macEvent);
					LCommander::SetUpdateCommandStatus(true);
				}
				else  
				{
					DispatchEvent(macEvent);
				}
			} 
			else 
			{
				UseIdleTime(macEvent);
			}
		}

										// Repeaters get time after every event
		LPeriodical::DevoteTimeToRepeaters(macEvent);
		
										// Update status of menu items
		SwitchTarget(theTarget);
		if (IsOnDuty() && GetUpdateCommandStatus()) 
		{
			LMenuBar *theMenuBar = LMenuBar::GetCurrentMenuBar();
			if(theMenuBar != 0)
				UpdateMenus();
		}
		
		if(mAboutBoxMode && (mPauseSeconds != 0))
		{
			UInt32 now;
			GetDateTime(&now);
			if(now >= (mStartSeconds + mPauseSeconds))
			{
				mDone = true;
				mResult = kOKBut;
				return mResult;
			}
		}
	}
	
	mDone = false;	// in case they want to check a result and leave dialog up
	return mResult;
}



StGrayDialog::StGrayDialog(ResIDT		inDialogResID)
{
	mDialog = (CGADialogBox*)LWindow::CreateWindow(inDialogResID,LCommander::GetTopCommander());
	SignalIf_(mDialog == nil);
//	mDialog->Show();		// done by caller after everything is set up
}


StGrayDialog::~StGrayDialog()
{
	delete mDialog;
}





StGrayAlertDialog::StGrayAlertDialog(StringPtr text)
{
	mDialog = (CGADialogBox*)LWindow::CreateWindow(kAlertDialogID, LCommander::GetTopCommander());
	SignalIf_(mDialog == nil);
	mDialog->SetDialogCaption(text);
	mDialog->Show();
	SysBeep(0);
	mDialog->WaitForResult();
}

StGrayAlertDialog::StGrayAlertDialog(short resID,short strID)
{
	LStr255 text(resID,strID);
	mDialog = (CGADialogBox*)LWindow::CreateWindow(kAlertDialogID, LCommander::GetTopCommander());
	SignalIf_(mDialog == nil);
	mDialog->SetDialogCaption(text);
	mDialog->Show();
	SysBeep(0);
	mDialog->WaitForResult();
}


StGrayAlertDialog::~StGrayAlertDialog()
{
	delete mDialog;
}
