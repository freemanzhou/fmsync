#pragma once

#include "FMAE.h"
#include "CFieldIDList.h"
#include "CDatabaseInfo.h"
#include "FMDatabasePtr.h"

class CFMFieldTyper {
public:
				CFMFieldTyper(FileMaker::DatabasePtr sourceLayout);
				
				
	CDatabaseInfo	GetDatabaseInfo() const;

	static bool TypeDeterminationNeedsData(int fieldType);
	static int JFileFromFMType(int fieldType, const string& sampleData, bool hasChoices);
	static void ExtraFromType(int fieldType, int& extra1, int& extra2);

private:
	void			SetupDefaultFieldMaps();
	void			SetupDefaultFieldTypes();
	static bool		StringIsTime(const string&);
	string			FindField(const FMAE::FieldID& fieldID);

	FileMaker::DatabasePtr			fDatabase;
	CDatabaseInfo					fInfo;
	map<FMAE::FieldID, string>		fCachedFields;
	map<FMAE::FieldID, bool>		fFieldCached;
	CFieldIDList					fFieldIDs;
	vector<int>						fAllRecordIDs;
	map<FMAE::FieldID,int>			fJFileFieldTypes;

};
