// ===========================================================================
//	LErrorMsg.cpp			            ©1996-1997 SP extreme All rights reserved.
// ===========================================================================
//
//	This is a basic error cheching class for use with powerplant.  It handles
//	calling basic Alert Windows(ALRT) and displays the give text.
//
//	For bug reports and ideas for added functions email Steve Paulin
//
//	st93eft3@dunx1.ocs.drexel.edu
//
//	or visit the SP extreme web site at:
//
//	http://144.118.247.241
//	http://144.118.28.34/SPextreme/index.html	(mirror site)
//

#include <Dialogs.h>

#include "LErrorMsg.h"
#include "Stringiness.h"
#include "Str255.h"
//#include "PFCResources.h"

using std::string;

// ---------------------------------------------------------------------------
//		¥ LErrorMsg
// ---------------------------------------------------------------------------
//Not used for anythings...Basic constructor.

enum {
	kOverwriteWarningStrings = 128,
	kErrorNumberStrings,
	kCouldNotConvertStrings,
	kSystem7Strings,
	kConverterErrorStrings,
	kFieldTruncationStrings,
	kNumberStrings
};

LErrorMsg::LErrorMsg( void )
{
}


// ---------------------------------------------------------------------------
//		¥ ~LErrorMsg
// ---------------------------------------------------------------------------
//Since we don't construct anything why do we distroy it???	

LErrorMsg::~LErrorMsg( )
{
}


string
LErrorMsg::MessageFromParameters( ResIDT inStrRsrcID, ResIDT inStrRsrcIndex, StringPtr theStr)
{
	string theMessage("<message omitted>");
	if( inStrRsrcID == 0 )
	{
		theMessage = AsString(theStr);
	}
	else if( theStr == 0 )
	{
		Str255		stringFromNum;
		GetIndString( stringFromNum, inStrRsrcID, inStrRsrcIndex );
		theMessage = AsString(stringFromNum);
	} else if (theStr != 0) {
		theMessage = AsString(theStr);
	}
	return theMessage;
}

int
LErrorMsg::AlertWithMessageAM( int alertType, const string& errorString, bool hasCancel)
{
	AlertStdAlertParamRec params;
	
	params.movable = false;
	params.helpButton = false;
#ifndef CONDUIT
	UDesktop::Deactivate();
	params.filterProc = NewModalFilterUPP(UModalAlerts::GetModalEventFilter());
	params.movable = true;
#else
	params.filterProc = 0;
	params.movable = false;
#endif
	params.defaultText = (StringPtr)kAlertDefaultOKText;
	if (hasCancel)
		params.cancelText = (StringPtr)kAlertDefaultCancelText;
	else
		params.cancelText = 0;
	params.otherText = 0;
	params.defaultButton = kAlertStdAlertOKButton;
	if (hasCancel)
		params.cancelButton = kAlertStdAlertCancelButton;
	else
		params.cancelButton = 0;
	params.position = kWindowDefaultPosition;
	LStr255 theText(errorString.c_str());
	short itemHit;
	::StandardAlert(alertType, theText, 0, &params, &itemHit);
#ifndef CONDUIT
	UDesktop::Activate();
	DisposeModalFilterUPP(params.filterProc);
#endif
	return itemHit;
}

int
LErrorMsg::AlertWithMessage( int alertType, const string& errorString, bool hasCancel)
{
	return AlertWithMessageAM(alertType, errorString, hasCancel);
}

int
LErrorMsg::AlertOld(ResIDT alertRsrc, int alertType)
{
	int itemHit = 1;
	switch(alertType) {
		case kAlertStopAlert:
			itemHit = UModalAlerts::StopAlert(alertRsrc);
			break;
		case kAlertNoteAlert:
			itemHit = UModalAlerts::NoteAlert(alertRsrc);
			break;
		case kAlertCautionAlert:
			itemHit = UModalAlerts::CautionAlert(alertRsrc);
			break;
		case kAlertPlainAlert:
			itemHit = UModalAlerts::Alert(alertRsrc);
			break;
	};
	return itemHit;
}

int
LErrorMsg::AlertWithMessageAndDetailsAM( int alertType, const string& errorString, const string& detailsString, bool hasCancel)
{
	AlertStdAlertParamRec params;
	
#ifndef CONDUIT
	UDesktop::Deactivate();
	params.filterProc = NewModalFilterUPP(UModalAlerts::GetModalEventFilter());
	params.movable = true;
#else
	params.filterProc = 0;
	params.movable = false;
#endif

	params.helpButton = false;
	params.defaultText = (StringPtr)kAlertDefaultOKText;
	if (hasCancel)
		params.cancelText = (StringPtr)kAlertDefaultCancelText;
	else
		params.cancelText = 0;
	params.otherText = 0;
	params.defaultButton = kAlertStdAlertOKButton;
	if (hasCancel) {
		params.cancelButton = kAlertStdAlertCancelButton;
	} else {
		params.cancelButton = 0;
	}
	params.position = kWindowDefaultPosition;
	LStr255 theText(errorString.c_str());
	LStr255 otherText(detailsString.c_str());
	short itemHit;
	ThrowIfOSErr_(::StandardAlert(alertType, theText, otherText, &params, &itemHit));
#ifndef CONDUIT
	UDesktop::Activate();
	DisposeModalFilterUPP(params.filterProc);
#endif
	return itemHit;
}

int
LErrorMsg::AlertWithMessageAndDetails( int alertType, const string& errorString, const string& detailsString, bool hasCancel)
{
	return AlertWithMessageAndDetailsAM(alertType, errorString, detailsString, hasCancel);
}

// ---------------------------------------------------------------------------
//		¥ StopAlertMsg( ResIDT, ResIDT, StringPtr )
// ---------------------------------------------------------------------------
//	This function gets the string resource id and gets a string or
//	a resource number and outputs that to the alert box.  It decides which
//	is used by which value is set to NULL.  This will also terminate the
//	program.
//		
//		RsrcStrID	the STR# resource id( required)
//		inStrRsrcID	the number to use in the STR# resource( may be NULL )
//		inStr		the string to print to the alert( may be NULL )

void
LErrorMsg::StopAlertMsg( ResIDT inRsrcStrID, ResIDT inStrRsrcID, StringPtr inStr )
{
	string message(MessageFromParameters(inRsrcStrID, inStrRsrcID, inStr));
	AlertWithMessage(kAlertStopAlert, message);
}


// ---------------------------------------------------------------------------
//		¥ StopAlertMsg( ResIDT )
// ---------------------------------------------------------------------------
//	This just calls the above with a str# resource number and a preset 
//	str# resource ID 128.

void
LErrorMsg::StopAlertMsg( ResIDT inSTRRsrc )
{
	StopAlertMsg( Strnum_RsrcID, inSTRRsrc, NULL );
}


// ---------------------------------------------------------------------------
//		¥ StopAlertMsg( StringPtr )
// ---------------------------------------------------------------------------
//	It handles a string call to the main StopError function.  It uses
//	the default string ResourceID since one really ins't need.

void
LErrorMsg::StopAlertMsg( StringPtr inStr  )
{
	StopAlertMsg( Strnum_RsrcID, NULL, inStr );
}


// ---------------------------------------------------------------------------
//		¥ WarnAlertMsg1Btn( ResIDT, ResIDT, StringPtr )
// ---------------------------------------------------------------------------
//	This function gets the string resource id and gets a string or
//	a resource number and outputs that to the alert box.  It decides which
//	is used by which value is set to NULL.  This will just let the person know 
//	what happen and wait for them to click ok.
//		
//		inRsrcStrID	the STR# resource id( required)
//		inStrRsrcID	the number to use in the STR# resource( may be NULL )
//		inStr		the string to print to the alert( may be NULL )

void
LErrorMsg::WarnAlertMsg1Btn( ResIDT inRsrcStrID, ResIDT inStrRsrcID, StringPtr inStr )
{
	string message(MessageFromParameters(inRsrcStrID, inStrRsrcID, inStr));
	AlertWithMessage(kAlertNoteAlert, message);
}


// ---------------------------------------------------------------------------
//		¥ WarnAlertMsg1Btn( ResIDT )
// ---------------------------------------------------------------------------
//	Calls the main warning 1 button msg with the STR# resource ID and passes
//	it the number in that resource to use.

void
LErrorMsg::WarnAlertMsg1Btn( ResIDT inSTRRsrc )
{
	WarnAlertMsg1Btn( Strnum_RsrcID, inSTRRsrc, NULL );
}


// ---------------------------------------------------------------------------
//		¥ WarnAlertMsg1Btn( StringPtr )
// ---------------------------------------------------------------------------
//	This calls the main warning 1 button function with a string.  It also gives
//	the standard string resource just for a dumn reason.
	
void
LErrorMsg::WarnAlertMsg1Btn( StringPtr inStr )
{
	WarnAlertMsg1Btn( Strnum_RsrcID, NULL, inStr );
}


// ---------------------------------------------------------------------------
//		¥ WarnAlertMsg2Btn( ResIDT, ResIDT, StringPtr )
// ---------------------------------------------------------------------------
//	This function gets the string resource id and gets a string or
//	a resource number and outputs that to the alert box.  It decides which
//	is used by which value is set to NULL.  This will return an option of
//	which button was pressed.  OK( 1 ) or Cancel( 2 ).
//		
//		inRsrcStrID	the STR# resource id( required)
//		inStrRsrcID	the number to use in the STR# resource( may be NULL )
//		inStr		the string to print to the alert( may be NULL )


short
LErrorMsg::WarnAlertMsg2Btn( ResIDT inRsrcStrID, ResIDT inStrRsrcID, StringPtr inStr )
{
	string message(MessageFromParameters(inRsrcStrID, inStrRsrcID, inStr));
	return AlertWithMessage(kAlertNoteAlert, message, true);
}


// ---------------------------------------------------------------------------
//		¥ WarnAlertMsg2Btn( ResIDT )
// ---------------------------------------------------------------------------
//	This just returns a value of 1( OK ) or 2( Cancel ) of whatever the user
//	had pushed.  It calls the main warning 2 button function to get the 
//	answer.

short
LErrorMsg::WarnAlertMsg2Btn( ResIDT inSTRRsrc )
{
	return( WarnAlertMsg2Btn( Strnum_RsrcID, inSTRRsrc, NULL ) );
}


// ---------------------------------------------------------------------------
//		¥ WarnAlertMsg2Btn( StringPtr )
// ---------------------------------------------------------------------------
//	This just returns a value of 1( OK ) or 2( Cancel ) of whatever the user
//	had pushed.  It calls the main warning 2 button function to get the 
//	answer.

short
LErrorMsg::WarnAlertMsg2Btn( StringPtr inStr )
{
	return( WarnAlertMsg2Btn( Strnum_RsrcID, NULL, inStr ) );
}

void
LErrorMsg::AnnounceError(const string& errorString)
{
	AlertWithMessage(kAlertStopAlert, errorString);
}

void
LErrorMsg::AnnounceError(OSErr err, const FSSpec& inputFile)
{
	LStr255 errorString(kErrorNumberStrings, 1);
	LStr255 errorString2(kErrorNumberStrings, 2);
	LStr255 errorString3(kErrorNumberStrings, 3);
	errorString.Append(inputFile.name);
	errorString.Append(errorString2);
	errorString.Append((long)err);
	errorString.Append(errorString3);
	AnnounceError(AsString(errorString));
}

void
LErrorMsg::AnnounceTruncatedFields(int count)
{
	LStr255 errorString(kFieldTruncationStrings, 1);
	LStr255 wasTruncated(kFieldTruncationStrings, 2);
	LStr255 wereTruncated(kFieldTruncationStrings, 3);
	LStr255 limit(kFieldTruncationStrings, 4);
	if (count > 9) {
		errorString.Append((long)count);
		errorString.Append(' ');
		errorString.Append(wereTruncated);
	} else {
		LStr255 number(kNumberStrings, count);
		errorString.Append(number);
		errorString.Append(' ');
		if (count > 1)
			errorString.Append(wereTruncated);
		else
			errorString.Append(wasTruncated);
	}
	errorString.Append(limit);
	LErrorMsg::WarnAlertMsg1Btn(0, 0, errorString);
}

void
LErrorMsg::AnnounceConverterError(ConverterError err, const FSSpec& inputFile)
{
	LStr255 message(kConverterErrorStrings, err);
	LStr255 errorString(kCouldNotConvertStrings, 1);
	LStr255 errorString2(kCouldNotConvertStrings, 2);
	errorString.Append(inputFile.name);
	errorString.Append(errorString2);
	errorString.Append(message);
	AnnounceError(AsString(errorString));
}

static void
AnnounceError(const char *message, const FSSpec& inputFile)
{
	LStr255 errorString(130, 1);
	LStr255 errorString2(130, 2);
	LStr255 errorString3(message, std::strlen(message));
	errorString.Append(inputFile.name);
	errorString.Append(errorString2);
	errorString.Append(errorString3);
	LErrorMsg::AnnounceError(AsString(errorString));
}
