#pragma once

#include "WritePDB.h"
#include <vector>
#include <string>
#include "CFieldIDList.h"

class CDataSource;
class LStream;
class LStr255;
class CDatabaseInfo;

class CWriteJFilePro : public CWritePDB {
public:
			CWriteJFilePro(CDataSource* sourceData, Boolean translateText);
	virtual	~CWriteJFilePro();

	enum {kCreator = 'JFil', kType = 'JfDb'};

	static void 	WriteAppInfoBlock(LStream* targetStream, const CFieldIDList& fieldIDs, 
										const CDatabaseInfo& info, Boolean translateText);
	static void 	WriteRecord(LStream* targetStream, const vector<string>& fields, Boolean translateText);
	static void		WritePopupChoices(LStream* targetStream, int fieldCount, const CFieldIDList& fieldIDs, const CDatabaseInfo& info, bool translateText);

protected:
	
	static void 	WriteAppInfoBlock(LStream* targetStream, vector<string>& fieldNames, 
				const vector<int>& fieldWidths, const vector<int>& fieldTypes, const vector<string_vector>& popups, Boolean translateText);

	virtual void		WriteApplicationInfo(LStream& outStream);
	virtual UInt8		WriteRecord(UInt32 recordIndex, LStream& outStream);
	static char		IndexToChar(int i);
};
