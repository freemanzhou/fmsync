#include <UInternet.h>

//#include "JFile.h"
//#include "ReadPDB.h"
#include "CDatabase.h"
#include "Stringiness.h"
#include "Utilities.h"

CDataSource::CDataSource()
{
}

CDataSource::~CDataSource()
{
}

Boolean
CDataSource::FieldNamesKnown()
{
	return fFieldNamesKnown;
}

void
CDataSource::SetFieldNamesKnown(Boolean known)
{
	fFieldNamesKnown = known;
}

int
CDataSource::RecordSize(int recordNumber)
{
	vector<string> record;
	
	GetRecord(recordNumber, record);
	
	int startIndex = 0;
	int endIndex = record.size();
	int recordSize = 0;
	for (int i = 0; i < endIndex; i += 1)
		recordSize += (record[i].length() + 1);

	return recordSize;
}

string
CDataSource::RecordDigest(int recordNumber)
{
	vector<string> record;
	
	GetRecord(recordNumber, record);
	return DigestFields(record);
}

static string DigestFieldsSafe(const vector<string>& fields)
{
	const string kDelimString("\x03");
	string fieldsWithDelimeters;
	for (vector<string>::const_iterator i = fields.begin(); i != fields.end(); ++i) {
		fieldsWithDelimeters.append(*i);
		fieldsWithDelimeters.append(kDelimString);
	}
	MD5_CTX theDigest;
	MD5Init(&theDigest);
	MD5Update (&theDigest, (unsigned char *)fieldsWithDelimeters.data(), (UInt16) fieldsWithDelimeters.length());
	MD5Final(&theDigest);
	return string(reinterpret_cast<char*>(theDigest.digest), sizeof(theDigest.digest));
}

string
CDataSource::DigestFields(const vector<string>& fields)
{
	return DigestFieldsSafe(fields);
}

const int kDigestLength = 16;

bool
CDataSource::DigestFieldsMatch(const vector<string>& fields, const string& thisDigest)
{
	if (thisDigest.length() != kDigestLength)
		return false;
	return DigestFieldsSafe(fields) == thisDigest;
}

#pragma mark ===== CDatabase ===

CDatabase::CDatabase()
	: fFieldNamesIncluded(false)
{
}

CDatabase::~CDatabase()
{
}

void
CDatabase::ForgetData()
{
	fFields.clear();
}


int
CDatabase::FindFieldIndex(const string& fieldName)
{
	vector<string> fieldNames;
	GetFieldNames(fieldNames);
	vector<string>::iterator i = find(fieldNames.begin(), fieldNames.end(), fieldName);
	if (i == fieldNames.end())
		return -1;
	
	return i - fieldNames.begin();
}

int
CDatabase::IndexFromRecordAndField(int recordNumber, int fieldNumber)
{
	return recordNumber * FieldCount() + fieldNumber;
}

int
CDatabase::FieldCount()
{
	return fFieldNames.size();
}

string
CDatabase::GetFieldName(int fieldNumber)
{
	if (fFieldNamesIncluded)
		return fFields[fieldNumber];
	return fFieldNames[fieldNumber];
}

void
CDatabase::GetFieldNames(vector<string>& fieldNames)
{
	if (fFieldNamesIncluded)
		GetRecord(-1, fieldNames);
	else
		fieldNames = fFieldNames;
}

void
CDatabase::GetFieldTypes(vector<int>& theTypes)
{
	theTypes = fFieldTypes;
}

void
CDatabase::GetFieldChoices(vector<string_vector>&theChoices)
{
	theChoices = fPopupChoices;
}

void
CDatabase::AppendFieldName(const string& fieldName)
{
	fFieldNames.push_back(fieldName);
}

void
CDatabase::GetDefaultFieldNames(vector<string>& fieldNames)
{
	int count = FieldCount();
	for (long i = 1; i < count; i += 1) {
		string fieldName("Field ");
		fieldName += AsString(i);
		fieldNames.push_back(fieldName);
	}
}

void
CDatabase::SetFieldNamesIncluded(Boolean included)
{
	fFieldNamesIncluded = included;
}

void
CDatabase::AppendFieldType(int fieldType)
{
	vector<string> noChoices;
	AppendFieldType(fieldType, noChoices);
}

void
CDatabase::AppendFieldType(int fieldType, const vector<string>& inPopupChoices)
{
	fFieldTypes.push_back(fieldType);
	fPopupChoices.push_back(inPopupChoices);
}

int
CDatabase::RecordCount()
{
	int recordCount = fFields.size() / FieldCount();
	if (recordCount > 0 && fFieldNamesIncluded)
		recordCount -= 1;
	
	return recordCount;
}

void
CDatabase::AppendField(long dataValue)
{
	AppendField(AsString(dataValue));
}

void
CDatabase::AppendField(const string& dataValue)
{
	fFields.push_back(dataValue);
}

void
CDatabase::AppendFields(const vector<string>& fields)
{
	vector<string>::const_iterator i = fields.begin();
	while (i != fields.end()) {
		fFields.push_back(*i);
		++i;
	}
}

void
CDatabase::SetFieldNames(const vector<string>& fieldNames)
{
	fFieldNames = fieldNames;
}

void
CDatabase::GetRecord(int recordNumber, vector<string>& theResult)
{
	if (fFieldNamesIncluded)
		recordNumber += 1;
	int startIndex = IndexFromRecordAndField(recordNumber, 0);
	int endIndex = startIndex + FieldCount();
	for (int i = startIndex; i < endIndex; i += 1)
		theResult.push_back(fFields.at(i));
}

void
CDatabase::GetField(int recordNumber, int fieldNumber, string& theResult)
{
	if (fFieldNamesIncluded)
		recordNumber += 1;
	int startIndex = IndexFromRecordAndField(recordNumber, fieldNumber);
	theResult = fFields.at(startIndex);
}

void
CDatabase::DeleteRecord(int recordNumber)
{
	if (fFieldNamesIncluded)
		recordNumber += 1;
	vector<string>::iterator i = fFields.begin();
	ThrowIf_(i == fFields.end());
	i += IndexFromRecordAndField(recordNumber, 0);
	vector<string>::iterator j = i + FieldCount();
	ThrowIf_(j == fFields.end());
	fFields.erase(i, j);
	
}

void
CDatabase::SetFields(int recordNumber, const vector<string>& fields)
{
	if (fFieldNamesIncluded)
		recordNumber += 1;
	string toDigest;
	vector<string>::const_iterator k = fields.begin();
	vector<string>::iterator i = fFields.begin();
	i += IndexFromRecordAndField(recordNumber, 0);
	vector<string>::iterator j = i + FieldCount();
	while (i != j) {
		*i = *k;
		++i;
		++k;
	}
}

CDatabaseSlice::CDatabaseSlice(const vector<int> &sliceVector, CDatabase* sourceData)
	: fSlice(sliceVector), fSource(sourceData)
{
}

CDatabaseSlice::~CDatabaseSlice()
{
}

int
CDatabaseSlice::RecordCount()
{
	return fSource->RecordCount();
}

int
CDatabaseSlice::FieldCount()
{
	return fSlice.size();
}

void
CDatabaseSlice::GetFieldNames(vector<string>& theResult)
{
	vector<string> fieldNames;
	fSource->GetFieldNames(fieldNames);
	int count = fSlice.size();
	for (int i = 0; i < count; i += 1) {
		int index = fSlice[i];
		theResult.push_back(fieldNames[index]);
	}
}

void
CDatabaseSlice::GetRecord(int recordNumber, vector<string>& theResult)
{
	vector<string> record;
	fSource->GetRecord(recordNumber, record);
	int count = fSlice.size();
	for (int i = 0; i < count; i += 1) {
		int index = fSlice[i];
		theResult.push_back(record[index]);
	}
}

void
CDatabaseSlice::GetFieldTypes(vector<int>&)
{
}
