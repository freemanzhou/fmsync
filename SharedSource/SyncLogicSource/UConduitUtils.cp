/*
	File:		UConduitUtils.cp

	Contains:	xxx put contents here xxx

	Created by:	Chris LeCroy on Fri, Sep 5, 1997

	Copyright:	© 1997 by Now Software, Inc., all rights reserved.

	Change History (most recent first):

				 00/00/00	---	file 	created

	To Do:
*/


#include "UConduitUtils.h"

#include <Memory.h>
#include <String.h>

#include "MoreFilesExtras.h"
#include "HotSyncDefines.h"
#include "UGladUtilities.h"
#include "LGladFile.h"

void UConduitUtils::ExtractUserVisibleConduitName(Handle inCinfResource,
												  StringPtr outConduitName)
{
	SInt8 saveState = ::HGetState(inCinfResource);
	::HLock(inCinfResource);
	
	StringPtr conduitName = (StringPtr)((*inCinfResource) + 
										sizeof(short) +	//conduitType
										sizeof(long) +	//conduit version
										sizeof(long));	//conduit creator
										//next byte is beginning of conduit name
	
	::memcpy(outConduitName, conduitName, conduitName[0]+1);
	
	::HSetState(inCinfResource, saveState);
}


OSErr UConduitUtils::GetOrCreateDataDir(const FSSpec& inUserFSSpec,
										ConstStr255Param conduitDataDirName,
										long* outDataDirID)
{
	FSSpec dataDirFSSpec = {};
	OSErr err = noErr;
	long userDirID = 0;
	Boolean isDirectory;

	err = ::GetDirectoryID(inUserFSSpec.vRefNum, inUserFSSpec.parID, inUserFSSpec.name, &userDirID, &isDirectory);
	if (err == noErr)
	{
		err = ::FSMakeFSSpec(inUserFSSpec.vRefNum, userDirID, conduitDataDirName, &dataDirFSSpec);
		if (err == fnfErr || err == dirNFErr)		//dataDirFSSpec gets filled in regardless of whether or not FSMakeFSSpec succeeds
		{
			err = ::FSpDirCreate(&dataDirFSSpec, smSystemScript, outDataDirID);
		}
		else
		{
			err = ::GetDirectoryID(dataDirFSSpec.vRefNum,dataDirFSSpec.parID,dataDirFSSpec.name,outDataDirID,&isDirectory);
		}
	}
	
	return err;
}


OSErr UConduitUtils::GetOrCreateSettingsFileSpec(short conduitRef,const FSSpec &userSpec,FSSpec* spec)
{
	OSErr error;
	CMyStr255 conduitDataDirName(kFileStringsID,kConduitSettingsFolder);
	CMyStr255 settingsFileNameEnd(kFileStringsID,kConduitSettingsFileEnd);
	CMyStr255 settingsFileName;
	long outDataDirID;
	FSSpec conduitSpec;
	
	if(conduitRef != kBackupConduitRef)
	{
			// get the name of the conduit and use it to make a name for the conduit settings file
		ThrowIfOSErr_(FSpGetFileLocation(conduitRef,&conduitSpec));

		settingsFileName = conduitSpec.name;
		settingsFileName += settingsFileNameEnd;
		if(settingsFileName[0] > 31) settingsFileName[0] = 31;	// name example:  "InstallConduit Settings"	
	}
	else
		settingsFileName.Assign(kFileStringsID,kBackupConduitSettings);
	
	ThrowIfOSErr_(GetOrCreateDataDir(userSpec,conduitDataDirName,&outDataDirID));
	
	error = ::FSMakeFSSpec(userSpec.vRefNum, outDataDirID, settingsFileName,spec);
	if(error == fnfErr || error == dirNFErr)
	{
		LGladFile settingsFile(*spec);	// create the settings file
		settingsFile.OpenOrCreateDataFork(fsRdPerm,kHSM_creator,kConduitSettingsFile,0);
		error = ::FSMakeFSSpec(userSpec.vRefNum, outDataDirID, settingsFileName,spec);
	}

	ThrowIfOSErr_(error);
		 
	return error;
}



ConduitSettings UConduitUtils::GetConduitSettingsValues(short conduitRef,const FSSpec &userSpec)
{
	ConduitSettings settingAndDefault = {0,0};
	FSSpec settingsSpec = {};
	SInt32 answerSize = sizeof(settingAndDefault);
	ThrowIfOSErr_(GetOrCreateSettingsFileSpec(conduitRef,userSpec,&settingsSpec));
		
	LGladFile settingsFile(settingsSpec);
	settingsFile.OpenOrCreateDataFork(fsRdPerm,kHSM_creator,kConduitSettingsFile,0);
	if(settingsFile.GetLength() == 0)
	{
	//	DebugStr("\p new or empty settings file");
		return settingAndDefault;	// need to do proper error dialog here
	}
		
	settingsFile.ReadBytes(&settingAndDefault,answerSize);
	
	return settingAndDefault;
}

short UConduitUtils::GetConduitCurrentSetting(short ConduitRef,const FSSpec &userSpec)
{
	ConduitSettings settingAndDefault = GetConduitSettingsValues(ConduitRef,userSpec);
	return settingAndDefault.currentSetting;
}


short UConduitUtils::GetConduitDefaultSetting(short ConduitRef,const FSSpec &userSpec)
{
	ConduitSettings settingAndDefault = GetConduitSettingsValues(ConduitRef,userSpec);
	return settingAndDefault.defaultSetting;
}




void UConduitUtils::SetConduitSettingsValues(short conduitRef,const FSSpec &userSpec,short setting,short defaultSetting)
{
	ConduitSettings settingAndDefault = {setting,defaultSetting};
	FSSpec settingsSpec = {};
	SInt32 answerSize = sizeof(ConduitSettings);
	ThrowIfOSErr_(GetOrCreateSettingsFileSpec(conduitRef,userSpec,&settingsSpec));
		
	LGladFile settingsFile(settingsSpec);	
	settingsFile.OpenOrCreateDataFork(fsRdWrPerm,kHSM_creator,kConduitSettingsFile,0);
	settingsFile.WriteBytes(&settingAndDefault,answerSize);
}


