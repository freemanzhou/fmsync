#pragma once
#include "ReadPDB.h"
#include "CFieldIDList.h"

class CDatabaseInfo;

class CReadJFilePro : public CReadPDB {
public:
			CReadJFilePro(const FSSpec& sourceFile, Boolean translateText);
			~CReadJFilePro();

static Boolean	IsVersionProJFile(const FSSpec& spec);
static int		CharToIndex(unsigned char c);

static void		ReadRecord(LStream *stream, vector<string>& target, int fieldCount, Boolean translateText);
static void		ExtractFromAppInfo(const void* appInfoPtr, bool translateText, const CFieldIDList& fieldIDs, CDatabaseInfo& info);
static void		ExtractPopups(const void* inAppInfoPtr, bool translateText, const CFieldIDList& fieldIDs, CDatabaseInfo& info);

protected:
	virtual void		ValidateHeader(const PilotDatabaseHeader &header);
	virtual void		HandleApplicationInfo(const Ptr applicationInfo, UInt32 applicationInfoLength);
	virtual void		HandleRecord(const PilotRecordEntry& entry, const Ptr recordData, UInt32 recordLength);
};
