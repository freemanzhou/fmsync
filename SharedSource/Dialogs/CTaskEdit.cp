#include "CTaskEdit.h"
#include "WriteJFile5.h"
#include "Str255.h"
#include "CDatabaseFile.h"
#include "OtherStrings.h"
#include "FMDatabase.h"
#include "Utilities.h"
#include <UGAColorRamp.h>
#include <UDrawingUtils.h>

#include <numeric>

using JFile5::SplitRepeatingFields;

CTaskEdit::CTaskEdit(LStream *inStream)
	: LTable(inStream), fDBFile(0), fFMDb(0), fReadOnlyClick(0)
{
	fDeleted = LoadString(kOtherStrings, kDeletedIndex);
}

CTaskEdit::~CTaskEdit()
{
}
		
static void
SetForegroundTo(int bgColor)
{
	RGBColor bgColorRGB;
	UGAColorRamp::GetColor(bgColor, bgColorRGB);
	RGBForeColor(&bgColorRGB);
}

static void
SetBackgroundToHiliteColor()
{
	RGBColor bgColorRGB;
	::HiliteColor(&bgColorRGB);
	RGBBackColor(&bgColorRGB);
}

void
CTaskEdit::SetDatabaseFile(CDatabaseFile::Ptr theDBFile)
{
	fDBFile = theDBFile;
}

void
CTaskEdit::SetFMDatabase(FileMaker::DatabasePtr theFMdb)
{
	fFMDb = theFMdb;
}

void
CTaskEdit::DrawCell(const TableCellT& inCell)
{
	Rect frame;
	::TextFont(applFont);
	::TextSize(10);
	if (FetchLocalCellFrame(inCell, frame)) {
		int rowNumber = inCell.row - 1;
		if (fFieldDiffers[rowNumber]) {
			SetForegroundTo(colorRamp_Black);
		} else {
			SetForegroundTo(colorRamp_Gray7);
		}
		switch(fColumnID[inCell.col - 1]) {
		case kFieldName:
			DrawFieldName(rowNumber, frame);
			break;
		case kLocalData:
			DrawLocalFieldData(rowNumber, frame);
			break;
		case kRemoteData:
			DrawRemoteFieldData(rowNumber, frame);
			break;
		}
	}
}

void
CTaskEdit::GetTask(CDataTask& task)
{
	task = fTask;
}

void
CTaskEdit::SetupFieldDiffers()
{
	fFieldDiffers.clear();
	if (fLocalData.fDataState == CDataHolder::kDeleted || fRemoteData.fDataState == CDataHolder::kDeleted) {
		vector<bool> allDifferent(fFieldIDsWithRepeat.size(), true);
		fFieldDiffers = allDifferent;
		return;
	}
	
	vector<string>::const_iterator localData = fLocalFields.begin();
	vector<string>::const_iterator remoteData = fRemoteFields.begin();
	for (vector<FieldIDAndRepeat>::const_iterator i = fFieldIDsWithRepeat.begin(); 
		i != fFieldIDsWithRepeat.end(); ++i) {
		bool fieldDiffers = false;
		FMAE::FieldID fieldID(i->GetFieldID());
		if (!fFMDb->FieldReadOnly(fieldID)) {
			fieldDiffers = (*localData != *remoteData);
		}
		fFieldDiffers.push_back(fieldDiffers);
		++remoteData;
		++localData;
	}
}

void
CTaskEdit::SetupColumnIDs()
{
	fColumnID.clear();
	fColumnID.push_back(kFieldName);
	fColumnID.push_back(kLocalData);
	fColumnID.push_back(kRemoteData);
}

void
CTaskEdit::SetupDataHolders()
{
	fTask.GetLocalData(fLocalData);
	fTask.GetRemoteData(fRemoteData);
}

void
CTaskEdit::SetupRowCount()
{
	int newRowCount = fFieldIDs.size();
	TableIndexT rowCount, colCount;
	GetTableSize(rowCount, colCount);
	int delta = newRowCount - rowCount;
	if (delta > 0) {
		InsertRows(delta, max_Int32, 0);
	} else {
		RemoveRows(-delta, 1);
	}
	Refresh();
}

void
CTaskEdit::SetupFieldIDsWithRepeats()
{
	fFieldIDsWithRepeat.clear();
	vector<int>::const_iterator repeatIter = fRepeats.begin();
	for (vector<FMAE::FieldID>::const_iterator i = fFieldIDs.begin(); 
		i != fFieldIDs.end(); ++i) {
		int repeats = *repeatIter;
		++repeatIter;
		for (int j = 1; j <= repeats; ++j) {
			fFieldIDsWithRepeat.push_back(FieldIDAndRepeat(*i, j));
		}
	}
	ThrowIf_(accumulate(fRepeats.begin(), fRepeats.end(), 0) != fFieldIDsWithRepeat.size());
}

void
CTaskEdit::SetupFieldNames()
{
	fFieldNames.clear();
	map<FMAE::FieldID, string> fieldNames = fFMDb->GetFieldNameMap();
	for (vector<FieldIDAndRepeat>::const_iterator i = fFieldIDsWithRepeat.begin(); 
		i != fFieldIDsWithRepeat.end(); ++i) {
		fFieldNames.push_back(JFile5::CWriteJFile5::MakeFieldName(fieldNames[i->GetFieldID()], i->GetRepeatIndex()));
	}
}

vector<string>
CTaskEdit::SetupFieldData(const CDataHolder& holder)
{
	vector<string> theFields;
	vector<string> sourceData(holder.MakeDataVector());
	vector<int>::const_iterator repeatIter = fRepeats.begin();
	vector<string>::const_iterator dataIter = sourceData.begin();
	for (vector<FMAE::FieldID>::const_iterator i = fFieldIDs.begin(); 
		i != fFieldIDs.end(); ++i) {
		int repeats = *repeatIter;
		++repeatIter;
		if (repeats == 1) {
			theFields.push_back(*dataIter);
		} else {
			vector<string> repeatedFields(SplitRepeatingFields(*dataIter, repeats));
			theFields.insert(theFields.end(), repeatedFields.begin(), repeatedFields.end());
		}
		++dataIter;
	}
	ThrowIf_(theFields.size() != fFieldIDsWithRepeat.size());
	return theFields;
}

void
CTaskEdit::SetupFieldData()
{
	fLocalFields = SetupFieldData(fLocalData);
	fRemoteFields = SetupFieldData(fRemoteData);
	CDataHolder mergedHolder;
	fTask.GetMergedData(mergedHolder);
	fMergedFields = SetupFieldData(mergedHolder);
}

void
CTaskEdit::SetupTask()
{
	SetupDataHolders();
	SetupColumnIDs();
	SetupFieldIDsWithRepeats();
	SetupFieldNames();
	SetupFieldData();
	SetupFieldDiffers();
	SetupRowCount();
}

void
CTaskEdit::SetTask(const CDataTask& task, const CFieldIDList& fieldsToEdit, const vector<int>& repeats)
{
	ThrowIfNil_(fDBFile.get());
	ThrowIfNil_(fFMDb.get());
	
	fTask = task;
	
	fFieldIDs = fieldsToEdit;
	fRepeats = repeats;
	
	SetupTask();
}

void
CTaskEdit::DrawRowString(const string& theData, const Rect& r)
{
	string localData(theData);
	string::size_type i;
	do {
		i = localData.find('\r');
		if (i != string::npos) {
			localData.replace(i, 1, ",");
		}
	} while (i != string::npos);
	UTextDrawing::DrawTruncatedWithJust(AsStr255(localData), r, teFlushDefault);
}

static void
SetBackgroundTo(int bgColor)
{
	RGBColor bgColorRGB;
	UGAColorRamp::GetColor(bgColor, bgColorRGB);
	RGBBackColor(&bgColorRGB);
}

void
CTaskEdit::DrawFieldName(int rowNumber, const Rect& localCellRect)
{
	string fieldName(fFieldNames.at(rowNumber));
	::TextFace(bold);
	Rect r = localCellRect;
	r.left += 5;
	DrawRowString(fieldName, r);
}

void
CTaskEdit::DrawLocalFieldData(int rowNumber, const Rect& localCellRect)
{
	::TextFace(normal);
	Rect r = localCellRect;
	r.left += 5;
	FMAE::FieldID fieldID = fFieldIDs[rowNumber];
	string mergeDataString(fMergedFields.at(rowNumber));
	string localDataString(fLocalFields.at(rowNumber));
	if (fLocalData.fDataState == CDataHolder::kDeleted) {
		::TextFace(italic);
		DrawRowString(fDeleted, r);
	} else {
		DrawRowString(localDataString, r);
	}
	if (fFieldDiffers[rowNumber] && localDataString == mergeDataString) {
		UDrawingUtils::SetHiliteModeOn();
		InvertRect(&localCellRect);
	}
}

void
CTaskEdit::DrawRemoteFieldData(int rowNumber, const Rect& localCellRect)
{
	::TextFace(normal);
	Rect r = localCellRect;
	r.left += 5;
	FMAE::FieldID fieldID = fFieldIDs[rowNumber];
	string mergeDataString(fMergedFields.at(rowNumber));
	string remoteDataString(fRemoteFields.at(rowNumber));
	if (fRemoteData.fDataState == CDataHolder::kDeleted) {
		::TextFace(italic);
		DrawRowString(fDeleted, r);
	} else {
		string dataString(fRemoteFields.at(rowNumber));
		DrawRowString(remoteDataString, r);
	}
	if (fFieldDiffers[rowNumber] && remoteDataString == mergeDataString) {
		UDrawingUtils::SetHiliteModeOn();
		::InvertRect(&localCellRect);
	}
}

void
CTaskEdit::RefreshRow(const TableCellT &inCell)
{
	TableIndexT rowCount, colCount;
	GetTableSize(rowCount, colCount);
	TableCellT localCell;
	localCell.row = inCell.row;
	Rect frame;
	for (localCell.col = 2; localCell.col <= colCount; localCell.col += 1) {
		if (FetchLocalCellFrame(localCell, frame)) {
			FocusDraw();
			InvalPortRect(&frame);
		}
	}
}

void
CTaskEdit::ClickCell(const TableCellT &inCell, const SMouseDownEvent&	/* inMouseDown */)
{
	int rowNumber = inCell.row - 1;
	if (fFieldDiffers[rowNumber]) {
		FieldIDAndRepeat fieldID = fFieldIDsWithRepeat[rowNumber];

		if (fMergedFields.at(rowNumber) == fLocalFields.at(rowNumber))
			fMergedFields.at(rowNumber) = fRemoteFields.at(rowNumber);
		else
			fMergedFields.at(rowNumber) = fLocalFields.at(rowNumber);
			
		fTask.SetMergedField(fieldID, fMergedFields.at(rowNumber));

		if (fLocalData.fDataState == CDataHolder::kDeleted || fRemoteData.fDataState == CDataHolder::kDeleted) {
			Refresh();
		} else {
			RefreshRow(inCell);
		}
	}
}

void
CTaskEdit::DrawSelf()
{
	StColorState colorState;
	SetBackgroundTo(colorRamp_White);
	Rect r;
	if (CalcLocalFrameRect(r)) {
		::EraseRect(&r);
	}
	LTable::DrawSelf();
}