/*
	File:		LGladFile.h

	Contains:	xxx put contents here xxx

	Created by:	Chris LeCroy on Mon, Sep 15, 1997

	Copyright:	© 1997 by Now Software, Inc., all rights reserved.

	Change History (most recent first):

				 00/00/00	---	file created

	To Do:
*/

#pragma once

#ifndef __LGladFile__
#define __LGladFile__

#ifndef __LFile__
#include "LFile.h"
#endif


class LGladFile : public LFile
{
public:
	LGladFile();
	LGladFile( const FSSpec  &inFileSpec);
	virtual ~LGladFile();
	
	virtual SInt16 OpenOrCreateResourceFork( SInt16 inPrivileges,
											OSType  inCreator,
											OSType  inFileType,
											ScriptCode inScriptCode);
	
	virtual SInt16 OpenOrCreateDataFork( SInt16 inPrivileges,
										OSType  inCreator,
										OSType  inFileType,
										ScriptCode inScriptCode);
	virtual Boolean Exists();

	virtual void DeleteFile();

	virtual void Duplicate(LFile* inDestinationFile);
	virtual SInt32  GetLength() const;

	virtual SInt32  GetCurrentFilePos() const;

	virtual void  SetLength(  SInt32  inLength);
	virtual void ReadBytes( void  *outBuffer,  SInt32  &ioByteCount);

	virtual void WriteBytes(  const void  *inBuffer,  SInt32  &ioByteCount);


};

#endif	//__LGladFile__
