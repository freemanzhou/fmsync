#pragma once

#ifndef __UDebugOut__
#define __UDebugOut__

#ifdef _DEBUG
#define _DEBUGOUTPUT
#endif

#ifdef _DEBUGOUTPUT
#define	kDefaultDebugFileName	"\pUDebugOut.default"
#define	kDebugFileCreator		'CWIE'
#define	kDebugFileType			'TEXT'

#define	kAlwaysOverWrite		false

class UDebugOutPriv
{
	public:
		UDebugOutPriv();
		~UDebugOutPriv();

		void Output( ConstStr255Param fileName, const unsigned char* outText, long len );
		void Output( ConstStr255Param fileName, const char* outText );
		void Output( ConstStr255Param fileName, const vector<string>& theStrings);
		void Output( ConstStr255Param fileName, const vector<int>& theStrings);
		void Output( ConstStr255Param fileName, ConstStr255Param outString );
		void Output( ConstStr255Param fileName, long outNum );

	protected:
		void		ManageFileStatus( ConstStr255Param fileName );
		void		CloseOutFile();
		void		CreateAndOpenLocalOutFile( ConstStr255Param outFileName );
		Boolean		OutFileExists();

		typedef enum
		{
			kNoFileOpen
		};

		FSSpec		outFile;
		short		outFileRef;
		Boolean		filesManaged;
};
extern UDebugOutPriv gUDebugOut;

#define	DEBUG_OUT_DEF(text)							gUDebugOut.Output(kDefaultDebugFileName, text)
#define	DEBUG_OUT(filename, text)					gUDebugOut.Output(filename, text)
#define	DEBUG_OUT_LEN(filename, text, len)			gUDebugOut.Output(filename, text, len)
#define	DEBUG_OUT_NO_TS_DEF(text)					DEBUG_OUT(kDefaultDebugFileName, text)
#define	DEBUG_OUT_NO_TS(filename, text)				DEBUG_OUT(filename, text)

#else
#define	DEBUG_OUT_DEF( text)				{}
#define	DEBUG_OUT(filename, text)			{}
#define	DEBUG_OUT_LEN(filename, text, len)	{}
#define	DEBUG_OUT_NO_TS_DEF(text)			{}
#define	DEBUG_OUT_NO_TS(filename, text)		{}
#endif

#endif