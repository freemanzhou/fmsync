#include "CAEDescriptor.h"
#include "Descriptors.h"
#include "PaneUtilities.h"
#include "Str255.h"

#include <LEditText.h>
#include <LStaticText.h>
#include <LGAFocusBorder.h>
#include <LPopupButton.h>
#include <LScrollerView.h>
#include <vector>

void SetPopupButton(LPopupButton *button, const vector<string>& namesToAppend)
{
	ThrowIfNil_(button);
	button->DeleteAllMenuItems();
	for (int i = 0; i < namesToAppend.size(); i += 1) {
		button->AppendMenu(AsStr255(namesToAppend[i]), true);
	}
}

std::string GetTextFromEditBox(LEditText* editor)
{
	Size len;
	editor->GetText(0, 0, &len);
	std::vector<char> buffer(len);
	editor->GetText(&buffer[0], buffer.size(), &len);
	return std::string(&buffer[0], buffer.size());
}

std::string GetTextFromEditBox(LTextEditView* editor)
{
	Handle h(editor->GetTextHandle());
	StHandleLocker hl(h);
	return string(reinterpret_cast<const char*>(*h), GetHandleSize(h));
}

std::string GetTextFromEditBox(LStaticText* editor)
{
	Size len;
	editor->GetText(0, 0, &len);
	std::vector<char> buffer(len);
	editor->GetText(&buffer[0], buffer.size(), &len);
	return std::string(&buffer[0], buffer.size());
}

void FillEditBox(LEditText* editor,const std::string& s)
{
	editor->SetText((Ptr)s.data(), s.length());
}

void FillEditBox(LTextEditView* editor,const std::string& s)
{
	editor->SetTextPtr((Ptr)s.data(), s.length());
}

void FillStaticTextBox(LStaticText* box,const std::string& s)
{
	box->SetText((Ptr)s.data(), s.length());
}

void SetButtonEnabled(LPane* b, bool v)
{
	if (v) {
		b->Enable();
	} else {
		b->Disable();
	}
}

static void AddRectToRegion(RgnHandle rh, const Rect& r)
{
	StRegion rectRgn(r);
	UnionRgn(rh, rectRgn, rh);
}

static void AddHandleRegion(RgnHandle rg, int top, int left, int hdlSize)
{
	Rect handleR;
	handleR.top = top;
	handleR.bottom = handleR.top + hdlSize;
	handleR.left = left;
	handleR.right = handleR.left + hdlSize;
	AddRectToRegion(rg, handleR);
}

void MakePaneHandleRegion(LPane* b, RgnHandle h)
{
	Rect r;
	b->CalcLocalFrameRect(r);
	
	SetEmptyRgn(h);
	const int kHandleSize = 5;
	AddHandleRegion(h, r.top, r.left, kHandleSize);
	AddHandleRegion(h, r.top, r.right - kHandleSize, kHandleSize);
	AddHandleRegion(h, r.bottom - kHandleSize, r.left, kHandleSize);
	AddHandleRegion(h, r.bottom - kHandleSize, r.right - kHandleSize, kHandleSize);
}

void DrawPaneHandles(LPane* b, bool isActive)
{
	static RGBColor rgbBlack = {0,0,0};
	StColorPortState portState(UQDGlobals::GetCurrentPort());
	StRegion rh;
	MakePaneHandleRegion(b, rh);
	RGBForeColor(&rgbBlack);
	if (isActive)
		PaintRgn(rh);
	else
		FrameRgn(rh);
}

std::vector<TableIndexT> GetSelectedIndexes(LTableView* view)
{
	std::vector<TableIndexT> selectedIndexes;
	STableCell cell(view->GetFirstSelectedCell());
	while (view->IsValidCell(cell)) {
		TableIndexT index;
		view->CellToIndex(cell, index);
		selectedIndexes.push_back(index-1);
		view->GetNextSelectedCell(cell);
	}
	return selectedIndexes;
}

std::vector<STableCell> GetSelectedCells(LTableView* view)
{
	std::vector<STableCell> selected;
	STableCell cell(view->GetFirstSelectedCell());
	while (view->IsValidCell(cell)) {
		selected.push_back(cell);
		view->GetNextSelectedCell(cell);
	}
	return selected;
}

void LinkEnclosedBroadcasters(LView* v, LListener* l)
{
	TArrayIterator<LPane*> iterator(v->GetSubPanes());
	LPane	*theSub;
	while (iterator.Next(theSub)) {
		LBroadcaster* b = dynamic_cast<LBroadcaster*>(theSub);
		if (b)
			b->AddListener(l);
		LView *subV = dynamic_cast<LView*>(theSub);
		if (subV)
			LinkEnclosedBroadcasters(subV, l);
	}
}

void AttachFocusBox(LPane* p, PaneIDT theID)
{
	LPane* cp = p->FindPaneByID(theID);
	ThrowIfNil_(cp);
	LCommander* commander = dynamic_cast<LCommander*>(cp);
	if (commander == 0)	
		return;
		
	LView* superView = p->GetSuperView();
	SPoint32 pLoc;
	p->GetFrameLocation(pLoc);
	SDimension16 frameSize;
	p->GetFrameSize(frameSize);

	LGAFocusBorder *border = new LGAFocusBorder();
	border->SetCanFocus(true);
	border->SetSuperCommander(commander->GetSuperCommander());
	commander->SetSuperCommander(border);
	border->PutInside(superView, true);
	border->PlaceInSuperFrameAt(pLoc.h - 3, pLoc.v - 3, true);
	border->ResizeFrameTo(frameSize.width + 6, frameSize.height + 6, true);
	p->PutInside(border, true);
	border->SetCommanderSubPaneByID(theID);
	border->SetInsetSubPane(p);
}

#if __PowerPlant__ > 0x02108000
void AttachFocusBoxToScrollers(LView* v)
{
	TArrayIterator<LPane*> iterator(v->GetSubPanes());
	LPane	*theSub;
	while (iterator.Next(theSub)) {
		LScrollerView* scv = dynamic_cast<LScrollerView*>(theSub);
		if (scv) {
			LView* targetView = scv->GetScrollingView();
			LCommander* commander = dynamic_cast<LCommander*>(targetView);
			if (commander)
				AttachFocusBox(scv, scv->GetScrollingView()->GetPaneID());
		}
		LView *subV = dynamic_cast<LView*>(theSub);
		if (subV)
			AttachFocusBoxToScrollers(subV);
	}
}
#endif


#if TARGET_API_MAC_CARBON
template <> string GetDataFromPane<LEditText>(LEditText* p)
{
	return GetTextFromEditBox(p);
}

template <> void SetPaneData<LEditText>(LEditText* p, const string& d)
{
	FillEditBox(p, d);
}

template <> string GetDataFromPane<LTextEditView>(LTextEditView* p)
{
	return GetTextFromEditBox(p);
}

template <> void SetPaneData<LTextEditView>(LTextEditView* p, const string& d)
{
	FillEditBox(p, d);
}
#endif