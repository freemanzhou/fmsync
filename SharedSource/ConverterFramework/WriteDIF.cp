#include "WriteDIF.h"
#include "CDatabase.h"
#include "ConverterErrors.h"
#include "Stringiness.h"
#include "Str255.h"
#include "charset.h"

CWriteDIF::CWriteDIF(CDataSource* sourceData)
	: CWriter(sourceData, false)
{
}

CWriteDIF::~CWriteDIF()
{
}

static void
PrintString(LStream* targetStream, const string& aString)
{
	targetStream->WriteBlock(aString.data(), aString.length());
}

static void
PrintStringWithQuotes(LStream* targetStream, const string& aString)
{
	char c = '"';
	targetStream->WriteBlock(&c, 1);
	targetStream->WriteBlock(aString.data(), aString.length());
	targetStream->WriteBlock(&c, 1);
}

static void
PrintEntry(LStream* targetStream, const string& label, int v1, int v2, const string& stringValue)
{
	PrintString(targetStream, label);
	PrintString(targetStream, "\r");

	string vectorsValue(AsString(v1) + "," + AsString(v2));
	PrintString(targetStream, vectorsValue);
	PrintString(targetStream,  "\r");

	PrintStringWithQuotes(targetStream, stringValue);
	PrintString(targetStream,  "\r");
}

static void
PrintEntry(LStream* targetStream, const string& label, int v1, int v2)
{
	string noValue;
	PrintEntry(targetStream, label, v1, v2, noValue);
}

void
CWriteDIF::WriteHeader(LStream* targetStream, const string& dbName)
{
	int fieldCount = fDatabase->FieldCount();
	int recordCount = fDatabase->RecordCount();

	// TABLE
	PrintEntry(targetStream, "TABLE", 0, 1, dbName);

	// VECTORS
	PrintEntry(targetStream, "VECTORS", 0, fieldCount);
	
	// TUPLES
	PrintEntry(targetStream, "TUPLES", 0, recordCount);

	vector<string> fieldNames;
	fDatabase->GetFieldNames(fieldNames);
	for (int i = 0; i < fieldCount; i += 1) {
		string fieldName = fieldNames[i];
		QuoteField(fieldName);
		PrintEntry(targetStream, "LABEL", i+1, 0, fieldName);
	}

	PrintEntry(targetStream, "DATA", 0, 0);
}

void
CWriteDIF::WriteData(LStream* targetStream)
{
	int fieldCount = fDatabase->FieldCount();
	int recordCount = fDatabase->RecordCount();
	for(int i = 0 ; i < recordCount; i+= 1) {
		PrintString(targetStream, "-1,0\rBOT\r");
		vector<string> theRecord;
		fDatabase->GetRecord(i, theRecord);
		for(int j = 0; j< fieldCount; j++) {
			string fieldData = theRecord[j];
			QuoteField(fieldData);
			PrintString(targetStream, "1,0\r");
			PrintStringWithQuotes(targetStream, fieldData);
			PrintString(targetStream,  "\r");
	 	}
	 }
	PrintString(targetStream, "-1,0\rEOD\r");
}

void
CWriteDIF::DoWrite(LStream* targetStream, ConstStringPtr inName)
{
	string dbName(AsString(inName));
	WriteHeader(targetStream, dbName);
	WriteData(targetStream);
}

void
CWriteDIF::TargetFileName(ConstStringPtr sourceName, StringPtr name)
{
	LString::CopyPStr(sourceName, name);
	LString::AppendPStr(name, "\p (DIF)");
}

OSType
CWriteDIF::TargetFileCreator()
{
	return 'ttxt';
}

OSType
CWriteDIF::TargetFileType()
{
	return 'TEXT';
}

void
CWriteDIF::QuoteField(string& inField)
{
	static char buffer[4096];
	int fieldLength = inField.length();
	char *resultP = buffer;
	const char tabCharacter = '"';
	const char *field = inField.c_str();
	const char *endPtr = field + fieldLength;
	const char *fieldPtr = field;
	char delim = '"';
	Boolean hasQuote = false;
	
	while (fieldPtr < endPtr && !hasQuote) {
		char c = *fieldPtr++;
		if (c == delim || c == 10 || c == 13)
			hasQuote = true;
	}
	
	if (hasQuote) {
		fieldPtr = field;
		while (fieldPtr < endPtr) {
			char c = *fieldPtr++;
			if (c == delim)
				*resultP++ = '\'';
			else {
				if (c == 10 || c == 13) {
					//*resultP++ = '"';
					*resultP++ = verticalTab;
					//*resultP++ = '"';
				} else
					*resultP++ = c;
			}
		}
		*resultP = 0;
		inField = string(buffer);
	}
}

