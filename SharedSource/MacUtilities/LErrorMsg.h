// ===========================================================================
//	LErrorMsg.h			            ©1996-1997 SP extreme All rights reserved.
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
//	http://144.118.247.241/
//	http://144.118.28.34/SPextreme/index.html	(mirror site)
//

#ifndef ErrorChecking_h
#define ErrorChecking_h
#pragma once

#include <PP_Types.h>
#include <stringfwd>
#include "ConverterErrors.h"

/*error alrt ids*/
const	ResIDT	Strnum_RsrcID	=	128;

class LErrorMsg
{
	public:
		//Constructor
		LErrorMsg( );
	
		//Destructor
		~LErrorMsg( );
		
		static void AnnounceTruncatedFields(int count);
		static void AnnounceError(OSErr err, const FSSpec& inputFile);
		static void AnnounceError(const std::string& errorString);
		static void AnnounceConverterError(ConverterError err, const FSSpec& inputFile);
	
		static int AlertWithMessage( int alertType, const std::string& errorString, bool hasCancel = false);
		static int AlertWithMessageAndDetails( int alertType, const std::string& errorString, const std::string& detailsString, bool hasCancel = false);
		static std::string MessageFromParameters( ResIDT RsrcStrID, ResIDT StrRsrcID, StringPtr theStr);

		static int AlertWithMessageAM( int alertType, const std::string& errorString, bool hasCancel = false);
		static int AlertWithMessageAndDetailsAM( int alertType, const std::string& errorString, const std::string& detailsString, bool hasCancel = false);

		static int AlertOld(ResIDT alertRsrc, int alertType);
		static int AlertWithMessageOld( int alertType, const std::string& errorString, bool hasCancel = false);
		static int AlertWithMessageAndDetailsOld( int alertType, const std::string& errorString, const std::string& detailsString, bool hasCancel = false);

		//Member functions
		//Stop alert messages
		static void StopAlertMsg( ResIDT RsrcStrID, ResIDT StrRsrcID, StringPtr theStr );
		static void StopAlertMsg( ResIDT STRRsrc );
		static void StopAlertMsg( StringPtr theStr  );
		
		//Warning alerts with one button
		static void WarnAlertMsg1Btn( ResIDT RsrcStrID, ResIDT StrRsrcID, StringPtr theStr );
		static void WarnAlertMsg1Btn( ResIDT STRRsrc );
		static void WarnAlertMsg1Btn( StringPtr theStr );
		
		//Warning Alerts with 2 Buttons and a return for what happen
		static short WarnAlertMsg2Btn( ResIDT RsrcStrID, ResIDT StrRsrcID, StringPtr theStr );
		static short WarnAlertMsg2Btn( ResIDT STRRsrc );
		static short WarnAlertMsg2Btn( StringPtr theStr );
	
};
#endif //ErrorChecking