#pragma once

#include <string>
#include <map>

class CRecord;
class CSyncRecord;
class CDatabase;

class CRecordInfo {
public:
					CRecordInfo();
					CRecordInfo(const CRecord& record);
					CRecordInfo(UInt32 recordID, const string& digest);

		friend bool operator == (const CRecordInfo&, const CRecordInfo&);

		string		fDigest;
		UInt32		fRecordID;
		bool		fDeleted;
};

class CRecord : public CRecordInfo {
public:
					CRecord();
					CRecord(const vector<int>& fieldIDs, const CSyncRecord& inRecord);
					CRecord(UInt32 recordID, const vector<string>& fields);
					CRecord(UInt32 recordID, const string& digest);

	string			MakeDigest();
	
	friend bool operator == (const CRecord&, const CRecord&);

	vector<string>	fFields;
};
