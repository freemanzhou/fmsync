#pragma once
#include "ReadPDB.h"
#include "CFieldIDList.h"

class CDatabaseInfo;

class CReadJFile5 : public CReadPDB {
public:
			CReadJFile5(const FSSpec& sourceFile, Boolean translateText);
			~CReadJFile5();
			enum {kMaxFields = 50};

static Boolean	IsVersion5JFile(const FSSpec& spec);
static int		CharToIndex(unsigned char c);

static void		ReadRecord(LStream *stream, vector<string>& target, int fieldCount, Boolean translateText, const vector<int>&);
static void		ExtractFromAppInfo(const void* inAppInfoPtr, int dataLength, bool translateText, const CFieldIDList& fieldIDs, const vector<int>& repeats, CDatabaseInfo& info);
static void		ExtractExtraData(const void* inAppInfoPtr, const void* limitP, bool translateText, const CFieldIDList& fieldIDs, CDatabaseInfo& info);
static const char* ExtractPopups(const char *p, const void* limitP, bool translateText, const CFieldIDList& fieldIDs, CDatabaseInfo& info);

protected:
	virtual void		ValidateHeader(const PilotDatabaseHeader &header);
	virtual void		HandleApplicationInfo(const Ptr applicationInfo, UInt32 applicationInfoLength);
	virtual void		HandleRecord(const PilotRecordEntry& entry, const Ptr recordData, UInt32 recordLength);
};
