#pragma once

#include "WritePDB.h"
#include <vector>
#include <string>
#include "CFieldIDList.h"

class CDataSource;
class LStream;
class LStr255;
class CDatabaseInfo;

namespace JFile5 {

vector<string> SplitRepeatingFields(const string&, int repeatCount);
string JoinRepeatingFields(const vector<string>&, int repeatCount);

class CWriteJFile5 : public CWritePDB {
public:
			CWriteJFile5(CDataSource* sourceData, Boolean translateText);
	virtual	~CWriteJFile5();

	enum {kCreator = 'JFi5', kType = 'JfD5'};

	static void 	WriteAppInfoBlock(LStream* targetStream, const CFieldIDList& fieldIDs, 
										const CDatabaseInfo& info, const vector<int>& repeats, 
										Boolean translateText, Boolean needsSortOrCalc);
	static void 	WriteRecord(LStream* targetStream, const vector<string>& fields, Boolean translateText, vector<int>& repeats);
	static void		WritePopupChoices(LStream* targetStream, int fieldCount, const CFieldIDList& fieldIDs, const CDatabaseInfo& info, bool translateText);
	
	static UInt32	ExtraDataForCalculatedFields(UInt32 f1, UInt32 f2, UInt32 oper);
	
	static string	MakeFieldName(const string& baseName, int repeatIndex);

protected:
	
	static void 	WriteAppInfoBlock(LStream* targetStream, vector<string>& fieldNames, 
				const vector<int>& fieldWidths, const vector<int>& fieldTypes, const vector<string_vector>& popups, Boolean translateText);

	virtual void		WriteApplicationInfo(LStream& outStream);
	virtual UInt8		WriteRecord(UInt32 recordIndex, LStream& outStream);
	static char		IndexToChar(int i);

	static void WriteCalcExtra(LStream* targetStream, int fieldCount, const CFieldIDList& fieldIDs, 
				const CDatabaseInfo& info, bool translateText, const map<FMAE::FieldID, string>& theMap, const string& calcLabel);
	static void WriteCalcExtra1(LStream* targetStream, int fieldCount, const CFieldIDList& fieldIDs, const CDatabaseInfo& info, bool translateText);
	static void WriteCalcExtra2(LStream* targetStream, int fieldCount, const CFieldIDList& fieldIDs, const CDatabaseInfo& info, bool translateText);
	static void WriteOtherExtra(LStream* targetStream, const CDatabaseInfo& info);
};

}