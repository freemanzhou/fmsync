#pragma once
#include <string>
#include "Writer.h"
#include "charset.h"

class CDataSource;
class LStream;
class LStr255;

const char quoteCharacter = '"';

class CWriteText : public CWriter {
public:
	virtual			~CWriteText();

	virtual void	DoWrite(LStream* targetStream, ConstStringPtr name = 0);
	virtual void	TargetFileName(ConstStringPtr sourceName, StringPtr name);
	virtual OSType	TargetFileCreator();
	virtual OSType	TargetFileType();

protected:
	virtual void	QuoteField(string& field) = 0;

					CWriteText(CDataSource* sourceData, char delimeter, Boolean writeFields = true);

	Boolean			fWriteFields;	
	char			fDelimeter;
};

class CWriteTabbedText : public CWriteText {
public:
	CWriteTabbedText(CDataSource* sourceData, Boolean writeFields = true);
	virtual ~CWriteTabbedText();

protected:
	virtual void	QuoteField(string& field);
};

class CWriteCSVText : public CWriteText {
public:
	CWriteCSVText(CDataSource* sourceData, Boolean writeFields = true);
	virtual ~CWriteCSVText();

protected:
	virtual void	QuoteField(string& field);
};
