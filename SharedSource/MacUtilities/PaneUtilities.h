#pragma once

#include <string>
#include <URegistrar.h>
#include <vector>

class LEditText;
class LStaticText;
class LTextEditView;
class LTableView;
class LPane;
class CAEDescriptor;
class LPopupButton;

std::string GetTextFromEditBox(LStaticText*);
std::string GetTextFromEditBox(LEditText*);
std::string GetTextFromEditBox(LTextEditView*);
void FillEditBox(LEditText*,const std::string&);
void FillEditBox(LTextEditView*,const std::string&);
void FillStaticTextBox(LStaticText* box,const std::string& s);
void SetButtonEnabled(LPane* b, bool v);
void LinkEnclosedBroadcasters(LView* v, LListener* l);
void SetPopupButton(LPopupButton *button, const vector<string>& namesToAppend);

void MakePaneHandleRegion(LPane* b, RgnHandle h);
void DrawPaneHandles(LPane* b, bool isActive = true);
void AttachFocusBox(LPane* p, PaneIDT theID);
void AttachFocusBoxToScrollers(LView* v);

std::vector<TableIndexT> GetSelectedIndexes(LTableView* view);
std::vector<STableCell> GetSelectedCells(LTableView* view);

template <class T>
class RegisterSelf {
public:
	RegisterSelf() {
		PP_PowerPlant::TRegistrar<T >::Register();
	}
};

template <class T>
void FindPane(LView *view, const PaneIDT& paneID, T** result)
{
	*result = 0;
	LPane* paneP = view->FindPaneByID(paneID);
	ThrowIfNil_(paneP);
	T* temp = dynamic_cast<T*>(paneP);
	ThrowIfNil_(temp);
	*result = temp;
}

template <class T, class S>
T* DownCast(S* p)
{
	T* d = dynamic_cast<T*>(p);
	ThrowIfNil_(d);
	return d;
}

template <class T> string GetDataFromPane(T* p);
template <class T> void SetPaneData(T* p, const string&);

template <> string GetDataFromPane<LStaticText>(LStaticText* p);
template <> void SetPaneData<LStaticText>(LStaticText* p, const string&);

template <> string GetDataFromPane<LEditText>(LEditText* p);
template <> void SetPaneData<LEditText>(LEditText* p, const string&);

template <> string GetDataFromPane<LTextEditView>(LTextEditView* p);
template <> void SetPaneData<LTextEditView>(LTextEditView* p, const string&);

template <class T>
class DisableMessage {
public:
	DisableMessage(T* theTarget) : fMessage(theTarget->GetValueMessage()), fTarget(theTarget)
	{
		fTarget->SetValueMessage(0);
	}
	~DisableMessage()
	{
		fTarget->SetValueMessage(fMessage);
	}

private:
	MessageT fMessage;
	T* fTarget;
};
