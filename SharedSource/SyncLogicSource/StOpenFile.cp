/*
	File:		StOpenFile.cp

	Contains:	xxx put contents here xxx

	Created by:	Chris LeCroy on Fri, Oct 10, 1997

	Copyright:	© 1997 by Now Software, Inc., all rights reserved.

	Change History (most recent first):

				00/00/00	---	file 	created

	To Do:
*/

#include "StOpenFile.h"

#include <LFile.h>


#include "UGladUtilities.h"

StOpenFileDataFork::StOpenFileDataFork(LFile* inFile, Int16  inPrivileges) :
	mFile(nil),
	mWasOpen(false)
{
	Assert_(inFile != nil);
	
	if (inFile != nil)
	{
		mFile = inFile;
		mWasOpen = inFile->GetDataForkRefNum() != refNum_Undefined;
		
		inFile->OpenDataFork(inPrivileges);
	}
}


StOpenFileDataFork::~StOpenFileDataFork()
{
//this is a weird situation - this is an automatic object and we do want to propogate
//the exception if the destructor is not being called due to another exception
// The solution is probably to add a ::Close() method and "require" the user to call it
// when they are safel done using the file

	Try_
	{
		if (mFile != nil && !mWasOpen)
		{
			mFile->CloseDataFork();
		}
	}
	Catch_(err)
	{	//don't ever throw exceptions out of destructors
		Assert_(err == noErr);
	}
}



StOpenFileRsrcFork::StOpenFileRsrcFork(LFile* inFile, Int16  inPrivileges)
{
	Assert_(inFile != nil);
	
	if (inFile != nil)
	{
		mFile = inFile;
		mWasOpen = inFile->GetResourceForkRefNum() != refNum_Undefined;
		
		inFile->OpenResourceFork(inPrivileges);
	}
}


StOpenFileRsrcFork::~StOpenFileRsrcFork()
{
//this is a weird situation - this is an automatic object and we do want to propogate
//the exception if the destructor is not being called due to another exception
// The solution is probably to add a ::Close() method and "require" the user to call it
// when they are safel done using the file

	Try_
	{
		if (mFile != nil && !mWasOpen)
		{
			mFile->CloseResourceFork();
		}
	}
	Catch_(err)
	{	//don't ever throw exceptions out of destructors
		Assert_(err == noErr);
	}
}



StOpenFile::StOpenFile(LFile* inFile, Int16  inPrivileges) :
	StOpenFileDataFork (inFile, inPrivileges),
	StOpenFileRsrcFork (inFile, inPrivileges)
	
{
}
