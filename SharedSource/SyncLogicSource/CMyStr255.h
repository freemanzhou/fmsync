// CMyStr255.h
// Created by Forest Hill on Thu, Mar 12, 1998 @ 12:25 PM.

#ifndef __CMyStr255__
#define __CMyStr255__

typedef unsigned char*			MyStrPtr;
typedef const unsigned char*	ConstMyStrPtr;

#define	kHybridDefault			true

class CMyStr255
{
public:
	CMyStr255(){ mIsHybrid = kHybridDefault; mStringData[0] = 0; };
	CMyStr255( ConstMyStrPtr inSource ){  mIsHybrid = kHybridDefault; this->Set( inSource ); };
	CMyStr255( const char* inCStr ){ mIsHybrid = kHybridDefault; this->Set( inCStr ); };
	CMyStr255( char inChar ){ mIsHybrid = kHybridDefault; this->Set( inChar ); };
	CMyStr255( short inListID, short inStrIndex ){ mIsHybrid = kHybridDefault; this->Set( inListID, inStrIndex ); };
	virtual ~CMyStr255(){};
	
	//local methods
	CMyStr255& 			Set( const CMyStr255& inSource );
	CMyStr255& 			Set( ConstMyStrPtr inSource );
	CMyStr255& 			Set( const char* inCStr );
	CMyStr255& 			Set( char inChar );
	CMyStr255&			Set( short inListID, short inStrIndex );
	CMyStr255& 			Assign( const CMyStr255& inSource ){ return this->Set( inSource ); };
	CMyStr255& 			Assign( ConstMyStrPtr inSource ){ return this->Set( inSource ); };
	CMyStr255& 			Assign( const char* inCStr ){ return this->Set( inCStr ); };
	CMyStr255& 			Assign( char inChar ){ return this->Set( inChar ); };
	CMyStr255&			Assign( short inListID, short inStrIndex ){ return this->Set( inListID, inStrIndex ); };
	CMyStr255& 			Append( const CMyStr255& inSource );
	CMyStr255& 			Append( ConstMyStrPtr inSource );
	CMyStr255& 			Append( char* inCStr );
	CMyStr255& 			Append( char inChar );
	CMyStr255&			Insert( const CMyStr255& inToInsert, UInt8 inInsertBefore );
	CMyStr255&			Insert( ConstMyStrPtr inToInsert, UInt8 inInsertBefore );
	CMyStr255&			Insert( char* inCStr, UInt8 inInsertBefore );
	char*				ToCStr( char* outCStr, UInt16 inDestSize );
	CMyStr255& 			MakeHybrid();
	
	unsigned char& 		operator[]( UInt8 inPosition) { return mStringData[inPosition]; };
						operator MyStrPtr(){ return (MyStrPtr)mStringData; }
	CMyStr255&			operator=( const CMyStr255& inSource ){ this->Set( inSource ); return *this; };
	CMyStr255&			operator=( ConstMyStrPtr inSource ){ this->Set( inSource ); return *this; };
	CMyStr255&			operator=( char* inCStr ){ this->Set( inCStr ); return *this; };
	CMyStr255&			operator=( char inChar ){ this->Set( inChar ); return *this; };
	CMyStr255&			operator+=( const CMyStr255& inToAppend ) { return this->Append(inToAppend); };
	CMyStr255&			operator+=( ConstMyStrPtr inToAppend ) { return this->Append(inToAppend); };
	CMyStr255&			operator+=( char* inToAppend ) { return this->Append(inToAppend); };
	CMyStr255&			operator+=( char inToAppend ) { return this->Append(inToAppend); };

	UInt8				Length(){ return mStringData[0]; };

	//static methods
	static MyStrPtr 	AppendPStr( MyStrPtr ioBase, ConstMyStrPtr inToAppend, UInt16 inBaseSize=sizeof( Str255) );
	static MyStrPtr		AppendChar( MyStrPtr ioBase, char inChar, UInt16 inBaseSize=sizeof( Str255) );
	static MyStrPtr		InsertIntoPStr( MyStrPtr ioBase, ConstMyStrPtr inToInsert, UInt8 insertBefore, UInt16 inBaseSize=sizeof( Str255) );
	static MyStrPtr 	CopyPStr( ConstMyStrPtr inSource, MyStrPtr outDest, UInt16 inDestSize=sizeof( Str255) );
	static MyStrPtr		MakeHybridString( MyStrPtr ioStr, UInt16 strSize=sizeof( Str255 ) );
	static MyStrPtr		CStrToPStr( const char* inCStr, MyStrPtr outPStr, UInt16 inDestSize=sizeof( Str255) );
	static MyStrPtr		CStrToPStr( const char* inCStr, UInt16 inDestSize );
	static char*		PStrToCStr( ConstMyStrPtr inPStr, char* outCStr, UInt16 inDestSize );
	static char*		PStrToCStr( MyStrPtr inPStr, UInt16 inDestSize=sizeof( Str255) );
	static UInt32 		CStrLen( const char* inCStr );
	
private:
	Str255		mStringData;
	Boolean		mIsHybrid;
};

CMyStr255			operator+( const CMyStr255& inLeft, const CMyStr255& inRight);
CMyStr255			operator+( const CMyStr255& inLeft, ConstMyStrPtr inRight);
CMyStr255			operator+( ConstMyStrPtr inLeft, const CMyStr255& inRight);
#endif
