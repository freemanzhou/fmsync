#pragma once

#include <PP_Prefix.h>

class CDataSource;
class LStream;
class LStr255;

class CWriter {
public:
			CWriter(CDataSource* sourceData, Boolean translateText);
	virtual		~CWriter();

	virtual void	Write(LStream* targetStream, ConstStringPtr name = 0);
	virtual void	DoWrite(LStream* targetStream, ConstStringPtr name = 0) = 0;
	virtual void	TargetFileName(ConstStringPtr sourceName, StringPtr name) = 0;
	virtual OSType	TargetFileCreator() = 0;
	virtual OSType	TargetFileType() = 0;

protected:

	CDataSource*	fDatabase;
	Boolean			fTranslateText;
};
