/* 
	CConduitWarner.h

	Author:			Rob Tsuk
	Description:	<describe the CConduitWarner class here>
*/

#ifndef CConduitWarner_H
#define CConduitWarner_H

#include "CWarnings.h"

class CConduitWarner
{
public:
	CConduitWarner(CWarnings& w);
	virtual ~CConduitWarner();

	bool			Warn(Warnings::WarningID warnID);

protected:

	CWarnings&	fWarnings;
};

#endif	// CConduitWarner_H
