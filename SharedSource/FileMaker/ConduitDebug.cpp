#include "ConduitDebug.h"
//#include "CUtility.h"

short gDebugRefNum;

#if 0

void DebugStart(StringPtr s) {}
void DebugStop(void) {}
void _DebugReportString(StringPtr s, bool noLineDelim) {}
void _DebugReportValue(unsigned long val, bool noLineDelim) {}

#else

void DebugStart(StringPtr s)
{
	if (0 == gDebugRefNum) {
		// ask debug file
		FSSpec fspec = {0, 0, "\pDebugOutput.txt"};
		OSErr err = noErr;
		
		// attempt to create, ignore error
		err = ::FSpCreate(&fspec, 'ttxt', 'TEXT', smSystemScript);

		err = ::FSpOpenDF(&fspec, fsCurPerm, &gDebugRefNum);
		if (err || 0 == gDebugRefNum) {
			SysBeep(0);
			return;
		}
		
		// always concat at end
		err = SetFPos(gDebugRefNum, fsFromLEOF, 0);
	}
	
	_DebugReportString("\p>>>>>>>>>> Opening");

	Str255 t = "\p";
	unsigned long seconds;
	::GetDateTime(&seconds);
	::DateString(seconds, false, t, NULL);
	_DebugReportString(t, true);

	_DebugReportString("\p - ", true);
	
	::TimeString(seconds, true, t, NULL);
	_DebugReportString(t);
	
	_DebugReportString(s);
}

void DebugStop(void)
{
	if (0 == gDebugRefNum) {
		// already closed or bad file
		return;
	}

	_DebugReportString("\p<<<<<<<<<< Closing");
	OSErr err = ::FSClose(gDebugRefNum);
}

void _DebugReportString(StringPtr s, bool noLineDelim)
{
	if (0 == gDebugRefNum) {
		return;
	}
	
	long count = s[0];
	OSErr err = ::FSWrite(gDebugRefNum, &count, &s[1]);
	if (err) {
		return;
	}
	
	if (noLineDelim) {
		return;
	}
	
	count = 1;
	err = ::FSWrite(gDebugRefNum, &count, "\r");
	if (err) {
		return;
	}
}

void _DebugReportValue(unsigned long val, bool noLineDelim)
{
	if (0 == gDebugRefNum) {
		return;
	}

	Str255 s;
	::NumToString(val, s);
	_DebugReportString(s, noLineDelim);
}

#endif