#pragma once

#include "WritePDB.h"
#include <vector>
#include <string>

class CDataSource;
class LStream;
class LStr255;

class CWriteJFile1 : public CWritePDB {
public:
			CWriteJFile1(CDataSource* sourceData, Boolean translateText);
	virtual	~CWriteJFile1();
	
	enum {kCreator = 0x4A426173, kType = 0x4A624462};

	static void 	WriteAppInfoBlock(LStream* targetStream, vector<string>& fieldNames, 
				const vector<int>& fieldWidths, Boolean translateText);
	static void 	WriteAppInfoBlock(LStream* targetStream, 
				vector<string>& fieldNames, Boolean translateText);
	static void 	WriteRecord(LStream* targetStream, const vector<string>& fields, Boolean translateText);

protected:
	virtual void		WriteApplicationInfo(LStream& outStream);
	virtual UInt8		WriteRecord(UInt32 recordIndex, LStream& outStream);
};
