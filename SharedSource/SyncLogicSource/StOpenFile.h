/*
	File:		StOpenFile.h

	Contains:	xxx put contents here xxx

	Created by:	Chris LeCroy on Fri, Oct 10, 1997

	Copyright:	© 1997 by Now Software, Inc., all rights reserved.

	Change History (most recent first):

				00/00/00---	file created

	To Do:
*/

#pragma once

#ifndef __StOpenFile__
#define __StOpenFile__

#include <PP_Prefix.h>

class LFile;

class StOpenFileDataFork
{
public:
	StOpenFileDataFork(LFile* inFile, SInt16  inPrivileges);
	virtual ~StOpenFileDataFork();
	
	LFile* mFile;
	Boolean mWasOpen;
};

class StOpenFileRsrcFork
{
public:
	StOpenFileRsrcFork(LFile* inFile, SInt16  inPrivileges);
	virtual ~StOpenFileRsrcFork();
	
	LFile* mFile;
	Boolean mWasOpen;
};

class StOpenFile:	public StOpenFileDataFork,
					public StOpenFileRsrcFork
{
public:
	StOpenFile(LFile* inFile, SInt16  inPrivileges);
};

#endif	//__StOpenFile__
