#include "CStringTable.h"
#include "Str255.h"
#include "UGAColorRamp.h"
#include <UDrawingUtils.h>

CStringTable::CStringTable(LStream *inStream)
	: LTable(inStream), fDoubleClickCommand(cmd_Nothing)
{
}

CStringTable::~CStringTable()
{
}
		
void
CStringTable::SetDoubleClickCommand(CommandT inCommand)
{
	fDoubleClickCommand = inCommand;
}

void
CStringTable::ClickSelf(const SMouseDownEvent &inMouseDown)
{
	if (sClickCount > 1 && fDoubleClickCommand != cmd_Nothing) {
		BroadcastMessage(fDoubleClickCommand, 0);
	} else {
		TableCellT oldSelect, newSelect;
		GetSelectedCell(oldSelect);
		LTable::ClickSelf(inMouseDown);
		GetSelectedCell(newSelect);
		if (!EqualCell(oldSelect,newSelect)) {
			BroadcastMessage(kSelectionChanged, 0);
		}
	}
}
void
CStringTable::DrawCell(const TableCellT& inCell)
{
	int cols = fColumns.size();
	int stringIndex = (inCell.row-1) * cols;
	if (stringIndex >= fStrings.size())
		return;
	
	int endIndex = stringIndex + cols;
	if (endIndex >= fStrings.size())
		endIndex = fStrings.size();
		
	Rect frame;
	::TextFont(applFont);
	::TextSize(10);
	::TextFace(0);
	StringList::const_iterator stringPtr = fStrings.begin() + stringIndex;
	if (FetchLocalCellFrame(inCell, frame)) {
		for (int i = 0; i < cols; i += 1) {
			Rect r = frame;
			r.left = fColumns[i];
			if (i < cols - 1)
				r.right =  fColumns[i+1];
			LStr255 textToDraw(stringPtr->c_str());
			UTextDrawing::DrawWithJustification((Ptr)&textToDraw[1], textToDraw[0], r, teFlushDefault);
			++stringPtr;
		}
	}
}

void
CStringTable::ReplaceStrings(const TableCellT& inCell, const StringList& strings)
{
	int cols = fColumns.size();
	int stringIndex = (inCell.row-1) * cols;
	StringList::const_iterator sourcePtr = strings.begin();
	StringList::iterator stringPtr = fStrings.begin() + stringIndex;
	StringList::iterator endPtr = stringPtr + cols;
	while (stringPtr != endPtr) {
		*stringPtr = *sourcePtr;
		++stringPtr;
		++sourcePtr;
	}
	Rect frame;
	if (FetchLocalCellFrame(inCell, frame)) {
		FocusDraw();
		InvalPortRect(&frame);
	}
}

void
CStringTable::SetStrings(const StringList& strings)
{
	ColumnWidths cols;
	cols.push_back(10);
	SetStrings(strings, cols);
}

void
CStringTable::SetStrings(const StringList& strings, const ColumnWidths& cols)
{
	if (fStrings == strings && fColumns == cols)
		return;
	fStrings = strings;
	fColumns = cols;
	int newColCount = fColumns.size();
	int newRowCount = (strings.size() + newColCount - 1)/newColCount;
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

bool
CStringTable::HasSelection()
{
	int index;
	return SelectedIndex(index);
}

bool
CStringTable::SelectedIndex(int& index)
{
	TableCellT cell;
	GetSelectedCell(cell);

	if (!IsValidCell(cell)) {
		index = -1;
		return false;
	}
		
	index = cell.row-1;
	return true;
}

void
CStringTable::SelectIndex(int index)
{
	TableCellT cell;
	cell.row = index + 1;
	cell.col = 1;
	SelectCell(cell);
}

void
CStringTable::SelectNone()
{
	TableCellT noCell;
	noCell.row = 0;
	noCell.col = 0;
	SelectCell(noCell);
}

void
CStringTable::UnhiliteCell(
	const TableCellT& inCell)
{
	if (IsVisible())
		LTable::UnhiliteCell(inCell);
}

