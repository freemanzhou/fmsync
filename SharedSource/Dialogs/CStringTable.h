#pragma once

#include <LTable.h>
#include <LBroadcaster.h>
#include <vector>
#include <string>

class CStringTable : public LTable, public LBroadcaster
{
public:
	enum { class_ID = FOUR_CHAR_CODE('STab') };
	typedef std::vector<std::string> StringList;
	typedef std::vector<int> ColumnWidths;
	
	enum {kSelectionChanged = 'SelC'};

		CStringTable(LStream *inStream);
		virtual ~CStringTable();

	virtual void	ClickSelf(const SMouseDownEvent &inMouseDown);
	void	SetDoubleClickCommand(CommandT inCommand);
	
	bool	HasSelection();				
	bool	SelectedIndex(int& index);				
	void	SelectIndex(int index);
	void	SelectNone();				

	void	SetStrings(const StringList& strings, const ColumnWidths& cols);
	void	SetStrings(const StringList& strings);
	void	ReplaceStrings(const TableCellT& inCell, const StringList& strings);
	
protected:
	virtual void	UnhiliteCell(const TableCellT &inCell);
	virtual void	DrawCell(const TableCellT &inCell);

private:
	
	StringList	fStrings;
	ColumnWidths		fColumns;
	CommandT		fDoubleClickCommand;
	
};

