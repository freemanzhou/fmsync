#pragma once

#include "FMAE.h"
#include "DebugOutput.h"

template <class T>
class OptField {
public:
	OptField() : fSet(false) {}
	OptField(const T& i) : fSet(true), fValue(i) {}
	
	bool GetValue(T& i) const
	{
		if (fSet) i = fValue;
		return fSet;
	}
	
	bool SetValue(const T& i)
	{
		fValue = i; 
		fSet = true;
	}
	
	void Reset()
	{
		fSet = false;
	}
	
	static void CompareAndSet(const T& orig, const T &current)
	{ 
		if (orig!=current)
			Set(current)
		else
			Reset()
	}

private:

	T fValue;
	bool fSet;
};

class CFieldOverrides {
public:

	void OverrideWith(const CFieldOverrides&);

	static void Read(LStream&, CFieldOverrides&);
	static void Read115(LStream&, CFieldOverrides&);
	static void Write(LStream&, const CFieldOverrides&);
	
	friend bool operator == (const CFieldOverrides&, const CFieldOverrides&);

	map<FMAE::FieldID, string> fName;
	map<FMAE::FieldID, int> fType;
	map<FMAE::FieldID, int> fWidth;
	map<FMAE::FieldID, int> fReadOnly;
	map<FMAE::FieldID, vector<string> > fPopups;
	map<FMAE::FieldID, int> fExtra;
	map<FMAE::FieldID, int> fExtra2;
	map<FMAE::FieldID, string> fFieldCalcValue1;
	map<FMAE::FieldID, string> fFieldCalcValue2;
};


namespace DebugOutput {

inline void
DoOutput(const CFieldOverrides& o)
{
	Output("fName:", o.fName);
	Output("fType:", o.fType);
	Output("fWidth:", o.fWidth);
	Output("fReadOnly:", o.fReadOnly);
	Output("fPopups:", o.fPopups);
	Output("fExtra:", o.fExtra);
	Output("fExtra2:", o.fExtra2);
	Output("fFieldCalcValue1:", o.fFieldCalcValue1);
	Output("fFieldCalcValue2:", o.fFieldCalcValue2);
}

}
