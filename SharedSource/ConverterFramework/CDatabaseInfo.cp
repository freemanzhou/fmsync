#include "CDatabaseInfo.h"
#include "BinaryFormat.h"
#include "Utilities.h"
#include "DebugOutput.h"

CDatabaseInfo::CDatabaseInfo()
{
	fShowDataWidth = 110;
	fSortFields[0] = 0;
	fSortFields[1] = 0;
	fSortFields[2] = 0;
	fFindField = 0;
	fFilterField = 0;
	fFlags = 0;
	fFirstColumnToShow = 1;
	fBackup = false;
	fFieldCount = 0;
}

bool
CDatabaseInfo::NamesMatch(const CDatabaseInfo& matchThis)
{
	return false;
}

bool
CDatabaseInfo::FieldType(FMAE::FieldID fieldID, int& fieldType) const
{
	map<FMAE::FieldID, int>::const_iterator f = fFieldTypes.find(fieldID);
	if (f != fFieldTypes.end()) {
		fieldType = f->second;
		return true;
	}
	return false;
}

bool
CDatabaseInfo::FieldAccess(FMAE::FieldID fieldID, int& fieldAccess) const
{
	map<FMAE::FieldID, int>::const_iterator f = fFieldAccess.find(fieldID);
	if (f != fFieldTypes.end()) {
		fieldAccess = f->second;
		return true;
	}
	return false;
}

bool
CDatabaseInfo::ColumnWidth(FMAE::FieldID fieldID, int& columnWidth) const
{
	map<FMAE::FieldID, int>::const_iterator f = fColumnWidths.find(fieldID);
	if (f != fColumnWidths.end()) {
		columnWidth = f->second;
		return true;
	}
	return false;
}

bool
CDatabaseInfo::FieldExtraData(FMAE::FieldID fieldID, int& extra) const
{
	map<FMAE::FieldID, int>::const_iterator f = fFieldExtraData.find(fieldID);
	if (f != fFieldExtraData.end()) {
		extra = f->second;
		return true;
	}
	return false;
}

bool
CDatabaseInfo::FieldExtraData2(FMAE::FieldID fieldID, int& extra) const
{
	map<FMAE::FieldID, int>::const_iterator f = fFieldExtraData2.find(fieldID);
	if (f != fFieldExtraData2.end()) {
		extra = f->second;
		return true;
	}
	return false;
}

bool
CDatabaseInfo::FieldName(FMAE::FieldID fieldID, string& fieldName) const
{
	map<FMAE::FieldID, string>::const_iterator f = fFieldNames.find(fieldID);
	if (f != fFieldNames.end()) {
		fieldName = f->second;
		return true;
	}
	return false;
}

bool
CDatabaseInfo::PopupValues(FMAE::FieldID fieldID, string_vector& popupChoices) const
{
	map<FMAE::FieldID, string_vector>::const_iterator f = fPopupValues.find(fieldID);
	if (f != fPopupValues.end()) {
		popupChoices = f->second;
		return true;
	}
	return false;
}

vector<string>
CDatabaseInfo::GetFieldNames(const CFieldIDList& fieldIDs)
{
	vector<string> names;
	CFieldIDList::const_iterator i = fieldIDs.begin();
	while(i != fieldIDs.end()) {
		names.push_back(fFieldNames[*i]);
		++i;
	}
	return names;
}

map<FMAE::FieldID, string>
CDatabaseInfo::GetFieldNames()
{
	return fFieldNames;
}

vector<int>
CDatabaseInfo::GetFieldTypes(const CFieldIDList& fieldIDs) const
{
	return MakeOrderedList(fFieldTypes, fieldIDs);
}

map<FMAE::FieldID, int>
CDatabaseInfo::GetFieldTypes() const
{
	return fFieldTypes;
}

vector<int>
CDatabaseInfo::GetColumnWidths(const CFieldIDList& fieldIDs) const
{
	return MakeOrderedList(fColumnWidths, fieldIDs);
}

vector<int>
CDatabaseInfo::GetFieldAccesses(const CFieldIDList& fieldIDs) const
{
	return MakeOrderedList(fFieldAccess, fieldIDs);
}


void
CDatabaseInfo::OverrideWith(const CFieldOverrides& o)
{
	ApplyDiffMap(fFieldNames, o.fName);
	ApplyDiffMap(fFieldTypes, o.fType);
	ApplyDiffMap(fColumnWidths, o.fWidth);
	ApplyDiffMap(fFieldAccess, o.fReadOnly);
	ApplyDiffMap(fPopupValues, o.fPopups);
	ApplyDiffMap(fFieldExtraData, o.fExtra);
	ApplyDiffMap(fFieldExtraData2, o.fExtra2);
	ApplyDiffMap(fFieldCalcValue1, o.fFieldCalcValue1);
	ApplyDiffMap(fFieldCalcValue2, o.fFieldCalcValue2);
}

CFieldOverrides
CDatabaseInfo::CompareWith(const CDatabaseInfo&newInfo) const
{
	CFieldOverrides overrides;
	overrides.fName = MakeDiffMap(fFieldNames, newInfo.fFieldNames);
	overrides.fType = MakeDiffMap(fFieldTypes, newInfo.fFieldTypes);
	overrides.fReadOnly = MakeDiffMap(fFieldAccess, newInfo.fFieldAccess);
	overrides.fWidth = MakeDiffMap(fColumnWidths, newInfo.fColumnWidths);
	overrides.fPopups = MakeDiffMap(fPopupValues, newInfo.fPopupValues);
	overrides.fExtra = MakeDiffMap(fFieldExtraData, newInfo.fFieldExtraData);
	overrides.fExtra2 = MakeDiffMap(fFieldExtraData2, newInfo.fFieldExtraData2);
	overrides.fFieldCalcValue1 = MakeDiffMap(fFieldCalcValue1, newInfo.fFieldCalcValue1);
	overrides.fFieldCalcValue2 = MakeDiffMap(fFieldCalcValue2, newInfo.fFieldCalcValue2);
	return overrides;
}

map<FMAE::FieldID,int>
CDatabaseInfo::GetFieldExtraData() const
{
	return fFieldExtraData;
}

void
CDatabaseInfo::SetFieldExtraData(const map<FMAE::FieldID,int>& inExtra)
{
	fFieldExtraData = inExtra;
}

map<FMAE::FieldID,int>
CDatabaseInfo::GetFieldExtraData2() const
{
	return fFieldExtraData2;
}

void
CDatabaseInfo::SetFieldExtraData2(const map<FMAE::FieldID,int>& inExtra)
{
	fFieldExtraData2 = inExtra;
}

void
CDatabaseInfo::Read(LStream&s, CDatabaseInfo&info)
{
	BinaryFormat::Read(s, info.fFieldNames);
	BinaryFormat::Read(s, info.fPopupValues);
	BinaryFormat::Read(s, info.fFieldTypes);
	BinaryFormat::Read(s, info.fFieldAccess);
	BinaryFormat::Read(s, info.fColumnWidths);
	BinaryFormat::Read(s, info.fFieldExtraData);
	BinaryFormat::Read(s, info.fFieldExtraData2);
	BinaryFormat::Read(s, info.fFieldCalcValue1);
	BinaryFormat::Read(s, info.fFieldCalcValue2);
	BinaryFormat::Read(s, info.fShowDataWidth);
	BinaryFormat::Read(s, info.fSortFields[0]);
	BinaryFormat::Read(s, info.fSortFields[1]);
	BinaryFormat::Read(s, info.fSortFields[2]);
	BinaryFormat::Read(s, info.fFindField);
	BinaryFormat::Read(s, info.fFilterField);
	BinaryFormat::Read(s, info.fFindString);
	BinaryFormat::Read(s, info.fFilterString);
	BinaryFormat::Read(s, info.fFlags);
	BinaryFormat::Read(s, info.fFirstColumnToShow);
	BinaryFormat::Read(s, info.fPassword);
	BinaryFormat::Read(s, info.fBackup);
}

void
CDatabaseInfo::Read115(LStream&s, CDatabaseInfo&info)
{
	BinaryFormat::Read(s, info.fFieldNames);
	BinaryFormat::Read(s, info.fPopupValues);
	BinaryFormat::Read(s, info.fFieldTypes);
	BinaryFormat::Read(s, info.fFieldAccess);
	BinaryFormat::Read(s, info.fColumnWidths);
	BinaryFormat::Read(s, info.fFieldExtraData);
	BinaryFormat::Read(s, info.fFieldExtraData2);
	BinaryFormat::Read(s, info.fShowDataWidth);
	BinaryFormat::Read(s, info.fSortFields[0]);
	BinaryFormat::Read(s, info.fSortFields[1]);
	BinaryFormat::Read(s, info.fSortFields[2]);
	BinaryFormat::Read(s, info.fFindField);
	BinaryFormat::Read(s, info.fFilterField);
	BinaryFormat::Read(s, info.fFindString);
	BinaryFormat::Read(s, info.fFilterString);
	BinaryFormat::Read(s, info.fFlags);
	BinaryFormat::Read(s, info.fFirstColumnToShow);
	BinaryFormat::Read(s, info.fPassword);
	BinaryFormat::Read(s, info.fBackup);
}

void
CDatabaseInfo::Write(LStream&s, const CDatabaseInfo&info)
{
	BinaryFormat::Write(s, info.fFieldNames);
	BinaryFormat::Write(s, info.fPopupValues);
	BinaryFormat::Write(s, info.fFieldTypes);
	BinaryFormat::Write(s, info.fFieldAccess);
	BinaryFormat::Write(s, info.fColumnWidths);
	BinaryFormat::Write(s, info.fFieldExtraData);
	BinaryFormat::Write(s, info.fFieldExtraData2);
	BinaryFormat::Write(s, info.fFieldCalcValue1);
	BinaryFormat::Write(s, info.fFieldCalcValue2);
	BinaryFormat::Write(s, info.fShowDataWidth);
	BinaryFormat::Write(s, info.fSortFields[0]);
	BinaryFormat::Write(s, info.fSortFields[1]);
	BinaryFormat::Write(s, info.fSortFields[2]);
	BinaryFormat::Write(s, info.fFindField);
	BinaryFormat::Write(s, info.fFilterField);
	BinaryFormat::Write(s, info.fFindString);
	BinaryFormat::Write(s, info.fFilterString);
	BinaryFormat::Write(s, info.fFlags);
	BinaryFormat::Write(s, info.fFirstColumnToShow);
	BinaryFormat::Write(s, info.fPassword);
	BinaryFormat::Write(s, info.fBackup);
}

void
CDatabaseInfo::DoOutput() const
{
	DebugOutput::Output("fFieldNames: ", fFieldNames);
	DebugOutput::Output("fPopupValues: ", fPopupValues);
	DebugOutput::Output("fFieldTypes: ", fFieldTypes);
	DebugOutput::Output("fFieldAccess: ", fFieldAccess);
	DebugOutput::Output("fColumnWidths: ", fColumnWidths);
	DebugOutput::Output("fFieldCount: ", fFieldCount);
	DebugOutput::Output("fFieldExtraData: ", fFieldExtraData);
	DebugOutput::Output("fFieldExtraData2: ", fFieldExtraData2);
	DebugOutput::Output("fFieldCalcValue1: ", fFieldCalcValue1);
	DebugOutput::Output("fFieldCalcValue2: ", fFieldCalcValue2);
	DebugOutput::Output("fJFileExtraValues: ", fJFileExtraValues);
}

#ifdef _DEBUG
namespace DebugOutput {

void
DoOutput(const CDatabaseInfo& item)
{
	item.DoOutput();
}

}
#endif