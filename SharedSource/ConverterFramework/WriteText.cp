#include "JFile.h"
#include "WriteText.h"
#include "CDatabase.h"
#include "ConverterErrors.h"

const int kSafeMaxStringLength = 128;
const int kSafeMaxLineLength = 1280;

CWriteText::CWriteText(CDataSource* sourceData, char delimeter, Boolean writeFields)
	: CWriter(sourceData, false), fDelimeter(delimeter), fWriteFields(writeFields)
{
}

CWriteText::~CWriteText()
{
}

void
CWriteText::DoWrite(LStream* targetStream, ConstStringPtr)
{
	int recordCount = fDatabase->RecordCount();
	
	// write the field names
	char newLineChar = '\r';
	
	int fieldCount = fDatabase->FieldCount();
	if (fWriteFields) {
		vector<string> fieldNames;
		fDatabase->GetFieldNames(fieldNames);
		for (int i = 0; i < fieldCount; i += 1) {
			string fieldName = fieldNames[i];
			QuoteField(fieldName);
			targetStream->WriteBlock(fieldName.data(), fieldName.length());
			if (i < fieldCount - 1)
				targetStream->WriteBlock(&fDelimeter, sizeof(fDelimeter));
		}
		targetStream->WriteBlock(&newLineChar, sizeof(newLineChar));
	}

	// write the field data

	for(int i = 0 ; i < recordCount; i+= 1) {
		vector<string> theRecord;
		fDatabase->GetRecord(i, theRecord);
		for(int j = 0; j< fieldCount; j++) {
			string fieldData = theRecord[j];
			QuoteField(fieldData);
	 	 	targetStream->WriteBlock(fieldData.data(), fieldData.length());
			if (j < fieldCount - 1)
				targetStream->WriteBlock(&fDelimeter, sizeof(fDelimeter));
	 	}
		targetStream->WriteBlock(&newLineChar, sizeof(newLineChar));
	 }
}

void
CWriteText::TargetFileName(ConstStringPtr sourceName, StringPtr name)
{
	LString::CopyPStr(sourceName, name);
	LString::AppendPStr(name, "\p.txt");
}

OSType
CWriteText::TargetFileCreator()
{
	return 'ttxt';
}

OSType
CWriteText::TargetFileType()
{
	return 'TEXT';
}

#pragma mark ===== CWriteTabbedText =====

CWriteTabbedText::CWriteTabbedText(CDataSource* sourceData, Boolean writeFields)
	: CWriteText(sourceData, '\t', writeFields)
{
}

CWriteTabbedText::~CWriteTabbedText()
{
}

void
CWriteTabbedText::QuoteField(string& inField)
{
	int fieldLength = inField.length();
	int maxLength = fieldLength + 1;
	StPointerBlock block(maxLength);
	char *resultP = block;
	const char tabCharacter = '"';
	const char *field = inField.c_str();
	const char *endPtr = field + fieldLength;
	const char *fieldPtr = field;
	char delim = fDelimeter;
	Boolean hasTab = false;
	
	while (fieldPtr < endPtr && !hasTab) {
		char c = *fieldPtr;
		if (c == delim || c == 13 || c == 10)
			hasTab = true;
		++fieldPtr;
	}
	
	if (hasTab) {
		fieldPtr = field;
		while (fieldPtr < endPtr) {
			char c = *fieldPtr++;
			if (c == tabCharacter)
				*resultP++ = ' ';
			else if (c == 13 || c == 10)
				*resultP++ = verticalTab;
			else
				*resultP++ = c;
		}
		*resultP = 0;
		inField = string((char*)Ptr(block));
	}
}

#pragma mark ===== CWriteCSVText =====

const char csvDelim = ',';

CWriteCSVText::CWriteCSVText(CDataSource* sourceData, Boolean writeFields)
	: CWriteText(sourceData, csvDelim, writeFields)
{
}

CWriteCSVText::~CWriteCSVText()
{
}

void
CWriteCSVText::QuoteField(string& inField)
{
	int fieldLength = inField.length();
	int maxLength = fieldLength * 2;
	StPointerBlock block(maxLength);
	char *resultP = block;
	const char *field = inField.c_str();
	const char *endPtr = field + fieldLength;
	const char *fieldPtr = field;
	Boolean hasComma = false;
	Boolean hasReturn = false;
	Boolean hasQuote = false;
	
	while (fieldPtr < endPtr && !hasComma && !hasReturn) {
		char c = *fieldPtr;
		if (c == 13 || c == 10)
			hasReturn = true;
		else if (c == csvDelim)
			hasComma = true;
		++fieldPtr;
	}
	
	if (hasComma || hasReturn) {
		fieldPtr = field;
		*resultP++ = quoteCharacter;
		while (fieldPtr < endPtr) {
			char c = *fieldPtr++;
			if (c == quoteCharacter) {
				*resultP++ = quoteCharacter;
				*resultP++ = c;
			} else if (c == 13 || c == 10) {
				*resultP++ = verticalTab;
			} else {
				*resultP++ = c;
			}
		}
		*resultP++ = quoteCharacter;
		*resultP = 0;
		inField = string((char*)Ptr(block));
	}
}

