#pragma once

namespace {
	
}

template <class T>
class CIDFactory {
public:
	CIDFactory() : fLastID(0) { }
	
	int		GetID(const T&);

private:
	int fLastID;
	map<T, int> fIDMap;
};

template <class T>
int
CIDFactory<T>::GetID(const T& target)
{
	map<T, int>::const_iterator f = fIDMap.find(target);
	if (f != fIDMap.end())
		return f->second;
	
	++fLastID;
	fIDMap[target] = fLastID;
	return fLastID;
}
