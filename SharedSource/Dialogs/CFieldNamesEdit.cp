#include "CFieldNamesEdit.h"
#include "Str255.h"
#include "CDatabaseFile.h"
#include "FMDatabase.h"
#include <UGAColorRamp.h>
#include <UDrawingUtils.h>
#include <LTableSingleSelector.h>
#include <LTableMultiGeometry.h>
#include "FMCResources.h"

CFieldNamesEdit::CFieldNamesEdit(LStream *inStream)
	: LTableView(inStream), fDragging(0), fSelectedRow(0)
{
}

CFieldNamesEdit::~CFieldNamesEdit()
{
}

static int *
VectorToArray(const vector<int>& inVector)
{
	int *p = new int[inVector.size()];
	int *s = p;
	vector<int>::const_iterator i = inVector.begin();
	while (i != inVector.end()) {
		*s++ = *i++;
	}
	return p;
}
		
static map<int,string>
MapStrings(const vector<int>& theInts, const vector<string>& theStrings)
{
	map<int,string> theMap;
	
	vector<int>::const_iterator i = theInts.begin();
	vector<string>::const_iterator s = theStrings.begin();
	
	while (i != theInts.end() && s != theStrings.end()) {
		theMap[*i] = *s;
		++i;
		++s;
	}
	return theMap;
}

void
CFieldNamesEdit::Setup(const vector<string>& leftStrings, const vector<string> rightStrings)
{
	fRightStrings = rightStrings;
	fLeftStrings = leftStrings;
	
	SDimension16 frameSize;
	GetFrameSize(frameSize);
	int colSize = (frameSize.width - 20)/2;
	int centerSize = frameSize.width - (colSize*2);
	fLeftLine = colSize - 1;
	fRightLine = colSize + centerSize;
	//SetTableSelector(new CFieldNamesSelector(this));
	LTableMultiGeometry* g = new LTableMultiGeometry(this, colSize, 16);
	ThrowIfMemFail_(g);
	SetTableGeometry(g);
	InsertCols(3, 0, 0, 0, true);
	SetColWidth(centerSize, 2, 2);
	SetUseDragSelect(true);
	SetCustomHilite(true);

	int rightStringsCount = fRightStrings.size();
	int leftStringsCount = fLeftStrings.size();

	int newRowCount = max(rightStringsCount, leftStringsCount);
	
	TableIndexT rowCount, colCount;
	GetTableSize(rowCount, colCount);
	int delta = newRowCount - rowCount;
	if (delta > 0) {
		InsertRows(delta, max_Int32, 0, 0, false);
	} else {
		RemoveRows(-delta, 1, false);
	}
	
	for (int i = 0; i < newRowCount; i+= 1) {
		fRightFieldOrder.push_back(i);
		if (i < rightStringsCount && i < leftStringsCount) {
			fCopyField.push_back(1);
		} else {
			fCopyField.push_back(0);
		}
	}
	Refresh();
}

void
CFieldNamesEdit::AdjustCursor(
	Point				inPortPt,
	const EventRecord	&inMacEvent)
{
	Point localPoint = inPortPt;
	PortToLocalPoint(localPoint);
	SPoint32 imagePt;
	LocalToImagePoint(localPoint, imagePt);
	STableCell cellHit;
	if (GetCellHitBy(imagePt, cellHit)) {
		switch(cellHit.col - 1) {
		case kRightString:
			UCursor::SetTheCursor(kCURSReorderCursor);
			break;
		case kRowAction:
			UCursor::SetTheCursor(kCURSCheckMarkCursor);
			break;
		case kLeftString:
			UCursor::SetArrow();
			break;
		}
	} else {
		UCursor::SetArrow();
	}
}

void
CFieldNamesEdit::DrawCell(const STableCell		&inCell,
						const Rect				&inLocalRect)
{
	int rowNumber = inCell.row - 1;
	switch(inCell.col - 1) {
	case kRightString:
		DrawRightStringName(rowNumber, inLocalRect);
		break;
	case kRowAction:
		DrawRowActionSymbol(rowNumber, inLocalRect);
		break;
	case kLeftString:
		DrawLeftString(rowNumber, inLocalRect);
		break;
	}
}

void
CFieldNamesEdit::DrawRowString(const string& theData, const Rect& r, int just)
{
	string localData(theData);
	string::size_type i;
	do {
		i = localData.find('\r');
		if (i != string::npos) {
			localData.replace(i, 1, ",");
		}
	} while (i != string::npos);
	UTextDrawing::DrawTruncatedWithJust(AsStr255(localData), r, just);
}

static void
SetBackgroundTo(int bgColor)
{
	RGBColor bgColorRGB;
	UGAColorRamp::GetColor(bgColor, bgColorRGB);
	RGBBackColor(&bgColorRGB);
}

void
CFieldNamesEdit::DrawRightStringName(int rowNumber, const Rect& localCellRect)
{
	::TextFace(bold);
	Rect r = localCellRect;
	r.left += 5;
	r.right -= 5;
	int rowIndex = fRightFieldOrder[rowNumber];
	if (fDragging && rowNumber == fDragTargetIndex) {
		rowIndex = fRightFieldOrder[fDragSourceIndex];
	}
	if (rowIndex < fRightStrings.size()) {
		DrawRowString(fRightStrings[rowIndex], r, teForceLeft);
	}
}

void
CFieldNamesEdit::DrawRowActionSymbol(int rowNumber, const Rect& localCellRect)
{
	::TextFace(bold);
	int rowIndex = fRightFieldOrder[rowNumber];
	if (fDragging && rowNumber == fDragTargetIndex) {
		rowIndex = fRightFieldOrder[fDragSourceIndex];
	}
	if (fCopyField[rowIndex] && rowNumber < fLeftStrings.size() && rowIndex < fRightStrings.size())
		UTextDrawing::DrawTruncatedWithJust("\p->", localCellRect, teCenter);
	else
		UTextDrawing::DrawTruncatedWithJust("\p--", localCellRect, teCenter);
}

void
CFieldNamesEdit::DrawLeftString(int rowNumber, const Rect& localCellRect)
{
	::TextFace(normal);
	if (rowNumber < fLeftStrings.size()) {
		Rect r = localCellRect;
		r.left += 5;
		r.right -= 5;
		DrawRowString(fLeftStrings[rowNumber], r, teFlushRight);
	}
}

void
CFieldNamesEdit::RefreshRow(const STableCell &inCell)
{
	RefreshRow(inCell.row);
}

void
CFieldNamesEdit::RefreshRow(int row)
{
	TableIndexT rowCount, colCount;
	GetTableSize(rowCount, colCount);
	STableCell leftCell, rightCell;
	leftCell.row = row;
	rightCell.row = row;
	leftCell.col = 1;
	rightCell.col = colCount;
	RefreshCellRange(leftCell, rightCell);
}

void
CFieldNamesEdit::HiliteCellActively(
	const STableCell	&inCell,
	Boolean				inHilite)
{
	Rect	cellFrame;
    if (GetLocalCellRect(inCell, cellFrame) && FocusExposed()) {
    	FocusDraw();
    	EraseRect(&cellFrame);
    	DrawCell(inCell, cellFrame);
    	if (inHilite) {
	        StColorPenState saveColorPen;   // Preserve color & pen state
	        StColorPenState::Normalize();
	        UDrawingUtils::SetHiliteModeOn();
			::InvertRect(&cellFrame);
		}
	}
}


void
CFieldNamesEdit::HiliteCellInactively(
	const STableCell	&inCell,
	Boolean				inHilite)
{
	Rect	cellFrame;
	if (GetLocalCellRect(inCell, cellFrame) && FocusExposed()) {
    	FocusDraw();
     	EraseRect(&cellFrame);
    	DrawCell(inCell, cellFrame);
    	if (inHilite) {
    		StColorPenState saveColorPen;   // Preserve color & pen state
	        StColorPenState::Normalize();
			UDrawingUtils::SetHiliteModeOn();
			::PenMode(srcXor);
			::FrameRect(&cellFrame);
		}
	}
}
void
CFieldNamesEdit::ClickCell(const STableCell &inCell, const SMouseDownEvent&	/* inMouseDown */)
{
	int rowNumber = inCell.row - 1;
	switch(inCell.col - 1) {
	case kRightString:
		DoFieldDrag(inCell);
		break;
	case kRowAction:
		int rowIndex = fRightFieldOrder[rowNumber];
		if (rowIndex < fRightStrings.size() && rowNumber < fLeftStrings.size()) {
			if(fCopyField[rowIndex])
				fCopyField[rowIndex] = 0;
			else
				fCopyField[rowIndex] = 1;
			RefreshCell(inCell);
		}
		break;
	case kLeftString:
		break;
	}
}

void
CFieldNamesEdit::DoFieldDrag(const STableCell &inCell)
{
	Boolean		inSameCell = true;
	StartDragging(inCell.row);
	RefreshRow(inCell.row);
	UpdatePort();
	STableCell	currCell = inCell;
	STableCell	hitCell;		//   currently containing the mouse
	while (::StillDown()) {			// Track mouse and select the cell
		SPoint32	imageLoc;
		Point		mouseLoc;
		FocusDraw();
		::GetMouse(&mouseLoc);
		if (AutoScrollImage(mouseLoc)) {
			FocusDraw();
			Rect	frame;
			CalcLocalFrameRect(frame);
			SInt32 pt = ::PinRect(&frame, mouseLoc);
			mouseLoc = *(Point*)&pt;
		}
		LocalToImagePoint(mouseLoc, imageLoc);
		GetCellHitBy(imageLoc, hitCell);
		
		if (currCell != hitCell) {
			inSameCell = false;
			if (IsValidCell(hitCell)) {
				SetDragTargetRow(hitCell.row);
				UpdatePort();
			}
			currCell = hitCell;
		}
	}
	StopDragging(!inSameCell);
	//UpdatePort();
}

void
CFieldNamesEdit::GetSelectionRect(Rect &r)
{
	if (fSelectedRow == 0) {
		::SetRect(&r, 0, 0, 0, 0);
	} else {
		STableCell c1(fSelectedRow, 2);
		GetLocalCellRect(c1, r);
		Rect c2r;
		STableCell c2(fSelectedRow, 3);
		GetLocalCellRect(c2, c2r);
		UnionRect(&r, &c2r, &r);
	}
}

void
CFieldNamesEdit::DrawSelf()
{
	StColorState colorState;
	SetBackgroundTo(colorRamp_White);
	Rect r;
	if (CalcLocalFrameRect(r)) {
		::EraseRect(&r);
	}
	::TextFont(applFont);
	::TextSize(9);
	LTableView::DrawSelf();
	if (fSelectedRow !=0) {
		GetSelectionRect(r);
        StColorPenState saveColorPen;   // Preserve color & pen state
        StColorPenState::Normalize();
        UDrawingUtils::SetHiliteModeOn();
		::InvertRect(&r);
	}
	::MoveTo(fLeftLine, 0);
	::LineTo(fLeftLine, mFrameSize.height);
	::MoveTo(fRightLine, 0);
	::LineTo(fRightLine, mFrameSize.height);
}

void
CFieldNamesEdit::StartDragging(int dragStartRow)
{
	fDragSourceIndex = fDragTargetIndex = dragStartRow-1;
	fDragging = true;
	SelectRow(dragStartRow);
}

void
CFieldNamesEdit::SelectRow(int row)
{
	if (row != fSelectedRow) {
		if(fSelectedRow)
			RefreshRow(fSelectedRow);
		fSelectedRow = row;
		if(fSelectedRow)
			RefreshRow(fSelectedRow);
	}
}

void
CFieldNamesEdit::SetDragTargetRow(int dragTargetRow)
{
	fDragTargetIndex = dragTargetRow-1;
	SelectRow(dragTargetRow);
}

static void
SwapInVector(vector<int>& thisVector, int i1, int i2)
{
	int v1 = thisVector[i1];
	int v2 = thisVector[i2];
	thisVector[i1] = v2;
	thisVector[i2] = v1;
}

void
CFieldNamesEdit::StopDragging(bool doSwap)
{
	if (doSwap && fDragSourceIndex != fDragTargetIndex) {
		SwapInVector(fRightFieldOrder, fDragSourceIndex, fDragTargetIndex);
		//SwapInVector(fCopyField, fDragSourceIndex, fDragTargetIndex);
	}
	RefreshRow(fDragSourceIndex+1);
	RefreshRow(fDragTargetIndex+1);
	fDragging = false;
	fDragTargetIndex = -1;
	fDragSourceIndex = -1;
}


CFieldNamesSelector::CFieldNamesSelector(LTableView *inTableView)
	: LTableMultiSelector(inTableView)
{
}

Boolean
CFieldNamesSelector::DragSelect(
	const STableCell		&inCell,
	const SMouseDownEvent&	/* inMouseDown */)
{
	Boolean		inSameCell = true;
	
	UnselectAllCells(false);
	STableCell c1(inCell.row, 2);
	STableCell c2(inCell.row, 3);
	SelectCellBlock(c1, c2);
	
	STableCell	currCell = inCell;
	
	while (::StillDown()) {			// Track mouse and select the cell
		STableCell	hitCell;		//   currently containing the mouse
		SPoint32	imageLoc;
		Point		mouseLoc;
		mTableView->FocusDraw();
		::GetMouse(&mouseLoc);
		if (mTableView->AutoScrollImage(mouseLoc)) {
			mTableView->FocusDraw();
			Rect	frame;
			mTableView->CalcLocalFrameRect(frame);
			SInt32 pt = ::PinRect(&frame, mouseLoc);
			mouseLoc = *(Point*)&pt;
		}
		mTableView->LocalToImagePoint(mouseLoc, imageLoc);
		mTableView->GetCellHitBy(imageLoc, hitCell);
		
		if (currCell != hitCell) {
			inSameCell = false;
	
			if (mTableView->IsValidCell(hitCell)) {
				UnselectAllCells(false);
				STableCell c1(hitCell.row, 2);
				STableCell c2(hitCell.row, 3);
				SelectCellBlock(c1, c2);
				currCell = hitCell;
			}
		}
	}
	
	return inSameCell;
}

vector<int>
CFieldNamesEdit::GetFieldSelection()
{
	return fCopyField;
}

vector<int>
CFieldNamesEdit::GetOrder()
{
	return fRightFieldOrder;
}
