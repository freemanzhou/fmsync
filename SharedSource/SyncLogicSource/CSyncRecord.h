#pragma once

#include <map>

typedef UInt32 FieldID;

enum DataType
{
	kWildcardData		= '****',	
	kBooleanData		= 'bool',	
	kStringData			= 'char',
	kInteger64Data		= 'lnln',
	kInteger32Data		= 'long',
	kInteger16Data		= 'shor',
	kInteger8Data		= 'byte'
};

enum{
	kArchivedBit	= 0x00000001,
	kDeletedBit		= 0x00000002,
	kModifiedBit	= 0x00000004,
	kPrivateBit		= 0x00000008,
	kTempAttrsMask	= kDeletedBit|kModifiedBit
};

//Required fields
enum
{
	kRecordIDField 		= 'ID  ',
	kAttributesField 	= 'ATTR',
	kLocalIDField 		= 'LID ',
	kCategoryIDField	= 'CAT '
};

class CFieldIDArray;
class CRecordIterator;

typedef pair<string, DataType> Field;

class CSyncRecord
{
public:
	typedef map<FieldID,Field> FieldMap;
	typedef map<FieldID,bool> IgnoreMap;
/*
	NOTES:
*/

	CSyncRecord();
	
#ifdef _DEBUG
	void OutputForDebug() const;
#endif

	//Data manipulation

	void SetValue(FieldID inFieldID, DataType inDataType, const string& v);
	void SetValue(FieldID inFieldID, const string& v);
	
	inline void SetValue(FieldID inFieldID, short inShortValue) 
			{this->SetValue(inFieldID, kInteger16Data, &inShortValue, sizeof(inShortValue));}	
	//etc
		
	Boolean GetValue(FieldID inFieldID, DataType* outDataType, 
								void* outDataPtr, short maximumSize, short* actualSize) const;
	
	Boolean GetValue(FieldID inFieldID, short* outShortValue) const;
	Boolean GetValue(FieldID inFieldID, long* outLongValue) const;
	string GetValue(FieldID inFieldID) const;
	
	//etc
	
	
	//etc
	
	void RemoveValue(FieldID inFieldID);
	
	Boolean FieldIDExists(FieldID inFieldID) const;
	
//Comparison functionality
	Boolean RecordDataEqualTo(const CSyncRecord& inCompareRec) const;
	Boolean RecordDataEqualTo(const CSyncRecord& inCompareRec, const IgnoreMap& ignoreThese) const;
	
	void GetDifferences(const CSyncRecord& inCompareRec, CFieldIDArray* outFieldIDArray) const;
	
	Boolean CompareFields(FieldID inFieldID, const CSyncRecord& inCompareRec) const;

//Standard Record stuff 
	UInt32 GetRemoteRecordID() const;
	UInt32 GetLocalRecordID() const;
	void SetRemoteRecordID(const UInt32 inRecordID);
	void SetLocalRecordID(const UInt32 inRecordID);

//categories!
	UInt8 GetCategoryID() const;
	void SetCategoryID( const UInt8 inCategoryID );
	
	UInt32 GetAttributes() const;
	void SetAttributes(UInt32 inAttributes);
	
	void SetArchived(Boolean inArchived);
	void SetDeleted(Boolean inDeleted);
	void SetModified(Boolean inModified);
	void SetPrivate(Boolean inPrivate);
	
	Boolean IsArchived() const {return ((this->GetAttributes() & kArchivedBit) != 0);}
	Boolean IsDeleted() const {return ((this->GetAttributes() & kDeletedBit) != 0);}
	Boolean IsModified() const {return ((this->GetAttributes() & kModifiedBit) != 0);}
	Boolean IsPrivate() const {return ((this->GetAttributes() & kPrivateBit) != 0);}
	Boolean RequiredFieldsExist() const;
	void RemoveFields();

	
#ifdef _DEBUG
	
#endif	//_DEBUG

private:
	inline void SetValue(FieldID inFieldID, const char* inCStringValue) 
			{this->SetValue(inFieldID, kStringData, inCStringValue, strlen(inCStringValue)+1);}
	const void* GetValuePtr(FieldID inFieldID, DataType* outDataType, short* outDataSize) const;
	void SetValue(FieldID inFieldID, DataType inDataType, const void* inDataPtr, short inDataSize);

	FieldMap fFields;
};

#ifdef _DEBUG
namespace DebugOutput {

void DoOutput(const CSyncRecord& item);

}
#endif