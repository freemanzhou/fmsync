#pragma once

#include "Reader.h"
//#include "JFile1.h"

struct PilotDatabaseHeader { 
         char	sTitle[32]; // description of this collection 
         UInt16 wAttr;      // 8=request for backup
         UInt16 wVersion; 
         UInt32 tCreated; 
         UInt32 tModded;   // in "seconds since Jan 1, 1970" 
         UInt32 tLastHS;   // time of last hotsync
         long 	dwRes0;
         long 	ofsAttributes; //
         long 	ofsCategories; //
         UInt32 dwType;       // 'Data'
         UInt32 dwCreator;    // 'dwDP'
         UInt32 dwRes1;
         UInt32 dwRes2;      // reserved
         UInt16 nRecs;        // number of records 
};                          

struct PilotRecordEntry {
	UInt32 fileOffset;
	UInt32 uniqueID;
};

class CReadPDB : public CReader {
public:
			CReadPDB(const FSSpec& sourceFile, Boolean translateText);
			~CReadPDB();

	virtual void 		SourceBaseName(ConstStringPtr fileName, StringPtr baseName);
#ifdef CONVERTER_FACTORY
	virtual CWriter*	MakeWriter(const FSSpec& targetFile, CDataSource* theSource);
#endif

protected:
	virtual void		DoRead(LStream&);
	
	virtual void		ValidateHeader(const PilotDatabaseHeader &header) = 0;
	virtual void		HandleApplicationInfo(const Ptr applicationInfo, UInt32 applicationInfoLength) = 0;
	virtual void		HandleRecord(const PilotRecordEntry& entry, const Ptr recordData, UInt32 recordLength) = 0;

	Boolean						fTranslateText;

private:
	void				ReadHeader(LStream& inStream);
	void				ReadRecordEntries(LStream& inStream);
	void				ReadApplicationInfo(LStream& inStream);
	void				ReadRecords(LStream& inStream);

	vector<PilotRecordEntry>	fRecordEntries;
	PilotDatabaseHeader			fHeader;
	UInt32						fRecordCount;
};

const UInt32 kDeletedFlag = 0x80000000;
	 