#include "DebugOutput.h"
#include "CSyncRecord.h"

CSyncRecord::CSyncRecord()
{
}

Boolean CSyncRecord::FieldIDExists(FieldID inFieldID) const
{
	return fFields.find(inFieldID) != fFields.end();
}

void CSyncRecord::SetValue(FieldID inFieldID, DataType inDataType, const string& v)
{
	fFields[inFieldID] = make_pair(v, inDataType);
}

void CSyncRecord::SetValue(FieldID inFieldID, const string& v)
{
	SetValue(inFieldID, kStringData, v);
}

void CSyncRecord::SetValue(FieldID inFieldID, DataType inDataType, const void* inDataPtr, short inDataSize)
{
	SetValue(inFieldID, inDataType, string(reinterpret_cast<const char*>(inDataPtr), inDataSize));
}


void CSyncRecord::RemoveValue(FieldID inFieldID)
{
	FieldMap::iterator f = fFields.find(inFieldID);
	Assert_(f != fFields.end());
	if (f != fFields.end()) {
		fFields.erase(f);
	}
}


Boolean CSyncRecord::GetValue(FieldID inFieldID, short* outShortValue) const
{
	Boolean valueExists = false;
	DataType dataType = kWildcardData;
	short dataLen = sizeof(short);
	short actualDataLen = 0;
	
	valueExists = this->GetValue(inFieldID, &dataType, outShortValue, dataLen, &actualDataLen);
	Assert_(dataType == kInteger16Data);
	Assert_(actualDataLen == sizeof(short));
	
	return valueExists;
}

Boolean CSyncRecord::GetValue(FieldID inFieldID, long* outLongValue) const
{
	Boolean valueExists = false;
	DataType dataType = kWildcardData;
	short dataLen = sizeof(long);
	short actualDataLen = 0;
	
	valueExists = this->GetValue(inFieldID, &dataType, outLongValue, dataLen, &actualDataLen);
	Assert_(dataType == kInteger32Data);
	Assert_(actualDataLen == sizeof(long));
	
	return valueExists;
}

const void* CSyncRecord::GetValuePtr(FieldID inFieldID, DataType* outDataType, short* outDataSize) const
{
	const void *outDataPtr = 0;
	bool valueExists = false;
	FieldMap::const_iterator f = fFields.find(inFieldID);
	valueExists = (f != fFields.end());
	
	if (valueExists) {
		Field theField = f->second;
		*outDataType = f->second.second;
		*outDataSize = f->second.first.length();
		outDataPtr = f->second.first.data();
	}
	
	return outDataPtr;
}


Boolean CSyncRecord::RecordDataEqualTo(const CSyncRecord& inCompareRec) const
{
	IgnoreMap ignoreThese;
	return RecordDataEqualTo(inCompareRec, ignoreThese);
}

Boolean CSyncRecord::RecordDataEqualTo(const CSyncRecord& inCompareRec, const IgnoreMap& ignoreThese) const
{
	FieldMap::const_iterator i = fFields.begin();
	for (;i != fFields.end();++i) {
		//skip over irrelevant header fields
		if (i->first == kRecordIDField ||
			i->first == kAttributesField ||
			i->first == kLocalIDField)
			continue;
		#if 0
		if (ignoreThese.find(i->first) != ignoreThese.end())
			continue;
		#endif
		FieldMap::const_iterator f = inCompareRec.fFields.find(i->first);
		if (f == inCompareRec.fFields.end() || i->second != f->second)
			return false;
	}
	
	return true;
}

string CSyncRecord::GetValue(FieldID inFieldID) const
{
	FieldMap::const_iterator f = fFields.find(inFieldID);
	if (f == fFields.end())
		return "";
	return f->second.first;
}


Boolean CSyncRecord::GetValue(FieldID inFieldID, DataType* outDataType, void* outDataPtr, 
								short maximumSize, short* actualSize) const
{
	Boolean valueExists = false;
	const void* actualDataPtr = nil;
	short returnedDataLen = 0;

	actualDataPtr = this->GetValuePtr(inFieldID, outDataType, actualSize);
	Assert_(actualDataPtr != nil);
	Assert_(*actualSize <= maximumSize);
	
	valueExists = (actualDataPtr != nil);
	
	if (actualDataPtr != nil)
	{
		//only move the actual number of bytes if they passed us a large buffer
		if (*actualSize > maximumSize)
		{
			returnedDataLen = maximumSize;
		}
		else
		{
			returnedDataLen = *actualSize;
		}
	
		::BlockMoveData(actualDataPtr, outDataPtr, returnedDataLen);
		
		//fix up known data types - should this really be here??
		if (*actualSize > maximumSize)
		{
			if (*outDataType == kStringData)
				((char*)outDataPtr)[maximumSize-1] = 0;
		}
	}

	return valueExists;
	
}


UInt32 CSyncRecord::GetRemoteRecordID() const
{
	UInt32 recordID = 0;
	DataType dataType = kWildcardData;
	short actualDataLen = 0;
	
	Boolean valueExists = this->GetValue(kRecordIDField, &dataType, &recordID, sizeof(recordID), &actualDataLen);
	Assert_(valueExists);
	Assert_(dataType == kInteger32Data);
	Assert_(actualDataLen == sizeof(recordID));
	
	return recordID;
}

UInt32 CSyncRecord::GetLocalRecordID() const
{
	UInt32 recordID = 0;
	DataType dataType = kWildcardData;
	short actualDataLen = 0;
	
	Boolean valueExists = this->GetValue(kLocalIDField, &dataType, &recordID, sizeof(recordID), &actualDataLen);
	Assert_(valueExists);
	Assert_(dataType == kInteger32Data);
	Assert_(actualDataLen == sizeof(recordID));
	
	return recordID;
}

UInt8 CSyncRecord::GetCategoryID() const
{
	UInt8 categoryID = 0;
	DataType dataType = kWildcardData;
	short actualDataLen = 0;
	
	Boolean valueExists = this->GetValue(kCategoryIDField, &dataType, &categoryID, sizeof(categoryID), &actualDataLen);
	Assert_(valueExists);
	Assert_(dataType == kInteger8Data);
	Assert_(actualDataLen == sizeof(categoryID));
	
	return categoryID;
}


UInt32 CSyncRecord::GetAttributes() const
{
	UInt32 attributes = 0;
	DataType dataType = kWildcardData;
	short actualDataLen = 0;
	
	Boolean valueExists = this->GetValue(kAttributesField, &dataType, &attributes, sizeof(attributes), &actualDataLen);
	Assert_(dataType == kInteger32Data);
	Assert_(actualDataLen == sizeof(attributes));
	
	return attributes;
}


void CSyncRecord::SetRemoteRecordID(const UInt32 inRecordID)
{
	this->SetValue(kRecordIDField, kInteger32Data, &inRecordID, sizeof(inRecordID));
}

void CSyncRecord::SetLocalRecordID(const UInt32 inRecordID)
{
	this->SetValue(kLocalIDField, kInteger32Data, &inRecordID, sizeof(inRecordID));
}

void CSyncRecord::SetCategoryID( const UInt8 inCategoryID )
{
	this->SetValue(kCategoryIDField, kInteger8Data, &inCategoryID, sizeof( inCategoryID ) );
}



void CSyncRecord::SetAttributes(UInt32 inAttributes)
{
	this->SetValue(kAttributesField, kInteger32Data, &inAttributes, sizeof(inAttributes));
}



void CSyncRecord::GetDifferences(const CSyncRecord& inCompareRec, CFieldIDArray* outFieldIDArray) const
{
	
}



Boolean CSyncRecord::CompareFields(FieldID inFieldID, const CSyncRecord& inCompareRec) const
{
	FieldMap::const_iterator f1 = fFields.find(inFieldID);
	FieldMap::const_iterator f2 = inCompareRec.fFields.find(inFieldID);
	
	if (f1 == fFields.end()) {
		//both non-existant we treat as equal
		return f2 == inCompareRec.fFields.end();
	}
	return *f1 == *f2;
}


void CSyncRecord::SetPrivate(Boolean inPrivate)
{
	UInt32 attr = this->GetAttributes();
	
	if (inPrivate)
		attr |= kPrivateBit;
	else
		attr &= ~kPrivateBit;
		
	this->SetAttributes(attr);
}


Boolean CSyncRecord::RequiredFieldsExist() const
{
	return ( 	this->FieldIDExists(kRecordIDField) &&
				this->FieldIDExists(kCategoryIDField) &&
				this->FieldIDExists(kAttributesField) );
}

void CSyncRecord::SetModified(Boolean inModified)
{
	UInt32 attr = this->GetAttributes();
	
	if (inModified)
		attr |= kModifiedBit;
	else
		attr &= ~kModifiedBit;
		
	this->SetAttributes(attr);
}


void CSyncRecord::SetDeleted(Boolean inDeleted)
{
	UInt32 attr = this->GetAttributes();
	
	if (inDeleted)
		attr |= kDeletedBit;
	else
		attr &= ~kDeletedBit;
		
	this->SetAttributes(attr);
}



void CSyncRecord::SetArchived(Boolean inArchived)
{
	UInt32 attr = this->GetAttributes();
	
	if (inArchived)
		attr |= kArchivedBit;
	else
		attr &= ~kArchivedBit;
		
	this->SetAttributes(attr);
}

void CSyncRecord::RemoveFields()
{
	fFields.clear();
}

#ifdef _DEBUG

void CSyncRecord::OutputForDebug() const
{
	DebugOutput::Output("fFields", fFields);
}

namespace DebugOutput {

void DoOutput(const CSyncRecord& item)
{
	item.OutputForDebug();
}

}
#endif