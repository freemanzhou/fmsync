/*
	File:		StSyncClasses.cp

	Contains:	xxx put contents here xxx

	Created by:	Chris LeCroy on Wed, Sep 17, 1997

	Copyright:	© 1997 by Now Software, Inc., all rights reserved.

	Change History (most recent first):

				00/00/00	---	file 	created

	To Do:
*/

#include "StSyncClasses.h"
#include "CSynchronizer.h"

#include <UException.h>
#include <LThread.h>


StSyncRegisterConduit::StSyncRegisterConduit() :
	mConduitHandle(0)
{
	ThrowIfErrorC_(::SyncRegisterConduit(mConduitHandle));
}


StSyncRegisterConduit::~StSyncRegisterConduit()
{
	try
	{
		if (mConduitHandle != 0)
		{
			this->UnregisterConduit();
		}
	}
	catch (const LException& inErr)
	{	//don't ever throw exceptions out of destructors
		Assert_(inErr.GetErrorCode() == noErr);
	}
}


void StSyncRegisterConduit::UnregisterConduit()
{
	Assert_(mConduitHandle != 0);

	if (mConduitHandle != 0)
	{
		ThrowIfErrorC_(::SyncUnRegisterConduit(mConduitHandle));
		mConduitHandle = 0;
	}
}



StSyncOpenDB::StSyncOpenDB(const char* pName,
							int nCardNum,
							BYTE openMode) :
	mDbHandle(0)
{
	ThrowIfErrorC_(::SyncOpenDB(pName, nCardNum, mDbHandle, openMode));
}


StSyncOpenDB::~StSyncOpenDB()
{
	try
	{
		if (mDbHandle != nil)
		{
			this->CloseDB();
		}
	}
	catch (const LException& inErr)
	{	//don't ever throw exceptions out of destructors
		Assert_(inErr.GetErrorCode() == noErr);
	}

}


void StSyncOpenDB::CloseDB()
{
	Assert_(mDbHandle != 0);
	
	if (mDbHandle != 0)
	{
		ThrowIfOSErr_(::SyncCloseDB(mDbHandle));
		mDbHandle = 0;
	}
}



StSyncInit::StSyncInit(CTransportBase& inTransportBase) :
	mDeInited(true),
	mSyncEnded(true)
{
	Err err = ::SyncInit(inTransportBase);
	if (err != -1)							//-1 means it's already been called once...
		ThrowIfErrorC_(err);
		
	mDeInited = false;
	mSyncEnded = false;
}


StSyncInit::~StSyncInit()
{
	try
	{
		
		if (!mDeInited)
		{
			this->DeInit();
		}
		
		if (!mSyncEnded)
		{
			this->EndSync(eOther);
		}
	}
	catch (const LException& inErr)
	{	//don't ever throw exceptions out of destructors
		//Assert_(err == noErr);
	}
}


void StSyncInit::DeInit()
{
	if (!mDeInited)
	{
		(void)::SyncDeInit(0);
		mDeInited = true;
	}
}



void StSyncInit::EndSync(eEndStatus inSyncEndStatus)
{
	if (!mSyncEnded)
	{
		ThrowIfErrorC_(::SyncEndOfSync(inSyncEndStatus));
		
		//hang out for a few seconds and wait for the pilot to shutdown...
		UInt32 timeToGo = ::TickCount()+(60*3);
		while(::TickCount() < timeToGo)
		{
			LThread::Yield();
		}
		
		mSyncEnded =true;
	}
}



