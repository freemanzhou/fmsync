/* 
	CFrontProcess.cp

	Author:			Rob Tsuk
	Description:	<describe the CFrontProcess class here>
*/

#include "CFrontProcess.h"



CFrontProcess::CFrontProcess()
{
	ThrowIfOSErr_(GetFrontProcess(&fFrontProcess));
}


CFrontProcess::~CFrontProcess()
{
	ProcessSerialNumber frontProc;
	if( noErr == GetFrontProcess(&frontProc)) {
		Boolean isSame;
		if (noErr == SameProcess(&frontProc, &fFrontProcess, &isSame) && !isSame) {
			SetFrontProcess(&fFrontProcess);
		}
	}
}



