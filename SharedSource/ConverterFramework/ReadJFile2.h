#pragma once
#error "JFile 2 not supported"
#include "ReadPDB.h"
#include "CFieldIDList.h"

class CDatabaseInfo;

class CReadJFile2 : public CReadPDB {
public:
			CReadJFile2(const FSSpec& sourceFile, Boolean translateText);
			~CReadJFile2();

static void		ReadRecord(LStream *stream, vector<string>& target, int fieldCount, Boolean translateText);
static Boolean	IsVersion2JFile(const FSSpec& spec);
static void		ExtractFromAppInfo(const void* appInfoPtr, bool translateText, const CFieldIDList& fieldIDs, CDatabaseInfo& info);

protected:
	virtual void		ValidateHeader(const PilotDatabaseHeader &header);
	virtual void		HandleApplicationInfo(const Ptr applicationInfo, UInt32 applicationInfoLength);
	virtual void		HandleRecord(const PilotRecordEntry& entry, const Ptr recordData, UInt32 recordLength);

};
