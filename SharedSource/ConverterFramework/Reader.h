#pragma once

#include <PP_Prefix.h>
#include <string>
#include "CDatabase.h"

class CWriter;

class CReaderProgress {
public:
					CReaderProgress();
	virtual 		~CReaderProgress();
			
	virtual void	DoProgress(const string& progressString) = 0;
};

class CReader {
public:
			CReader(const FSSpec& targetFile, CReaderProgress* progress = 0);
			virtual ~CReader();
	
	CDatabase*	GetDatabase();
	
#ifdef CONVERTER_FACTORY
	virtual CWriter*	MakeWriter(const FSSpec& targetFile, CDataSource *theSource) = 0;
	virtual void		SourceBaseName(ConstStringPtr fileName, StringPtr baseName) = 0;
#endif
	virtual void		Read();
	void				DoProgress(const string& progressString);

protected:
	virtual void		DoRead(LStream& inStream) = 0;

	FSSpec				fSourceFile;
	CDatabase			fDatabase;
	CReaderProgress*	fProgress;
};
