#include "CUniqueNamer.h"
#include "Stringiness.h"

CUniqueNamer::CUniqueNamer()
{
}

CUniqueNamer::CUniqueNamer(const vector<string>& existingNames)
	: fNames(existingNames)
{
}

bool
CUniqueNamer::NameIsOK(const string& testName)
{
	vector<string>::const_iterator i = 
		find_if(fNames.begin(), fNames.end(), NoCaseComparePred(testName));
	return (i == fNames.end());
}

string
CUniqueNamer::MakeValidName(const string& testName, int startingIndex)
{	
	if (NameIsOK(testName))
		return testName;
		
	string ourBaseName(BaseName(testName));
	string newName(ourBaseName);
	const string spaceString(" ");
	int index = startingIndex;
	while (!NameIsOK(newName)) {
		newName = ourBaseName + spaceString + AsString(index);
		index += 1;
	}
	return newName;
}

string
CUniqueNamer::BaseName(const string& testName)
{
	vector<string> words(SplitString(testName, " "));
	int wordCount = words.size();
	if (wordCount <= 1)
		return testName;
	
	int lastWordIndex = wordCount - 1;
	int index = atoi(words[lastWordIndex].c_str());
	if (index == 0)
		return testName;
		
	words[lastWordIndex] = AsString(index + 1);
	return JoinString(words, " ");
}