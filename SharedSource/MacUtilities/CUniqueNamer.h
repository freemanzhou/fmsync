#pragma once

#include "stringiness.h"

class NoCaseComparePred {
public:
	NoCaseComparePred(const string& searchString);

	bool operator() (const string& a) {
		return cmp_nocase(a, fSearchString) == 0;
	};
	
	string fSearchString;
};

NoCaseComparePred::NoCaseComparePred(const string& searchString)
	: fSearchString(searchString)
{
}

class CUniqueNamer {
public:
		CUniqueNamer();
		CUniqueNamer(const vector<string>& existingNames);
		
		bool			NameIsOK(const string& testName);
		string			MakeValidName(const string& baseName, int startingIndex = 1);

private:
		string			BaseName(const string& testName);

		vector<string>	fNames;
};