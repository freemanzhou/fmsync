#pragma once

#include <PP_Prefix.h>
#include <vector>
#include <string>

typedef class vector<string> string_vector;

class CDataSource {
protected:
					CDataSource();

public:
		virtual		~CDataSource();
		
		int		RecordSize(int recordNumber);
		string	RecordDigest(int recordNumber);
		static string	DigestFields(const vector<string>& fields);
		static bool		DigestFieldsMatch(const vector<string>& fields, const string& thisDigest);
		Boolean	FieldNamesKnown();
		void	SetFieldNamesKnown(Boolean known);

		virtual int		RecordCount() = 0;
		virtual int		FieldCount() = 0;
		virtual void	GetFieldNames(vector<string>& theResult) = 0;
		virtual void	GetRecord(int recordNumber, vector<string>& theResult) = 0;
		virtual void	GetFieldTypes(vector<int>&) = 0;
		virtual void	GetFieldChoices(vector<string_vector>&) = 0;

private:
	Boolean fFieldNamesKnown;
};

class CDatabase : public CDataSource {
public:
			CDatabase();
			virtual ~CDatabase();
	
	void	ForgetData();
	
	virtual int		FieldCount();
	int		FindFieldIndex(const string& fieldName);
	string	GetFieldName(int fieldNumber);
	virtual void GetFieldNames(vector<string>&);
	void	AppendFieldName(const string& fieldName);
	void	AppendFieldType(int fieldType = 1);
	void	AppendFieldType(int fieldType, const vector<string>& popupChoices);
	virtual int		RecordCount();
	void	DeleteRecord(int recordNumber);
	void	GetField(int recordNumber, int fieldNumber, string& theResult);
	void	GetRecord(int recordNumber, vector<string>& theResult);
	virtual void	GetFieldTypes(vector<int>&);
	void	AppendField(long dataValue);
	void	AppendFields(const vector<string>& fields);
	void	AppendField(const string& dataValue);
	
	void	SetFieldNames(const vector<string>& fieldNames);
	void	SetFieldTypes(const vector<int>& fieldTypes);
	void	SetFields(int recordNumber, const vector<string>& fields);
	void	SetFieldNamesIncluded(Boolean included);
	void	GetDefaultFieldNames(vector<string>&);
	virtual void	GetFieldChoices(vector<string_vector>&);
	
private:
			CDatabase(const CDatabase& inDatabase);
	int		IndexFromRecordAndField(int recordNumber, int fieldNumber);
	
	vector<string>				fFieldNames;
	vector<string>				fFields;
	vector<int>					fFieldTypes;
	vector<string_vector>		fPopupChoices;
	Boolean						fFieldNamesIncluded;
};

class CDatabaseSlice : public CDataSource {
public:
					CDatabaseSlice(const vector<int> &sliceVector, CDatabase* sourceData);
		virtual 	~CDatabaseSlice();

		virtual int		RecordCount();
		virtual int		FieldCount();
		virtual void	GetFieldNames(vector<string>& theResult);
		virtual void	GetRecord(int recordNumber, vector<string>& theResult);
		virtual void	GetFieldTypes(vector<int>&);
		virtual void	GetFieldChoices(vector<string_vector>&);

private:
		vector<int>		fSlice;
		CDatabase*		fSource;
};