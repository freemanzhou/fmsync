#pragma once

#include <LTable.h>
#include <LBroadcaster.h>
#include <vector>
#include <string>

class CFieldsTable : public LTable, public LBroadcaster
{
public:
	enum { class_ID = FOUR_CHAR_CODE('STab') };

		CFieldsTable(LStream *inStream);
		virtual ~CFieldsTable();

	virtual void	ClickSelf(const SMouseDownEvent &inMouseDown);

	void	SetColumns(const vector<string>& strings, const vector<int>& cols);
	void	ReplaceStrings(const TableCellT& inCell, const vector<string>& strings);
	
protected:
	virtual void	DrawCell(const TableCellT &inCell);

private:
	
	vector<string>	fStrings;
	vector<int>		fColumns;
	
};

