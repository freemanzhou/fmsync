#include <cstring>

#include "JFile1.h"
#include "WritePDB.h"
#include "CDatabase.h"
#include "charset.h"
#include "Utilities.h"
#include "Stringiness.h"
#include "Str255.h"

CWritePDB::CWritePDB(CDataSource* sourceData, Boolean translateText, UInt32 dbType, UInt32 dbCreator)
	: CWriter(sourceData, translateText), fDbType(dbType), fDbCreator(dbCreator)

{
	fRecordCount = sourceData->RecordCount();
}

CWritePDB::~CWritePDB()
{
}

void
CWritePDB::SetupHeader(UInt32 offsets, UInt32 dbType, UInt32 dbCreator, string dbName, PilotDatabaseHeader& inHeader)
{
	UInt32 secs;
	UInt32 pilotSecs;
	::GetDateTime(&secs);
	DateTimeRec pilotStart;
	Clear(pilotStart);
	pilotStart.year = 1970;
	pilotStart.month = 1;
	pilotStart.day = 1;
	::DateToSeconds(&pilotStart, &pilotSecs);
	
	if (secs > pilotSecs)
		pilotSecs = secs - pilotSecs;
	
	inHeader.nRecs = fRecordCount;
	inHeader.dwType = dbType;
	inHeader.dwCreator = dbCreator;
	inHeader.wVersion = 0;            
	inHeader.wAttr = 8;
	inHeader.tCreated = secs;
	inHeader.tModded = secs;
	inHeader.ofsAttributes = offsets;
	strncpy(inHeader.sTitle, dbName.c_str(), 31);
	if (fTranslateText)
		ConvertToPilotText(inHeader.sTitle);
}

void
CWritePDB::DoWrite(LStream* targetStream, ConstStringPtr name)
{
	int entriesOffset = 78;
	int appInfoOffset = entriesOffset + 8*fRecordCount + 2;
	
	targetStream->SetLength(appInfoOffset);
	targetStream->SetMarker(appInfoOffset, streamFrom_Start);

	WriteApplicationInfo(*targetStream);
	if (targetStream->GetMarker() == appInfoOffset)
		appInfoOffset = 0;

	for(int i = 0 ; i < fRecordCount; i+= 1) {
		PilotRecordEntry entry;
		entry.fileOffset = targetStream->GetMarker();
		UInt32 attrib = WriteRecord(i, *targetStream);
		entry.uniqueID = attrib << 24;
		fRecordEntries.push_back(entry);
	}
	
	Clear(fHeader);
	string dbName;
	if (name) {
		dbName = AsString(name);
		strncpy(fHeader.sTitle, dbName.c_str(), 31);
	}
	else
		fHeader.sTitle[0] = 0;
	
	if (fTranslateText)
		ConvertToPilotText(fHeader.sTitle);
		
	SetupHeader(appInfoOffset, fDbType, fDbCreator, dbName, fHeader);

	targetStream->SetMarker(0, streamFrom_Start);
	targetStream->WriteBlock(&fHeader, sizeof(fHeader));
	targetStream->SetMarker(entriesOffset, streamFrom_Start);
	for(int i = 0 ; i < fRecordCount; i+= 1) {
		PilotRecordEntry entry = fRecordEntries[i];
		targetStream->WriteBlock(&entry.fileOffset, sizeof(entry.fileOffset));
		targetStream->WriteBlock(&entry.uniqueID, sizeof(entry.uniqueID));
	}
}

void
CWritePDB::TargetFileName(ConstStringPtr sourceName, StringPtr name)
{
	LString::CopyPStr(sourceName, name);
	LString::AppendPStr(name, "\p.pdb");
}

OSType
CWritePDB::TargetFileCreator()
{
	return 'Gld1';
}

OSType
CWritePDB::TargetFileType()
{
	return 'Gld0';
}