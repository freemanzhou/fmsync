#pragma once

#include <LTable.h>
#include <LBroadcaster.h>
#include <vector>
#include <map>
#include <string>
#include "CDataTask.h"
#include "CDatabaseFile.h"
#include "CFieldIDList.h"
#include "FMDatabasePtr.h"

class CDatabaseFile;

class CTaskEdit : public LTable, public LBroadcaster
{
public:
	enum { class_ID = FOUR_CHAR_CODE('Task') };

		CTaskEdit(LStream *inStream);
		virtual ~CTaskEdit();

	void			SetDatabaseFile(CDatabaseFile::Ptr theDBFile);
	void			SetFMDatabase(FileMaker::DatabasePtr theFMdb);
	void			SetTask(const CDataTask& task, const CFieldIDList& fieldsToEdit, const vector<int>& repeats);
	void			GetTask(CDataTask& task);
	virtual void	DrawSelf();
	void			RefreshRow(const TableCellT &inCell);
	
protected:
	virtual void	DrawCell(const TableCellT &inCell);
	virtual void	ClickCell(const TableCellT &inCell,
									const SMouseDownEvent &inMouseDown);

private:
	enum 			{kFieldName, kLocalData, kRemoteData};
	void			DrawFieldName(int rowNumber, const Rect& localCellRect);
	void			DrawLocalFieldData(int rowNumber, const Rect& localCellRect);
	void			DrawRemoteFieldData(int rowNumber, const Rect& localCellRect);
	
	void			DrawRowString(const string& theData, const Rect& r);
	
	void			SetupTask();
	void			SetupFieldNames();
	void			SetupFieldData();
	void			SetupFieldIDsWithRepeats();
	void			SetupDataHolders();
	void			SetupFieldDiffers();
	void			SetupColumnIDs();
	void			SetupRowCount();
	
	vector<string>	SetupFieldData(const CDataHolder& holder);
	
	CDatabaseFile::Ptr		fDBFile;
	FileMaker::DatabasePtr	fFMDb;
	CDataTask		fTask;
	CDataHolder		fLocalData;
	CDataHolder		fRemoteData;
	string			fDeleted;
	vector<int>		fColumnID;
	vector<int>		fRepeats;
	CFieldIDList	fFieldIDs;
	vector<FieldIDAndRepeat> fFieldIDsWithRepeat;
	vector<bool>	fFieldDiffers;
	vector<string>	fFieldNames;
	vector<string>	fLocalFields;
	vector<string>	fRemoteFields;
	vector<string>	fMergedFields;
	int				fReadOnlyClick;
};

