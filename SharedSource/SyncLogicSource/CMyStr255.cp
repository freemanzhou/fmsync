// CMyStr255.cp
// Created by Forest Hill on Thu, Mar 12, 1998 @ 12:25 PM.

#ifndef __CMyStr255__
#include "CMyStr255.h"
#endif

#include <UException.h>

#pragma mark ---PUBLIC LOCAL METHODS---

CMyStr255& CMyStr255::Set( const CMyStr255& inSource )
{
	CMyStr255::CopyPStr( inSource.mStringData, mStringData );
	if( mIsHybrid )
		this->MakeHybrid();
	
	return *this;
}

CMyStr255& CMyStr255::Set( ConstMyStrPtr inSource )
{
	CMyStr255::CopyPStr( inSource, mStringData );
	if( mIsHybrid )
		this->MakeHybrid();
	
	return *this;
}

CMyStr255& CMyStr255::Set( const char* inCStr )
{
	Str255 tmpStr;
	CMyStr255::CopyPStr( CMyStr255::CStrToPStr( inCStr, tmpStr ), mStringData );
	if( mIsHybrid )
		this->MakeHybrid();
	
	return *this;
}

CMyStr255& CMyStr255::Set( short inListID, short inStrIndex )
{
	GetIndString( mStringData, inListID, inStrIndex);
	ThrowIfOSErr_( ResError() );

	if( mIsHybrid )
		this->MakeHybrid();
	
	return *this;
}

CMyStr255& CMyStr255::Set( char inChar )
{
	mStringData[0] = 1;
	mStringData[1] = inChar;
	if( mIsHybrid )
		this->MakeHybrid();
	
	return *this;
}

CMyStr255& CMyStr255::Append( const CMyStr255& inSource )
{
	CMyStr255::AppendPStr( mStringData, inSource.mStringData );
	if( mIsHybrid )
		this->MakeHybrid();
	
	return *this;
}

CMyStr255& CMyStr255::Append( ConstMyStrPtr inSource )
{
	CMyStr255::AppendPStr( mStringData, inSource );
	if( mIsHybrid )
		this->MakeHybrid();
	
	return *this;
}

CMyStr255& CMyStr255::Append( char* inCStr )
{
	Str255 tmpStr;
	CMyStr255::AppendPStr( mStringData, CMyStr255::CStrToPStr( inCStr, tmpStr ) );
	if( mIsHybrid )
		this->MakeHybrid();
	
	return *this;
}

CMyStr255& CMyStr255::Append( char inChar )
{
	CMyStr255::AppendChar( mStringData, inChar );
	if( mIsHybrid )
		this->MakeHybrid();
	
	return *this;
}

CMyStr255& CMyStr255::Insert( const CMyStr255& inToInsert, UInt8 inInsertBefore )
{
	CMyStr255::InsertIntoPStr( mStringData, inToInsert.mStringData, inInsertBefore );
	if( mIsHybrid )
		this->MakeHybrid();
	
	return *this;
}

CMyStr255& CMyStr255::Insert( ConstMyStrPtr inToInsert, UInt8 inInsertBefore )
{
	CMyStr255::InsertIntoPStr( mStringData, inToInsert, inInsertBefore );
	if( mIsHybrid )
		this->MakeHybrid();
	
	return *this;
}

CMyStr255& CMyStr255::Insert( char* inCStr, UInt8 inInsertBefore )
{
	Str255 tmpStr;
	CMyStr255::InsertIntoPStr( mStringData, CMyStr255::CStrToPStr( inCStr, tmpStr ), inInsertBefore );
	if( mIsHybrid )
		this->MakeHybrid();
	
	return *this;
}

char* CMyStr255::ToCStr( char* outCStr, UInt16 inDestSize )
{
	return( CMyStr255::PStrToCStr( mStringData, outCStr, inDestSize ) );
}

CMyStr255& CMyStr255::MakeHybrid()
{
	CMyStr255::MakeHybridString( mStringData );
	mIsHybrid = true;
	
	return *this;
}



#pragma mark -
#pragma mark ---PUBLIC STATIC METHODS---
MyStrPtr CMyStr255::CopyPStr( ConstMyStrPtr inSource, MyStrPtr outDest, UInt16 inDestSize )
{
	UInt16 len = inSource[0] + 1;
	
	if( len > inDestSize )
		len = inDestSize;
	
	::BlockMoveData( inSource, outDest, len );
	outDest[0] = len-1;
	
	return( outDest );
}

MyStrPtr CMyStr255::AppendPStr( MyStrPtr ioBase, ConstMyStrPtr inToAppend, UInt16 inBaseSize )
{
	UInt16 totLen = inToAppend[0] + ioBase[0];
	if( totLen > inBaseSize )
		totLen = inBaseSize;
	UInt16 appendLen = totLen - ioBase[0];

	::BlockMoveData( &inToAppend[1], &ioBase[ioBase[0]+1], appendLen );
	ioBase[0] = totLen;

	return( ioBase );
}

MyStrPtr CMyStr255::AppendChar( MyStrPtr ioBase, char inChar, UInt16 inBaseSize )
{
	if( ioBase[0] < inBaseSize-1 )
		ioBase[++ioBase[0]] = inChar;

	return( ioBase );
}

MyStrPtr CMyStr255::InsertIntoPStr( MyStrPtr ioBase, ConstMyStrPtr inToInsert, UInt8 insertBefore, UInt16 inBaseSize )
{
	//no inserting into the length byte!
	if( insertBefore == 0 )
		insertBefore = 1;

	//if they want to insert after the last character, then just do an append
	if( insertBefore > ioBase[0] )
		return( CMyStr255::AppendPStr( ioBase, inToInsert, inBaseSize ) );

	UInt8 numToInsert = inToInsert[0];

	//first move the end out of the way
	UInt8 numToMove = ioBase[0] - insertBefore + 1;
	
	if( numToMove + insertBefore + numToInsert > inBaseSize - 1 )
		numToMove = inBaseSize - insertBefore - numToInsert;
	::BlockMoveData( &ioBase[insertBefore], &ioBase[insertBefore+numToInsert], numToMove );
	if( ioBase[0] + numToInsert > inBaseSize - 1 )
		ioBase[0] = inBaseSize - 1;
	else
		ioBase[0] += numToInsert;
	
	//now move the new stuff in
	::BlockMoveData( &inToInsert[1], &ioBase[insertBefore], numToInsert );

	return( ioBase );
}

MyStrPtr CMyStr255::MakeHybridString( MyStrPtr ioStr, UInt16 strSize )
{
	UInt16 byteToMakeNull=ioStr[0]+1;
	if( byteToMakeNull >= strSize )
		byteToMakeNull = strSize-1;

	ioStr[byteToMakeNull] - '\0';
	
	return( ioStr );
}

MyStrPtr CMyStr255::CStrToPStr( const char* inCStr, MyStrPtr outPStr, UInt16 inDestSize )
{
	UInt32 cStrLen = CStrLen( inCStr );
	UInt16 len = inDestSize;
	if( cStrLen < len )
		len = cStrLen;
	
	::BlockMoveData( inCStr, &outPStr[1], len );
	outPStr[0] = len;
	
	return( outPStr );
}

MyStrPtr CMyStr255::CStrToPStr( const char* inCStr, UInt16 inDestSize )
{
	return( CMyStr255::CStrToPStr( inCStr, (MyStrPtr)inCStr, inDestSize ) );
}

UInt32 CMyStr255::CStrLen( const char* inCStr )
{
	UInt32 i;
	for( i=0; inCStr[i] != '\0'; i++ )
		;
	return( i );
}

char* CMyStr255::PStrToCStr( ConstMyStrPtr inPStr, char* outCStr, UInt16 inDestSize )
{
	UInt8 numToMove = inPStr[0];
	if( numToMove > inDestSize-1 )
		numToMove = inDestSize-1;
	::BlockMoveData( &inPStr[1], outCStr, numToMove );
	outCStr[numToMove] = '\0';

	return outCStr;
}

char* CMyStr255::PStrToCStr( MyStrPtr inPStr, UInt16 inDestSize )
{
	return( CMyStr255::PStrToCStr( inPStr, (char*)inPStr, inDestSize ) );
}

CMyStr255 operator+( const CMyStr255& inLeft, const CMyStr255& inRight)
{
	CMyStr255 outString = inLeft;
	return outString += inRight;
}

CMyStr255 operator+( const CMyStr255& inLeft, ConstMyStrPtr inRight)
{
	CMyStr255 outString = inLeft;
	return outString += inRight;
}

CMyStr255 operator+( ConstMyStrPtr inLeft, const CMyStr255& inRight)
{
	CMyStr255 outString = inLeft;
	return outString += inRight;
}


