/*
	File:		LGladFile.cp

	Contains:	xxx put contents here xxx

	Created by:	Chris LeCroy on Mon, Sep 15, 1997

	Copyright:	© 1997 by Now Software, Inc., all rights reserved.

	Change History (most recent first):

				 00/00/00	---	file 	created

	To Do:
*/


#include "LGladFile.h"



LGladFile::LGladFile() {
}


LGladFile::~LGladFile()
{
}


Int16 LGladFile::OpenOrCreateResourceFork(  Int16  inPrivileges,  OSType  inCreator,  OSType  inFileType,  ScriptCode inScriptCode)
{
	// try opening the resource fork
	if (!this->Exists())
	{
		// file didnÕt exist, letÕs create it			
		this->CreateNewFile (inCreator, inFileType, inScriptCode);
	} 
		
	return this->OpenResourceFork(inPrivileges);;
}



Int16 LGladFile::OpenOrCreateDataFork( Int16 inPrivileges,
										OSType  inCreator,
										OSType  inFileType,
										ScriptCode inScriptCode)
{
	// try opening the resource fork
	if (!this->Exists())
	{
		// file didnÕt exist, letÕs create it			
		this->CreateNewDataFile(inCreator, inFileType, inScriptCode);
	} 
		
	return this->OpenDataFork(inPrivileges);;
}



Boolean LGladFile::Exists()
{
	FSSpec fspec = {};
	
	OSErr err = ::FSMakeFSSpec(mMacFileSpec.vRefNum, mMacFileSpec.parID, mMacFileSpec.name, &fspec);
	if (err != fnfErr)
		ThrowIfOSErr_(err);
		
	return (err != fnfErr);
}



void LGladFile::DeleteFile()
{
	this->CloseDataFork();
	this->CloseResourceFork();
	
	ThrowIfOSErr_(::FSpDelete(&mMacFileSpec));
	
	mMacFileSpec.vRefNum = 0;	// ??? Is this a good default for
	mMacFileSpec.parID = 0;		//  a File Spec ???
	mMacFileSpec.name[0] = 0;
}



void LGladFile::Duplicate(LFile* inDestinationFile)
{
	FSSpec destSpec = {};
	HParamBlockRec pb = {};

	ThrowIfNil_(inDestinationFile);

	inDestinationFile->GetSpecifier(destSpec);
	
	pb.copyParam.ioVRefNum = mMacFileSpec.vRefNum;
	pb.copyParam.ioDirID = mMacFileSpec.parID;
	pb.copyParam.ioNamePtr = mMacFileSpec.name;
	pb.copyParam.ioDstVRefNum = destSpec.vRefNum;
	pb.copyParam.ioNewDirID = destSpec.parID;
	pb.copyParam.ioNewName = destSpec.name;
	pb.copyParam.ioCopyName = nil;
	ThrowIfNil_(::PBHCopyFileSync(&pb));
}



Int32  LGladFile::GetLength() const
{
	Int32	theLength;
	OSErr	err = ::GetEOF(this->GetDataForkRefNum(), &theLength);
	ThrowIfOSErr_(err);
	return theLength;
}



Int32  LGladFile::GetCurrentFilePos() const
{
	Int32	theMarker;
	OSErr	err = ::GetFPos(this->GetDataForkRefNum(), &theMarker);
	ThrowIfOSErr_(err);
	return theMarker;
}



void  LGladFile::SetLength(  Int32  inLength)
{
	OSErr	err = ::SetEOF(this->GetDataForkRefNum(), inLength);
	ThrowIfOSErr_(err);
}



LGladFile::LGladFile(  const FSSpec  &inFileSpec) :
	LFile(inFileSpec)
{
	
}



void LGladFile::ReadBytes( void  *outBuffer,  Int32  &ioByteCount)
{
	ThrowIfOSErr_(::FSRead(GetDataForkRefNum(), &ioByteCount, outBuffer));
}



void LGladFile::WriteBytes(  const void  *inBuffer,  Int32  &ioByteCount)
{
	ThrowIfOSErr_(::FSWrite(GetDataForkRefNum(), &ioByteCount, inBuffer));
}


