#pragma once

#include "Writer.h"
#include "ReadPDB.h"
#include <vector>
#include <string>

class CDataSource;
class LStream;
class LStr255;

class CWritePDB : public CWriter{
public:
			CWritePDB(CDataSource* sourceData, Boolean translateText, UInt32 dbType, UInt32 dbCreator);
	virtual	~CWritePDB();
	
	enum {kCreator = 0x4A426173, kType = 0x4A624462};

	virtual void	DoWrite(LStream* targetStream, ConstStringPtr name = 0);
	virtual void	TargetFileName(ConstStringPtr sourceName, StringPtr name);
	virtual OSType	TargetFileCreator();
	virtual OSType	TargetFileType();

protected:
	
	virtual void		SetupHeader(UInt32 offsets, UInt32 dbType, UInt32 dbCreator, string dbName, PilotDatabaseHeader &header);
	virtual void		WriteApplicationInfo(LStream& outStream) = 0;
	virtual UInt8		WriteRecord(UInt32 recordIndex, LStream& outStream) = 0;

	vector<PilotRecordEntry>	fRecordEntries;
	PilotDatabaseHeader			fHeader;
	UInt32						fRecordCount;
	UInt32						fDbType;
	UInt32						fDbCreator;
};
