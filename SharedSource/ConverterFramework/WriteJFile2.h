#pragma once

#include "WritePDB.h"
#include <vector>
#include <string>
#include "CFieldIDList.h"

class CDataSource;
class LStream;
class LStr255;
class CDatabaseInfo;

class CWriteJFile2 : public CWritePDB {
public:
			CWriteJFile2(CDataSource* sourceData, Boolean translateText);
	virtual	~CWriteJFile2();
	
	enum {kCreator = 0x4A426173, kType = 0x4A624462};

	static void 	WriteAppInfoBlock(LStream* targetStream, const CFieldIDList& fieldIDs, const CDatabaseInfo& info, Boolean translateText);
	static void 	WriteAppInfoBlock(LStream* targetStream, vector<string>& fieldNames, 
				const vector<int>& fieldWidths, const vector<int>& fieldTypes, const vector<string_vector>& popups, Boolean translateText);
	static void 	WriteAppInfoBlock(LStream* targetStream, vector<string>& fieldNames, 
				const vector<int>& fieldWidths, Boolean translateText);
	static void 	WriteAppInfoBlock(LStream* targetStream, 
				vector<string>& fieldNames, Boolean translateText);
	static void 	WriteRecord(LStream* targetStream, const vector<string>& fields, Boolean translateText);
	static void		WritePopupChoices(LStream* targetStream, int fieldCount, const CFieldIDList& fieldIDs, const CDatabaseInfo& info, bool translateText);

protected:
	virtual void		WriteApplicationInfo(LStream& outStream);
	virtual UInt8		WriteRecord(UInt32 recordIndex, LStream& outStream);
};
