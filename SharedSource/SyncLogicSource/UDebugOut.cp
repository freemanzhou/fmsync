#include "UDebugOut.h"
#include "Stringiness.h"
#ifdef _DEBUGOUTPUT

#include <string.h>
#include <UException.h>

UDebugOutPriv gUDebugOut;

UDebugOutPriv::UDebugOutPriv()
{
	outFile.name[0] = 0;
	outFileRef = kNoFileOpen;
	filesManaged = false;
}

UDebugOutPriv::~UDebugOutPriv()
{
	this->CloseOutFile();
}

void UDebugOutPriv::Output( ConstStr255Param fileName, const unsigned char* outText, long len )
{
	this->ManageFileStatus( fileName );

	filesManaged = false;

	if( outText != nil )
	{
		ThrowIfOSErr_( ::SetFPos( outFileRef, fsFromLEOF, 0 ) );
		ThrowIfOSErr_( ::FSWrite( outFileRef, &len, outText ) );
	}
}

void UDebugOutPriv::Output( ConstStr255Param fileName, const char* outText )
{
	this->ManageFileStatus( fileName );

	if( outText != nil )
		this->Output( fileName, (unsigned char*)outText, (long)strlen( outText ) );
}

void UDebugOutPriv::Output( ConstStr255Param fileName, ConstStr255Param outString )
{
	this->ManageFileStatus( fileName );

	if( outString != nil )
		this->Output( fileName, &outString[1], (long)outString[0] );
}

void UDebugOutPriv::Output( ConstStr255Param fileName, const vector<string>& theseStrings )
{
	int itemCount = 0;
	this->ManageFileStatus( fileName );

	vector<string>::const_iterator i = theseStrings.begin();
	while (i != theseStrings.end()) {
		if (itemCount > 0)
			this->Output( fileName, ", ");
		this->Output( fileName, (unsigned char *)i->data(), i->length() );
		++i;
		++itemCount;
	}
}

void UDebugOutPriv::Output( ConstStr255Param fileName, const vector<int>& theseInts )
{
	int itemCount = 0;
	this->ManageFileStatus( fileName );

	vector<int>::const_iterator i = theseInts.begin();
	while (i != theseInts.end()) {
		if (itemCount > 0)
			this->Output( fileName, ", ");
		string intAsString(AsString(*i));
		this->Output( fileName, (unsigned char *)intAsString.data(), intAsString.length() );
		++i;
		++itemCount;
	}
}

void UDebugOutPriv::Output( ConstStr255Param fileName, long outNum )
{
	this->ManageFileStatus( fileName );

	Str255 outString;
	NumToString( outNum, outString );
	this->Output( fileName, outString );
}

void UDebugOutPriv::ManageFileStatus( ConstStr255Param fileName )
{
	if( this->OutFileExists() )
	{
		//if the files have already been managed, then no work to be done
		if( filesManaged ) return;
		
		if( outFileRef != kNoFileOpen && !EqualString( outFile.name, fileName, false, false ) )
			this->CloseOutFile();
	}
	//create the file if necessary.  then open it.
	if( outFileRef == kNoFileOpen )
		this->CreateAndOpenLocalOutFile( fileName );

	//mark the files as managed
	filesManaged = true;
}

void UDebugOutPriv::CloseOutFile()
{
	filesManaged = false;
	if( outFileRef != kNoFileOpen )
	{
		short tmpFileRef = outFileRef;
		outFileRef = kNoFileOpen;
		ThrowIfOSErr_( :: FSClose( tmpFileRef ) );
	}
}

void UDebugOutPriv::CreateAndOpenLocalOutFile( ConstStr255Param outFileName )
{
	this->CloseOutFile();
	
	short vRefNum;
	long parID;
	ThrowIfOSErr_(FindFolder(kOnAppropriateDisk, kCurrentUserFolderType, false, &vRefNum, &parID));
	OSErr err = ::FSMakeFSSpec( vRefNum, parID, outFileName, &outFile );
	switch( err )
	{
		case noErr:
			if( !kAlwaysOverWrite )
				break;
			ThrowIfOSErr_( ::FSpDelete( &outFile ) );
			//fallthrough
		case fnfErr:
			ThrowIfOSErr_( ::FSpCreate( &outFile, kDebugFileCreator, kDebugFileType, smSystemScript ) );
			break;
		default:
			ThrowOSErr_( err );
			break;
	}
	
	ThrowIfOSErr_( ::FSpOpenDF( &outFile, fsRdWrPerm, &outFileRef ) );
}

Boolean UDebugOutPriv::OutFileExists()
{
	FSSpec tmpSpec;
	
	OSErr err = FSMakeFSSpec( outFile.vRefNum, outFile.parID, outFile.name, &tmpSpec );
	switch( err )
	{
		case noErr:
			return( true );
			break;
		case fnfErr:
			this->CloseOutFile();
			return( false );
			break;
		default:
			ThrowOSErr_( err );
			break;
	}
	return( false );
}

#endif