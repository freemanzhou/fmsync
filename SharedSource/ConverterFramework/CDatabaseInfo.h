#pragma once

#include <string>
#include <vector>
#include <map>

#include "CDatabase.h"
#include "FMAE.h"
#include "CFieldIDList.h"
#include "CFieldOverrides.h"
#ifdef CONDUIT
#include "BinaryFormat.h"
#endif

const int kFileMakerWriteAccessMask = 2;

class CDatabaseInfo {
public:
	CDatabaseInfo();

	vector<string> GetFieldNames(const CFieldIDList& fieldIDs);
	map<FMAE::FieldID,string> GetFieldNames();
	bool NamesMatch(const CDatabaseInfo& matchThis);

	bool FieldType(FMAE::FieldID fieldID, int& fieldType) const;
	vector<int> GetFieldTypes(const CFieldIDList& fieldIDs) const;
	map<FMAE::FieldID,int> GetFieldTypes() const;

	bool FieldAccess(FMAE::FieldID fieldID, int& fieldType) const;
	vector<int> GetFieldAccesses(const CFieldIDList& fieldIDs) const;
	map<FMAE::FieldID,int> GetFieldAccesses() const;

	bool ColumnWidth(FMAE::FieldID fieldID, int& columnWidth) const;
	vector<int> GetColumnWidths(const CFieldIDList& fieldIDs) const;
	map<FMAE::FieldID,int> GetColumnWidths() const;

	bool FieldName(FMAE::FieldID fieldID, string& fieldName) const;

	bool PopupValues(FMAE::FieldID fieldID, string_vector& popupChoices) const;

	CFieldOverrides CompareWith(const CDatabaseInfo&newInfo) const;
	void OverrideWith(const CFieldOverrides&);
	
	map<FMAE::FieldID,int> GetFieldExtraData() const;
	bool FieldExtraData(FMAE::FieldID fieldID, int& extra) const;
	void SetFieldExtraData(const map<FMAE::FieldID,int>&);

	map<FMAE::FieldID,int> GetFieldExtraData2() const;
	bool FieldExtraData2(FMAE::FieldID fieldID, int& extra) const;
	void SetFieldExtraData2(const map<FMAE::FieldID,int>&);

	static void  Write(LStream&, const CDatabaseInfo&);
	static void  Read(LStream&, CDatabaseInfo&);
	static void  Read115(LStream&, CDatabaseInfo&);

	void DoOutput() const;

	map<FMAE::FieldID,string> fFieldNames;
	map<FMAE::FieldID,string_vector> fPopupValues;
	map<FMAE::FieldID,int> fFieldTypes;
	map<FMAE::FieldID,int> fFieldAccess;
	map<FMAE::FieldID,int> fRepeats;
	map<FMAE::FieldID,int> fColumnWidths;
	map<FMAE::FieldID,int> fFieldExtraData;
	map<FMAE::FieldID,int> fFieldExtraData2;
	map<FMAE::FieldID,string> fFieldCalcValue1;
	map<FMAE::FieldID,string> fFieldCalcValue2;
	vector<string_vector> fJFileExtraValues;
	int fFieldCount;
	int fShowDataWidth;
	int fSortFields[3];
	int fFindField;
	int fFilterField;
	string fFindString;
	string fFilterString;
	int fFlags;
	int fVersion5;
	int fFirstColumnToShow;
	string fPassword;
	bool fBackup;
};

inline
Boolean operator==(
				const CDatabaseInfo&	inLhs,
				const CDatabaseInfo&	inRhs)
		{
			return
				(inLhs.fFieldNames == inRhs.fFieldNames) && 
				(inLhs.fFieldTypes == inRhs.fFieldTypes) && 
				(inLhs.fFieldAccess == inRhs.fFieldAccess) && 
				(inLhs.fColumnWidths == inRhs.fColumnWidths) && 
				(inLhs.fFieldExtraData == inRhs.fFieldExtraData) && 
				(inLhs.fFieldExtraData2 == inRhs.fFieldExtraData2) && 
				(inLhs.fFieldCalcValue1 == inRhs.fFieldCalcValue1) && 
				(inLhs.fFieldCalcValue2 == inRhs.fFieldCalcValue2) && 
				(inLhs.fPopupValues == inRhs.fPopupValues) && 
				(inLhs.fJFileExtraValues == inRhs.fJFileExtraValues) && 
				(inLhs.fFieldCount == inRhs.fFieldCount) && 
				(inLhs.fShowDataWidth == inRhs.fShowDataWidth) && 
				(inLhs.fSortFields[0] == inRhs.fSortFields[0]) && 
				(inLhs.fSortFields[1] == inRhs.fSortFields[1]) && 
				(inLhs.fSortFields[2] == inRhs.fSortFields[2]) && 
				(inLhs.fFindField == inRhs.fFindField) && 
				(inLhs.fFilterField == inRhs.fFilterField) && 
				(inLhs.fFindString == inRhs.fFindString) && 
				(inLhs.fFilterString == inRhs.fFilterString) && 
				(inLhs.fFlags == inRhs.fFlags) && 
				(inLhs.fVersion5 == inRhs.fVersion5) && 
				(inLhs.fFirstColumnToShow == inRhs.fFirstColumnToShow) && 
				(inLhs.fPassword == inRhs.fPassword) && 
				(inLhs.fBackup == inRhs.fBackup);
		}


#ifdef CONDUIT
namespace BinaryFormat {

template <>
inline void Write(LStream& s, const CDatabaseInfo& info)
{
	CDatabaseInfo::Write(s, info);
}

template <>
inline void Read(LStream& s, CDatabaseInfo& info)
{
	CDatabaseInfo::Read(s, info);
}

template <>
inline void Write(LStream& s, const CFieldOverrides& info)
{
	CFieldOverrides::Write(s, info);
}

template <>
inline void Read(LStream& s, CFieldOverrides& info)
{
	CFieldOverrides::Read(s, info);
}

}
#endif

#ifdef _DEBUG
namespace DebugOutput {

void DoOutput(const CDatabaseInfo& item);

}
#endif