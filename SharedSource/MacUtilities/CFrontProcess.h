/* 
	CFrontProcess.h

	Author:			Rob Tsuk
	Description:	<describe the CFrontProcess class here>
*/

#ifndef CFrontProcess_H
#define CFrontProcess_H

#include "Processes.h"



class CFrontProcess
{
public:
	CFrontProcess();
	virtual ~CFrontProcess();
protected:
	ProcessSerialNumber fFrontProcess;

};

#endif	// CFrontProcess_H
