/*
	File:		GladPortDefs.h

	Contains:	compatibility defs for Windows code

	Written by:	Chris LeCroy, Jeff Pritchard

	Copyright:	© 1997 by Palm Computing, Inc., all rights reserved.

	Change History (most recent first):

		 <7>	10/15/97	csl		Merged from branch 3a1.
		 <2>	  9/9/97	csl		

	To Do:
*/

#pragma options align = mac68k

#pragma once

#ifndef __GladPortDefs__
#define __GladPortDefs__

#include <va_list.h>
#include <stdio.h>
#include <stdarg.h>

#include <Types.h>

#define _MAC macintosh
//the following is a bit skanky - but the original implementation equates 68k with mac
#define _68K_ macintosh

//Windows redefines ...
#define wsprintf	sprintf

#define ExportFunc
#define CALLBACK	

typedef unsigned short	PALMBOOL; // Changed to resolve type conflicts with Augustus
typedef	void*			PVOID;
typedef	unsigned long	UINT;
typedef	char*			LPSTR;
typedef	char			TCHAR;
typedef	const char*		LPCSTR;
typedef	const char*		LPCTSTR;
typedef	unsigned short	USHORT;
typedef char*			LPTSTR;

#define TRUE			true
#define FALSE			false

#define SYS_TRAP(void)
#define SYNC_API  
#define TRANS_API 

//other defs
typedef		unsigned short 	Word;
typedef		unsigned long	DWord;
typedef		unsigned short	Int;
typedef		unsigned long	UInt;
typedef		unsigned long	ULong;
typedef		unsigned long	Long;

typedef		char			CHAR;
typedef		unsigned char 	BYTE;
typedef		unsigned short	WORD;
typedef		short			SHORT;
#ifndef BuildingDevo	//lame...
typedef		unsigned long	LONG;
#endif
typedef		unsigned long	DWORD;

typedef		long			Err;
typedef		Boolean*		BooleanPtr;
typedef		void*			VoidPtr;
typedef		ULong*			ULongPtr;
typedef		DWord*			DWordPtr;	
typedef		Word*			WordPtr;
typedef		Handle			HANDLE;
typedef		Boolean			BOOLEAN;
typedef		long 			__int32;
typedef		unsigned long	HKEY;


typedef		unsigned long	CONDHANDLE;

typedef		Handle			HINSTANCE;


#define sysTicksPerSecond  60

#define CBR_300		300	
#define CBR_1200	1200
#define CBR_2400	2400
#define CBR_4800	4800
#define CBR_9600	9600
#define CBR_14400	14400
#define CBR_19200	19200
#define CBR_38400	38400
#define CBR_57600	57600

#define serErrorClass	25000	//our serial err starting number


#define _CrtIsValidPointer(ptr, len, validHeap) true
#define ErrDisplay(str)	debugstr(str)
#define ErrFatalDisplayIf(cond, str)	{if ((cond)) debugstr(str);}
#define ErrNonFatalDisplayIf(cond, str)	{if ((cond)) debugstr(str);}
#define _ASSERTE(cond)	{if (!(cond)) debugstr("_ASSERTE failed");}
#define _ASSERT(cond)	{if (!(cond)) debugstr("_ASSERT failed");}
#undef ASSERT
#define ASSERT(cond)	{if (!(cond)) debugstr("ASSERT failed");}

#define MAKELONG(hiWord, loWord)	((hiWord << 16) + loWord)

#endif //__GladPortDefs__

#pragma options align = reset