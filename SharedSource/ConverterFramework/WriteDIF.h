#pragma once
#include <string>
#include "Writer.h"

class CDataSource;
class LStream;
class LStr255;

class CWriteDIF : public CWriter {
public:
					CWriteDIF(CDataSource* sourceData);
	virtual			~CWriteDIF();

	virtual void	DoWrite(LStream* targetStream, ConstStringPtr name = 0);
	virtual void	TargetFileName(ConstStringPtr sourceName, StringPtr name);
	virtual OSType	TargetFileCreator();
	virtual OSType	TargetFileType();

protected:
	void			QuoteField(string& field);
	void			WriteHeader(LStream* targetStream, const string& dbName);
	void			WriteData(LStream* targetStream);
};

