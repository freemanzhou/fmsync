#pragma once

#include "CFieldIDList.h"
#include "FieldIDAndRepeat.h"

typedef map<FMAE::FieldID,bool> FieldBoolMap;
typedef map<FMAE::FieldID,int> RepeatMap;
typedef map<FMAE::FieldID,string> DataMap;

class CDataHolder {
public:
	enum {kUnchanged, kChanged, kDeleted, kAdded, kDoesNotExist};

	CDataHolder();
	CDataHolder(int dataState, int recordID, const CFieldIDList& fieldIDs, const vector<int>& repeats, const vector<string>& fieldData);
	~CDataHolder();
	
	DataMap				MakeDataMap() const;
	RepeatMap			GetRepeatMap() const {return fRepeatMap;}
	void				SetDataMap(const CFieldIDList& fieldIDs, const vector<int>& repeats, const vector<string>& fieldData);
	void				SetReadOnly(const FieldBoolMap& ro);
	void				AddToDataMap(const CFieldIDList&, const vector<string>&);
	void				AddToMap(FMAE::FieldID fieldID, const string&);
	vector<string>		MakeDataVector() const;
	vector<bool>		MakeReadOnlyVector() const;
	vector<string>		MakeOrderedDataVector(const CFieldIDList& fieldIDs) const;
	CFieldIDList		MakeFieldIDVector() const;
	
	int					FieldCount() const;
	bool				HasField(FMAE::FieldID fieldID) const;
	void				SetMergedRepeatingField(const FieldIDAndRepeat& fieldID, const string& mergedField);
		
	void				DoOutput() const;

	DataMap 		fDataMap;
	RepeatMap		fRepeatMap;
	FieldBoolMap	fReadOnly;
	int				fRecordID;
	int				fDataState;

private:
	void				AddToMap(FMAE::FieldID fieldID, const char *p, int dataSize);
};

class CDataTask {
public:
	friend class CJFileSynchronizer;
	friend class CAddressSynchronizer;
	
	enum {kNotSet, kDoNothing, kLocalAddToRemote, kRemoteAddToLocal, kDeleteLocal, 
	kDeleteRemote, kLocalReplaceRemote, kRemoteReplaceLocal, kRemoteReplaceLocalDeleteRemote, kMergeDeleteRemote, 
	kConflict, kConflictDeleteRemote, kMerge, kDuplicate, kDuplicateDeleteRemote};
				
				CDataTask();
				CDataTask(int action);
				~CDataTask();
				
	void		SetLocalData(const CDataHolder& holder);
	void		GetLocalData(CDataHolder& holder) const;
	int			GetLocalRecordID() const;
	int			GetLocalState() const;

	void		SetRemoteData(const CDataHolder& holder);
	void		GetRemoteData(CDataHolder& holder) const;
	int			GetRemoteRecordID() const;
	int			GetRemoteState() const;
	
	void		GetMergedData(CDataHolder& holder) const;

	string		MergedField(const FieldIDAndRepeat& fieldID);
	void		SetMergedField(const FieldIDAndRepeat& fieldID, const string& mergedField);

	int			GetAction() const;
	int			GetOriginalAction() const;
	string		GetActionString() const;

	bool		GetDuplicate() const;
	void		SetDuplicate(bool duplicate);
	
	bool		IsConflict() const;
	bool		RemoteMatchesLocal();
	void		OverrideAction(bool duplicate);
	void		HandleDeleteConflict(bool duplicate);
	void		HandleDataConflict(bool duplicate);
	
	const CDataHolder& 		GetLocalDataRef();
	const CDataHolder& 		GetRemoteDataRef();
	
	void 		DoOutput() const;
	
private:
	void		InitializeFieldSelector(bool localWins);
	
	CDataHolder				fLocal;
	CDataHolder				fRemote;
	CDataHolder				fMerged;
	int						fAction;
	int						fOverrideAction;
	bool					fIsDeleteConflict;
	bool					fDeleteTakeLocal;
	bool					fDefaultTakeLocal;
	bool					fDuplicate;
	bool					fSelectorsInitialized;
};
namespace DebugOutput {

void DoOutput(const CDataTask&);
void DoOutput(const CDataHolder&);
}