/*
	File:		StSyncClasses.h

	Contains:	xxx put contents here xxx

	Created by:	Chris LeCroy on Wed, Sep 17, 1997

	Copyright:	© 1997 by Now Software, Inc., all rights reserved.

	Change History (most recent first):

				00/00/00---	file created

	To Do:
*/

#pragma once

#ifndef __StSyncClasses__
#define __StSyncClasses__

#include "SyncMgr.h"

class StSyncRegisterConduit
{
public:
	StSyncRegisterConduit();
	virtual ~StSyncRegisterConduit();
	operator CONDHANDLE()	{ return mConduitHandle; }
	void UnregisterConduit();

	
private:
	CONDHANDLE mConduitHandle;
};


class StSyncOpenDB
{
public:
	StSyncOpenDB(const char* pName,
							int nCardNum,
							BYTE openMode = (eDbWrite | eDbRead | eDbExclusive));
	virtual ~StSyncOpenDB();
	
	operator BYTE()	{ return mDbHandle; }
	void CloseDB();


private:
	BYTE mDbHandle;
};


class StSyncInit
{
public:
	StSyncInit(CTransportBase& inTransportBase);
	virtual ~StSyncInit();
	void DeInit();

	void EndSync(eEndStatus inSyncEndStatus);
private:
	Boolean mDeInited;
	Boolean mSyncEnded;
};



#endif	//__StSyncClasses__
