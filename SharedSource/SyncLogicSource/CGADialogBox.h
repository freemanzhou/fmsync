/*
	File:		CGADialogBox.h

	Contains:	xxx put contents here xxx

	Written by:	Chris LeCroy, Jeff Pritchard

	Copyright:	© 1997 by Now Software, Inc., all rights reserved.

	Change History (most recent first):

		 <8>	10/27/97	csl		Added CGADialogBox::ShowAlertDialog

	To Do:
*/

// CGADialogBox.h			jap				8/27/97
// a subclass of LGADialogBox that handles some common features of HotSync Modal Dialogs
// borrows heavily from powerplants stdialoghandler class

#pragma once

#include <LGADialogBox.h>
#include <LCommander.h>
#include <LEventDispatcher.h>


enum dialogsIDs
{
	kAlertDialog		= 10000,
	kSureDeleteDlg		= 10001
};

enum standardIDs
{
	kOKButton			= 'OKBt',
	kCancelButton		= 'Cnbt',
	kCaptionID			= 'capt',
	kEditField			= 'edit',
	kCon1				= 'con1',
	kCon2				= 'con2'
};

enum outputValues
{
	kCancelBut			= 0,
	kOKBut				= 1,
	kControl1			= 2,
	kControl2			= 3,
	kUpdateEverything	= 4
};

class CGADialogBox : public LGADialogBox,
						public LEventDispatcher
						  
{
public:
	enum	{ class_ID = 'cdlg' };

						CGADialogBox();
						CGADialogBox(LStream *inStream);
						~CGADialogBox();
						
	virtual	void		FinishCreateSelf();
	virtual void		ListenToMessage	(MessageT inMessage,void* ioParam );	
	
			void		SetDialogTitle(StringPtr title);
			void		SetDialogCaption(StringPtr caption);
			void		GetEditText(StringPtr editText);
			void		SetEditText(StringPtr editText);
			void		EnableEditMenu() { mNoEditMenu = false; };
			void		DisableEditMenu() { mNoEditMenu = true; };
			void		EnableOtherMenus()	{ mNoOtherMenus = false; };
			void		DisableOtherMenus() { mNoOtherMenus = true; };
			void		SetAboutBoxMode(Boolean about,UInt32 seconds = 0);
			void		ClickInContent( const EventRecord	&inMacEvent);
			short		WaitForResult();
	virtual void		FindCommandStatus(
								CommandT			inCommand,
								Boolean				&outEnabled,
								Boolean				&outUsesMark,
								Char16				&outMark,
								Str255				outName);

	
	static void ShowAlertDialog(StringPtr alertString); //convenience function

protected:
	Boolean 		mDone;
	short 			mResult;
	Boolean			mAboutBoxMode;
	Boolean			mNoEditMenu;
	Boolean			mNoOtherMenus;
	UInt32			mPauseSeconds;
	UInt32			mStartSeconds;
};


class	StGrayDialog
{
public:
						StGrayDialog(ResIDT inDialogResID);
	virtual				~StGrayDialog();
	
			CGADialogBox* GetDialogBox() { return mDialog; };

protected:
	CGADialogBox		*mDialog;
};


class StGrayAlertDialog
{
public:
						StGrayAlertDialog(StringPtr text);
						StGrayAlertDialog(short resID,short strID);
	virtual				~StGrayAlertDialog();
	
protected:
	CGADialogBox		*mDialog;
};

