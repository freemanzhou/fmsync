#include "CRecord.h"
#include "CDatabase.h"
#include "Stringiness.h"
#ifdef CONDUIT
#include "CSyncRecord.h"
#endif

CRecordInfo::CRecordInfo()
	: fRecordID(0), fDeleted(false)
{
}

CRecordInfo::CRecordInfo(UInt32 recordID, const string& digest)
	: fRecordID(recordID), fDigest(digest), fDeleted(false)
{
}

CRecordInfo::CRecordInfo(const CRecord& record)
	: fRecordID(record.fRecordID), fDigest(record.fDigest), fDeleted(false)
{
}

bool operator==(const CRecordInfo&	inLhs, const CRecordInfo& inRhs)
{
	return
		(inLhs.fDigest == inRhs.fDigest) && 
		(inLhs.fRecordID == inRhs.fRecordID) && 
		(inLhs.fDeleted == inRhs.fDeleted);
}


CRecord::CRecord()
{
}

CRecord::CRecord(UInt32 recordID, const vector<string>& fields)
	: CRecordInfo(recordID, CDatabase::DigestFields(fields)), fFields(fields)
{
}

CRecord::CRecord(UInt32 recordID, const string& digest)
	: CRecordInfo(recordID, digest)
{
}

#ifdef CONDUIT
CRecord::CRecord(const vector<int>& fieldIDs, const CSyncRecord& inRecord)
{
	for (int fieldIndex = 0; fieldIndex < fieldIDs.size(); fieldIndex += 1) {
		fFields.push_back(inRecord.GetValue(fieldIDs[fieldIndex]));
	}
	fRecordID = inRecord.GetRemoteRecordID();
	fDigest = MakeDigest();
}
#endif

string
CRecord::MakeDigest()
{
	return CDatabase::DigestFields(fFields);
}

