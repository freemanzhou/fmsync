#pragma once

#include "Stringiness.h"

namespace DebugOutput {

template <class T> 
void DoOutput(const T& item)
{
	DoOutput(AsString(item).c_str());
}

template <class T, class U>
void DoOutput(const pair<T,U>& i)
{
	DoOutput("{");
	DoOutput(i.first);
	DoOutput(",");
	DoOutput(i.second);
	DoOutput("}");
}

template <class ForwardIterator>
void DoOutput(ForwardIterator begin, ForwardIterator end)
{
	for (ForwardIterator i = begin; i != end; ++i) {
		if (i != begin) {
			DoOutput(", ");
		}
		DoOutput(*i);
	}
}

template <class T> 
void DoOutput(const vector<T>& item)
{
	DoOutput(item.begin(), item.end());
}

template <class T, class U> 
void DoOutput(const map<T,U>& item)
{
	DoOutput(item.begin(), item.end());
}

void DoOutput(const char* item);
void DoOutput(const int& item);
void DoOutput(const string& item);

template <class T> 
inline void Output(const T& item)
{
#ifdef _DEBUG
	DoOutput(item);
	DoOutput("\r");
#endif	
}

template <class T> 
inline void Output(bool flag, const T& item)
{
#ifdef _DEBUG
	if (flag) {
		DoOutput(item);
		DoOutput("\r");
	}
#endif	
}

template <class T> 
inline void Output(bool flag, const char *label, const T& item)
{
#ifdef _DEBUG
	if (flag) {
		DoOutput(label);
		DoOutput(item);
		DoOutput("\r");
	}
#endif	
}

template <class T> 
inline void Output(const char *label, const T& item)
{
#ifdef _DEBUG
	DoOutput(label);
	DoOutput(item);
	DoOutput("\r");
#endif	
}

}
