#pragma once

class CUploadRequest {
public:
	CUploadRequest();
	~CUploadRequest();
	CUploadRequest(const string& databaseName, bool changesOnly = false);
	CUploadRequest(const CUploadRequest&);
	
	void SetLocation(const FSSpec&);
	bool GetLocation(FSSpec&);

	CUploadRequest&	operator=(const CUploadRequest&);
	void			swap(CUploadRequest& inOther);

	string fName;
	string fMacName;
	Handle fAlias;
	bool fChanges;
	bool fTranslateText;
};

namespace BinaryFormat {

void Read(LStream&, CUploadRequest&);
void Write(LStream&, const CUploadRequest&);

}

