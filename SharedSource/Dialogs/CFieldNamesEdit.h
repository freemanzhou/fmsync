#pragma once

#include <LTableView.h>
#include <LTableMultiSelector.h>
#include <LBroadcaster.h>
#include <vector>
#include <map>
#include <string>
#include "CDataTask.h"

class CDatabaseFile;

class CFieldNamesSelector : public LTableMultiSelector
{
public:
						CFieldNamesSelector(
								LTableView				*inTableView);

	virtual Boolean		DragSelect(
								const STableCell		&inCell,
								const SMouseDownEvent	&inMouseDown);
};

class CFieldNamesEdit : public LTableView, public LBroadcaster
{
public:
	enum { class_ID = FOUR_CHAR_CODE('FNEd') };

		CFieldNamesEdit(LStream *inStream);
		virtual ~CFieldNamesEdit();

	void			Setup(const vector<string>& leftName, const vector<string> rightNames);
	vector<int>		GetFieldActions();
	vector<int>		GetFieldOrder();
	
	virtual void	DrawSelf();
	void			RefreshRow(const STableCell &inCell);
	void			RefreshRow(int row);
	
	virtual void		HiliteCellActively(
								const STableCell		&inCell,
								Boolean					inHilite);
	virtual void		HiliteCellInactively(
								const STableCell		&inCell,
								Boolean					inHilite);

	void			StartDragging(int dragStartRow);
	void			SetDragTargetRow(int dragTargetRow);
	void			StopDragging(bool doSwap);
	
	vector<int>		GetFieldSelection();
	vector<int>		GetOrder();
	
protected:
	virtual void		DrawCell(
								const STableCell		&inCell,
								const Rect				&inLocalRect);
	virtual void		ClickCell(
								const STableCell		&inCell,
								const SMouseDownEvent	&inMouseDown);
	virtual void		AdjustCursor(
								Point				inPortPt,
								const EventRecord	&inMacEvent);
private:
	enum 			{kLeftString, kRowAction, kRightString};
	void			DoFieldDrag(const STableCell &inCell);
	void			GetSelectionRect(Rect &r);
	void			SelectRow(int row);
	
	void			DrawRightStringName(int rowNumber, const Rect& localCellRect);
	void			DrawRowActionSymbol(int rowNumber, const Rect& localCellRect);
	void			DrawLeftString(int rowNumber, const Rect& localCellRect);
	
	void			DrawRowString(const string& theData, const Rect& r, int just = teFlushDefault); 
	
	vector<int>			fRightFieldOrder;
	vector<int>			fCopyField;
	vector<string>		fRightStrings;
	vector<string>		fLeftStrings;
	int					fLeftLine;
	int					fRightLine;
	int					fSelectedRow;
	int					fDragSourceIndex;
	int					fDragTargetIndex;
	bool				fDragging;
};

