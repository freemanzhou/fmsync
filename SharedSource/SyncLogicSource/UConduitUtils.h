/*
	File:		UConduitUtils.h

	Contains:	xxx put contents here xxx

	Created by:	Chris LeCroy on Fri, Sep 5, 1997

	Copyright:	© 1997 by Now Software, Inc., all rights reserved.

	Change History (most recent first):

				 00/00/00	---	file created

	To Do:
*/

#pragma once

#include "CMyStr255.h"

#ifndef __UConduitUtils__
#define __UConduitUtils__


const short kBackupConduitRef = 0;

typedef struct ConduitSettings
	{
		short currentSetting;
		short defaultSetting;
	} ConduitSettings,*ConduitSettingsPtr;

class UConduitUtils
{
public:
	
	static void ExtractUserVisibleConduitName(Handle inCinfResource,
												  StringPtr outConduitName);
	static OSErr GetOrCreateDataDir(const FSSpec& inUserFSSpec,
										ConstStr255Param conduitDataDirName,
										long* outDataDirID);
										
	static OSErr GetOrCreateSettingsFileSpec(short ConduitRef,const FSSpec &userSpec,FSSpec* spec);

	static ConduitSettings GetConduitSettingsValues(short ConduitRef,const FSSpec &userSpec);
	static void SetConduitSettingsValues(short ConduitRef,const FSSpec &userSpec,short setting,short defaultSetting);
	static short GetConduitCurrentSetting(short ConduitRef,const FSSpec &userSpec);
	static short GetConduitDefaultSetting(short ConduitRef,const FSSpec &userSpec);
	
};

#endif	//__UConduitUtils__
