#pragma once

#include <map>

template <class T, class U>
std::vector<T> MakeOrderedList(const std::map<U, T>& theMap, const std::vector<U>& theKeys)
{
	std::vector<T> theResult;
	std::vector<U>::const_iterator i = theKeys.begin();
	while(i != theKeys.end()) {
		std::map<U,T>::const_iterator f = theMap.find(*i);
		if (f == theMap.end())
			theResult.push_back(T());
		else
			theResult.push_back(f->second);
		++i;
	}
	return theResult;
}

template <class T, class U>
void AddToMap(const std::vector<U>& theKeys, const std::vector<T>& values, std::map<U, T>& theResult)
{
	std::vector<U>::const_iterator i;
	std::vector<T>::const_iterator j;
	for (i = theKeys.begin(), j = values.begin(); i != theKeys.end() && j != values.end();
		++i, ++j) {
		theResult[*i] = *j;
	}
}

template<class K, class V> void MergeMap(std::map<K,V>& baseMap, const std::map<K,V>& deltas)
{
	for (std::map<K,V>::const_iterator i = deltas.begin();
			i != deltas.end(); ++i) {
		baseMap[i->first] = i->second;
	}
}

template <class T, class U>
std::map<U, T> MakeMap(const std::vector<U>& theKeys, const std::vector<T>& values)
{
	std::map<U, T> theResult;
	AddToMap(theKeys, values, theResult);
	return theResult;
}

template <class T, class U>
std::map<U, T> MakeSubset(const std::vector<U>& theKeys, const std::map<U, T>& sourceMap)
{
	std::vector<T> values(MakeOrderedList(sourceMap, theKeys));
	return MakeMap(theKeys, values);
}

template <class T, class U>
std::map<U, T> MakeDiffMap(const std::map<U, T>& baseMap, const std::map<U, T>& modifiedMap)
{
	typedef std::map<U, T> M;
	M diffsMap;
	
	for (M::const_iterator i = modifiedMap.begin(); i != modifiedMap.end(); i++) {
		M::const_iterator f = baseMap.find(i->first);
		if (f == baseMap.end() || f->second != i->second) {
			diffsMap[i->first] = i->second;
		}
	}
	return diffsMap;
}

template <class T, class U>
void ApplyDiffMap(std::map<U, T>& baseMap, const std::map<U, T>& diffMap)
{
	typedef std::map<U, T> M;
	M diffsMap;
	
	for (M::const_iterator i = diffMap.begin(); i != diffMap.end(); i++) {
		baseMap[i->first] = i->second;
	}
}
