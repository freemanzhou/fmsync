#include "ReadPDB.h"
#include "CDatabase.h"
#include "charset.h"
#include "ConverterErrors.h"
#ifdef CONVERTER_FACTORY
#include "ConverterFactory.h"
#endif
CReadPDB::CReadPDB(const FSSpec& sourceFile, Boolean translateText)
	: CReader(sourceFile), fTranslateText(translateText)
{
}

CReadPDB::~CReadPDB()
{
}

void
CReadPDB::DoRead(LStream& inStream)
{
	ReadHeader(inStream);
	ValidateHeader(fHeader);
	ReadRecordEntries(inStream);
	ReadApplicationInfo(inStream);
	ReadRecords(inStream);
}

void
CReadPDB::SourceBaseName(ConstStringPtr fileName, StringPtr baseName)
{
	LStr255 destName = fileName;
	LStr255 upName = destName;
	LStr255 upExten = "\p.pdb";
	UpperString(upName, false);
	UpperString(upExten, false);

	UInt8 location = upName.ReverseFind(upExten);
	if (location) { 
		destName.Remove(location, 4);
	}
	LString::CopyPStr(destName, baseName);
}

#ifdef CONVERTER_FACTORY
CWriter*
CReadPDB::MakeWriter(const FSSpec& targetFile, CDataSource* theSource)
{
	return CConverterFactory::GetConverterFactory()->MakeTextWriter(theSource, targetFile);
}
#endif

void
CReadPDB::ReadHeader(LStream& inStream)
{
	inStream.ReadBlock(&fHeader, 78);
}

void
CReadPDB::ReadRecordEntries(LStream& inStream)
{
	inStream.SetMarker(78, streamFrom_Start);
	fRecordCount = fHeader.nRecs;
	int numEntries = fRecordCount;
	for (int i = 0; i < numEntries; i += 1) {
		PilotRecordEntry recordEntry;
		inStream.ReadBlock(&recordEntry, sizeof(recordEntry));
		if(recordEntry.uniqueID & 0x80000000)
		 	fRecordCount -= 1;
		fRecordEntries.push_back(recordEntry);
	}
	PilotRecordEntry lastEntry;
	lastEntry.fileOffset = inStream.GetLength();
	lastEntry.uniqueID = 0;
	fRecordEntries.push_back(lastEntry);
	UInt16 oddity;
	inStream.ReadBlock(&oddity, sizeof(oddity));		
}

void
CReadPDB::ReadApplicationInfo(LStream& inStream)
{
	if (fRecordEntries.size() == 0)
		return;

	UInt32 appInfoSize;
	PilotRecordEntry entry = fRecordEntries[fHeader.nRecs - 1];
	if (fHeader.ofsAttributes > entry.fileOffset) {
		appInfoSize = inStream.GetLength() - fHeader.ofsAttributes;
		inStream.SetMarker(fHeader.ofsAttributes, streamFrom_Start);
	} else {
		entry = fRecordEntries[0];
		appInfoSize = entry.fileOffset - inStream.GetMarker();
	}
	StPointerBlock applicationInfo(appInfoSize);
	inStream.ReadBlock(applicationInfo, appInfoSize);
	HandleApplicationInfo(applicationInfo, appInfoSize);
}

void
CReadPDB::ReadRecords(LStream& inStream)
{
	int numEntries = fHeader.nRecs;
	PilotRecordEntry entry = fRecordEntries[0];
	for (int i = 0; i < numEntries; i += 1) {
		PilotRecordEntry nextEntry = fRecordEntries[i+1];
		UInt32 recordLength = nextEntry.fileOffset - entry.fileOffset;
		StPointerBlock recordData(recordLength);
		inStream.SetMarker(entry.fileOffset, streamFrom_Start);
		inStream.ReadBlock(recordData, recordLength);
		HandleRecord(entry, recordData, recordLength);
		entry = nextEntry;
	}
}
