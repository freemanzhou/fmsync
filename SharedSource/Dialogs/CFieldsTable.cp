#include "CFieldsTable.h"
#include "Str255.h"
#include "UGAColorRamp.h"
#include <UDrawingUtils.h>

CFieldsTable::CFieldsTable(LStream *inStream)
	: LTable(inStream), fDoubleClickCommand(cmd_Nothing)
{
}

CFieldsTable::~CFieldsTable()
{
}
		
void
CFieldsTable::SetDoubleClickCommand(CommandT inCommand)
{
	fDoubleClickCommand = inCommand;
}

void
CFieldsTable::ClickSelf(const SMouseDownEvent &inMouseDown)
{
	if (sClickCount > 1 && fDoubleClickCommand != cmd_Nothing) {
		BroadcastMessage(fDoubleClickCommand, 0);
	} else {
		LTable::ClickSelf(inMouseDown);
	}
}
void
CFieldsTable::DrawCell(const TableCellT& inCell)
{
	int cols = fColumns.size();
	int stringIndex = (inCell.row-1) * cols;
	if (stringIndex >= fStrings.size())
		return;
	
	int endIndex = stringIndex + cols;
	if (endIndex >= fStrings.size())
		endIndex = fStrings.size();
		
	Rect frame;
	::TextFont(kFontIDGeneva);
	::TextSize(10);
	vector<string>::const_iterator stringPtr = fStrings.begin() + stringIndex;
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
CFieldsTable::ReplaceStrings(const TableCellT& inCell, const vector<string>& strings)
{
	int cols = fColumns.size();
	int stringIndex = (inCell.row-1) * cols;
	vector<string>::const_iterator sourcePtr = strings.begin();
	vector<string>::iterator stringPtr = fStrings.begin() + stringIndex;
	vector<string>::iterator endPtr = stringPtr + cols;
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
CFieldsTable::SetStrings(const vector<string>& strings, const vector<int>& cols)
{
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
}
