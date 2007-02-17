#include "CDataTask.h"
#include "Stringiness.h"
#include "DebugOutput.h"
#include "Utilities.h"
#include "WriteJFile5.h"

#pragma mark ======== CDataHolder ========

using JFile5::SplitRepeatingFields;
using JFile5::JoinRepeatingFields;

CDataHolder::CDataHolder(int dataState, int recordID, const CFieldIDList& fieldIDs, const vector<int>& repeats, const vector<string>& fieldData)
	: fDataState(dataState), fRecordID(recordID)
{
	SetDataMap(fieldIDs, repeats, fieldData);
}

CDataHolder::CDataHolder()
	: fDataState(kUnchanged), fRecordID(0)
{
}

CDataHolder::~CDataHolder()
{
}

vector<string>
CDataHolder::MakeDataVector() const 
{
	vector<string> fields;
	DataMap::const_iterator i = fDataMap.begin();
	while (i != fDataMap.end()) {
		fields.push_back(i->second);
		++i;
	}
	return fields;
}

vector<bool>
CDataHolder::MakeReadOnlyVector() const 
{
	vector<bool> ro;
	FieldBoolMap::const_iterator i = fReadOnly.begin();
	while (i != fReadOnly.end()) {
		ro.push_back(i->second);
		++i;
	}
	return ro;
}

vector<string>
CDataHolder::MakeOrderedDataVector(const CFieldIDList& fieldIDs) const 
{
	vector<string> fields;
	CFieldIDList::const_iterator i = fieldIDs.begin();
	while (i != fieldIDs.end()) {
		DataMap::const_iterator f = fDataMap.find(*i);
		if (f != fDataMap.end())
			fields.push_back(f->second);
		else
			fields.push_back(kEmptyString);
		++i;
	}
	return fields;
}

CFieldIDList
CDataHolder::MakeFieldIDVector() const
{
	CFieldIDList ids;
	DataMap::const_iterator i = fDataMap.begin();
	while (i != fDataMap.end()) {
		ids.push_back(i->first);
		++i;
	}
	return ids;
}

DataMap
CDataHolder::MakeDataMap() const
{
	return fDataMap;
}

int
CDataHolder::FieldCount() const
{
	return fDataMap.size();
}

bool
CDataHolder::HasField(FMAE::FieldID fieldID) const
{
	return fDataMap.find(fieldID) != fDataMap.end();
}

void CDataHolder::AddToMap(FMAE::FieldID fieldID, const char *p, int dataSize)
{
	AddToMap(fieldID, string(p, dataSize));
}

void CDataHolder::AddToMap(FMAE::FieldID fieldID, const string& v)
{
	fDataMap[fieldID] = v;
}

void
CDataHolder::AddToDataMap(const CFieldIDList& fieldIDs, const vector<string>& fieldData)
{
	::AddToMap(fieldIDs, fieldData, fDataMap);
}

void
CDataHolder::SetDataMap(const CFieldIDList& fieldIDs, const vector<int>& repeats, const vector<string>& fieldData)
{
	fDataMap = MakeMap(fieldIDs, fieldData);
	fRepeatMap = MakeMap(fieldIDs,repeats);
}

void
CDataHolder::SetReadOnly(const FieldBoolMap& ro)
{
	fReadOnly = ro;
}

void
CDataHolder::SetMergedRepeatingField(const FieldIDAndRepeat& fieldID, const string& mergedField)
{
	string originalField = fDataMap[fieldID.GetFieldID()];
	vector<string> splitFields = SplitRepeatingFields(originalField, fRepeatMap[fieldID.GetFieldID()]);
	splitFields.at(fieldID.GetRepeatIndex()) = mergedField;
	fDataMap[fieldID.GetFieldID()] = JoinRepeatingFields(splitFields, fRepeatMap[fieldID.GetFieldID()]);
}


void
CDataHolder::DoOutput() const
{
	DebugOutput::Output("fDataMap: ", fDataMap);
	DebugOutput::Output("fReadOnly: ", fReadOnly);
	DebugOutput::Output("fRecordID: ", fRecordID);
	DebugOutput::Output("fDataState: ", fDataState);
}



#pragma mark ======== CDataTask ========


CDataTask::CDataTask(int action)
	: fAction(action), fOverrideAction(kNotSet), fIsDeleteConflict(false), 
		fDeleteTakeLocal(false), fSelectorsInitialized(false), fDuplicate(false)
{
}

CDataTask::CDataTask()
	: fAction(kNotSet), fOverrideAction(kNotSet), fIsDeleteConflict(false), 
	fDeleteTakeLocal(false), fSelectorsInitialized(false), fDuplicate(false)
{
}

CDataTask::~CDataTask()
{
}

void
CDataTask::InitializeFieldSelector(bool localWins)
{
	if (fLocal.fDataState == CDataHolder::kDeleted) {
		fIsDeleteConflict = true;
		fDeleteTakeLocal = localWins;
		fMerged = fRemote;
	} else if (fRemote.fDataState == CDataHolder::kDeleted) {
		fIsDeleteConflict = true;
		fDeleteTakeLocal = localWins;
		fMerged = fLocal;
	} else {
		fIsDeleteConflict = false;
		if (localWins)
			fMerged = fLocal;
		else
			fMerged = fRemote;
	}
	fDefaultTakeLocal = localWins;
	fSelectorsInitialized = true;
}

string CDataTask::MergedField(const FieldIDAndRepeat& fieldID)
{
	string v = fMerged.MakeDataMap()[fieldID.GetFieldID()];
	return v;
}

void
CDataTask::SetMergedField(const FieldIDAndRepeat& fieldID, const string& mergedField)
{
	if (fMerged.GetRepeatMap()[fieldID.GetFieldID()] > 1) {
		fMerged.SetMergedRepeatingField(fieldID, mergedField);
	} else {
		fMerged.AddToMap(fieldID.GetFieldID(), mergedField);
	}
}

bool
CDataTask::IsConflict() const
{
	return fAction == kConflict || fAction == kConflictDeleteRemote;
}

bool
CDataTask::RemoteMatchesLocal()
{
	DataMap::const_iterator i = fRemote.fDataMap.begin();
	while (i != fRemote.fDataMap.end()) {
		if (fLocal.fDataMap[i->first] != i->second)
			return false;
		++i;
	}
	return true;
}


void
CDataTask::SetLocalData(const CDataHolder& holder)
{
	fLocal = holder;
}

void
CDataTask::GetLocalData(CDataHolder& holder) const
{
	holder = fLocal;
}

void
CDataTask::GetMergedData(CDataHolder& holder) const
{
	holder = fMerged;
}

void
CDataTask::SetRemoteData(const CDataHolder& holder)
{
	fRemote = holder;
}

void
CDataTask::GetRemoteData(CDataHolder& holder) const
{
	holder = fRemote;
}

int
CDataTask::GetRemoteRecordID() const
{
	return fRemote.fRecordID;
}

int
CDataTask::GetLocalRecordID() const
{
	return fLocal.fRecordID;
}

int
CDataTask::GetRemoteState() const
{
	return fRemote.fDataState;
}

int
CDataTask::GetLocalState() const
{
	return fLocal.fDataState;
}

int
CDataTask::GetAction() const
{
	if (fOverrideAction != kNotSet)
		return fOverrideAction;
	return fAction;
}

int
CDataTask::GetOriginalAction() const
{
	return fAction;
}

string
CDataTask::GetActionString() const
{
	switch(GetAction()) {
	case kNotSet:
		return "was not set";
	case kDeleteLocal:
		return "delete local";
	case kDeleteRemote:
		return "delete remote";
	case kLocalReplaceRemote:
		return "local replace remote";
	case kRemoteReplaceLocal:
		return "remote replace local";
	case kLocalAddToRemote:
		return "local add to remote";
	case kRemoteAddToLocal:
		return "remote add to local";
	case kDoNothing:
		return "do nothing";
	case kConflict:
		return "resolve conflict";
	};
	string actionString("unknown action ");
	actionString += AsString(GetAction());
	return actionString;
}

void
CDataTask::OverrideAction(bool duplicate)
{
	if (!fSelectorsInitialized) {
		InitializeFieldSelector(true);
	}
	bool doDuplicate = fDuplicate || duplicate;
	if (fIsDeleteConflict)
		HandleDeleteConflict(doDuplicate);
	else
		HandleDataConflict(doDuplicate);
}

void
CDataTask::HandleDataConflict(bool duplicate)
{
	if (duplicate) {
		if (fLocal.fDataState == CDataHolder::kDeleted)
			fOverrideAction = kDuplicateDeleteRemote;
		else
			fOverrideAction = kDuplicate;
	} else {
		if (fLocal.fDataState == CDataHolder::kDeleted)
			fOverrideAction = kMergeDeleteRemote;
		else
			fOverrideAction = kMerge;
	}
}

void
CDataTask::HandleDeleteConflict(bool duplicate)
{
	if (duplicate) {
		if (fLocal.fDataState == CDataHolder::kDeleted || fLocal.fDataState == CDataHolder::kDoesNotExist)
			fOverrideAction = kRemoteAddToLocal;
		else if (fRemote.fDataState == CDataHolder::kDeleted || fRemote.fDataState == CDataHolder::kDoesNotExist)
			fOverrideAction = kLocalAddToRemote;
	} else {
		if (fDeleteTakeLocal) {
			if (fLocal.fDataState == CDataHolder::kDeleted || fLocal.fDataState == CDataHolder::kDoesNotExist)
				fOverrideAction = kDeleteRemote;
			else
				fOverrideAction = kLocalAddToRemote;
		} else {
			if (fRemote.fDataState == CDataHolder::kDeleted || fRemote.fDataState == CDataHolder::kDoesNotExist)
				fOverrideAction = kDeleteLocal;
			else
				fOverrideAction = kRemoteAddToLocal;
		}
	}
}

bool
CDataTask::GetDuplicate() const
{
	return fDuplicate;
}


void
CDataTask::SetDuplicate(bool duplicate)
{
	fDuplicate = duplicate;
}
void
CDataTask::DoOutput() const
{
	DebugOutput::Output("fLocal: ", fLocal);
	DebugOutput::Output("fRemote: ", fRemote);
	DebugOutput::Output("fMerged: ", fMerged);
	DebugOutput::Output("fAction: ", fAction);
	DebugOutput::Output("fOverrideAction: ", fOverrideAction);
	DebugOutput::Output("action string: ", GetActionString());
	DebugOutput::Output("fIsDeleteConflict: ", fIsDeleteConflict);
	DebugOutput::Output("fDeleteTakeLocal: ", fDeleteTakeLocal);
	DebugOutput::Output("fDefaultTakeLocal: ", fDefaultTakeLocal);
	DebugOutput::Output("fDuplicate: ", fDuplicate);
	DebugOutput::Output("fSelectorsInitialized: ", fSelectorsInitialized);
}

namespace DebugOutput {

void DoOutput(const CDataTask& task)
{
	task.DoOutput();
}

void DoOutput(const CDataHolder& holder)
{
	holder.DoOutput();
}

}