/*
	File:		StSharedBuffer.h

	Contains:	xxx put contents here xxx

	Created by:	Chris LeCroy on Sun, Nov 16, 1997

	Copyright:	© 1997 by Now Software, Inc., all rights reserved.

	Change History (most recent first):

				00/00/00---	file created

	To Do:
*/

#pragma once

#ifndef __StSharedBuffer__
#define __StSharedBuffer__

#include <UDebugging.h>

const UInt32 kDefaultBufferLen = 0xFFF0;
const bool kSharedBufferFreeWhenReleased = true;
const bool kSharedBufferUseTempMem = true;

class StSharedBuffer
{
public:
	
	StSharedBuffer(UInt32 inBufferLen = kDefaultBufferLen, 
					Boolean releaseWhenDone = !kSharedBufferFreeWhenReleased, 
					Boolean useTempMem = !kSharedBufferUseTempMem);
	virtual ~StSharedBuffer();
	
	inline void Grab()
	{	
		Assert_(!mInUse);
		
		if (mBuffer == nil)
		{			
			this->AllocateBuffer();
		}
		
		::HLock(mBuffer);
		
		mInUse = true;
	}
	
	inline void Release()
	{
		Assert_(mInUse);
	
		::HUnlock(mBuffer);
		
		if (mDeallocateWhenReleased)
		{
			this->DeallocateBuffer();
		}
		
		mInUse = false;
	}
	
	inline void*  GetPtr() {return *mBuffer;}
	inline operator void*()	{ return *mBuffer; }
	inline operator char*()	{ return *mBuffer; }
	inline operator unsigned char*()	{ return (unsigned char*)*mBuffer; }
	inline UInt32 GetBufferLen(){ return mBufferLen;}
	
protected:
		
	void AllocateBuffer();
	void DeallocateBuffer();
	
	Handle mBuffer;
	Boolean mUseTempMem;
	Boolean mInUse;
	UInt32 mBufferLen;
	Boolean mDeallocateWhenReleased;
};

class StUseSharedBuffer
{
public:
	StUseSharedBuffer(StSharedBuffer& inBuffer)
	{
		mSharedBuffer = &inBuffer;
		mSharedBuffer->Grab();
	}
	virtual ~StUseSharedBuffer()
	{
		mSharedBuffer->Release();
	}
	
private:
	StSharedBuffer* mSharedBuffer;
};

#endif	//__StSharedBuffer__
  