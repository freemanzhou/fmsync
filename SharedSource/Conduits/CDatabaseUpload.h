#pragma once

class CConduitSettings;

class CDatabaseUpload {
public:
	CDatabaseUpload(CConduitSettings* settings, const map<string,bool>&); 
	virtual ~CDatabaseUpload();

	void UploadDatabases();
private:

	CConduitSettings* fSettings;
	map<string,bool> fProDBs;
};
