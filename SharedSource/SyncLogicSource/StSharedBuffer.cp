/*
	File:		StSharedBuffer.cp

	Contains:	xxx put contents here xxx

	Created by:	Chris LeCroy on Sun, Nov 16, 1997

	Copyright:	© 1997 by Now Software, Inc., all rights reserved.

	Change History (most recent first):

				00/00/00	---	file 	created

	To Do:
*/

#include "StSharedBuffer.h"

#include <UException.h>


StSharedBuffer::StSharedBuffer( UInt32 inBufferLen, 
								Boolean releaseWhenDone, 
								Boolean useTempMem) : 
	mDeallocateWhenReleased(false),
	mUseTempMem(false),
	mInUse(false),
	mBufferLen(inBufferLen),
	mBuffer(nil)
{

}


StSharedBuffer::~StSharedBuffer()
{
	Assert_(!mInUse);
	
	if (mBuffer != nil)
	{
		this->DeallocateBuffer();
	}
}


void StSharedBuffer::DeallocateBuffer()
{
	Assert_(mBuffer != nil);
	
	if (mBuffer != nil)
	{
		::DisposeHandle(mBuffer);
		mBuffer = nil;
	}
}



void StSharedBuffer::AllocateBuffer()
{
	Assert_(mBuffer == nil);

	if (mBuffer == nil)
	{		
		if (mUseTempMem)
		{
			OSErr err = noErr;
			mBuffer = ::TempNewHandle(this->GetBufferLen(), &err);
			ThrowIfNil_(mBuffer);
			ThrowIfOSErr_(err);
			
		}
		else
		{
			mBuffer = ::NewHandle(this->GetBufferLen());
			ThrowIfNil_(mBuffer);
		}
	}
}



