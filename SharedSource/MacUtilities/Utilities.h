#pragma once

#include <map>
#include <vector>

#include "OpaqueInteger.h"

#include <PP_Prefix.h>
#include <LFile.h>
#include <UGWorld.h>
#include <LString.h>

class LView;
class LListBox;
class LPopupButton;

template <class T>
inline void (Clamp) (T& x, T minValue, T maxValue)
{ 
	if (x < minValue) x = minValue;
	if (x > maxValue) x = maxValue;
}

template <class T>
inline void (ByteClamp) (T& x)
{ 
	if (x < 0) x = 0;
	if (x > 255) x = 255;
}

void AppendToPopupButton(LPopupButton *button, bool addSeparator, const std::vector<std::string>& namesToAppend);
void Normalize(const FSSpec& inFileSpec, FSSpec* outSpec);
bool FileExists(const FSSpec& inFileSpec);
void GetPreferencesFolder(FSSpec& spec);
short GetBootDisk();
long GetDesktopFolderOf(short);
long GetBootDiskDesktopFolder();
Ptr LockGWorldPixels(LGWorld* world);
void UnlockGWorldPixels(LGWorld* world);
int GWorldRowBytes(LGWorld* world);
OSErr ResolveAlias(AliasHandle aliasH, FSSpec& outFileSpec);
bool AliasHandlesEquivalent(AliasHandle a, AliasHandle b);
int CountItems(const AEDescList &);
inline int CountItems(const AEDescList *theList) {return CountItems(*theList);}

std::string GetText(LTextEditView* view);

void WriteStringAsText(LStream& inStream, const std::string& theString);

class CFSSpec {
public:
		CFSSpec();
		CFSSpec(const FSSpec& spec);
		
		operator const FSSpec * () const	{ return &fSpec; }
		operator FSSpec *	()		{ return &fSpec; }

private:
	FSSpec fSpec;

};

inline GrafPtr MakeGrafPtr(LGWorld *world)
		{
			return (GrafPtr)world->GetMacGWorld();
		}

inline const BitMap * MakeBitMapPtr(LGWorld *world)
		{
			GrafPtr ptr = (GrafPtr)world->GetMacGWorld();
#if TARGET_API_MAC_CARBON
			return GetPortBitMapForCopyBits(ptr);
#else
			return &ptr->portBits;
#endif
		}

inline GrafPtr MakeGrafPtr(StOffscreenGWorld &world)
		{
			return (GrafPtr)world.GetMacGWorld();
		}

void FillListBox(LListBox* theBox, const std::vector<std::string>& theStrings);

template <class T>
int FindIndex(const std::vector<T>& theList, const T& theItem)
{
	std::vector<T>::const_iterator i = find(theList.begin(), theList.end(), theItem);
	ThrowIf_(i == theList.end());
	return i - theList.begin();
}

template <class T, class U>
std::vector<T>
MakeOrderedList(const std::map<U, T>& theMap, const std::vector<U>& theKeys)
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
void
AddToMap(const std::vector<U>& theKeys, const std::vector<T>& values, std::map<U, T>& theResult)
{
	std::vector<U>::const_iterator i;
	std::vector<T>::const_iterator j;
	for (i = theKeys.begin(), j = values.begin(); i != theKeys.end() && j != values.end();
		++i, ++j) {
		theResult[*i] = *j;
	}
}

template<class K, class V>
void MergeMap(std::map<K,V>& baseMap, const std::map<K,V>& deltas)
{
	for (std::map<K,V>::const_iterator i = deltas.begin();
			i != deltas.end(); ++i) {
		baseMap[i->first] = i->second;
	}
}

template <class T, class U>
std::map<U, T>
MakeMap(const std::vector<U>& theKeys, const std::vector<T>& values)
{
	std::map<U, T> theResult;
	AddToMap(theKeys, values, theResult);
	return theResult;
}

template <class T, class U>
std::map<U, T>
MakeSubset(const std::vector<U>& theKeys, const std::map<U, T>& sourceMap)
{
	std::vector<T> values(MakeOrderedList(sourceMap, theKeys));
	return MakeMap(theKeys, values);
}

template <class T, class U>
std::map<U, T>
MakeDiffMap(const std::map<U, T>& baseMap, const std::map<U, T>& modifiedMap)
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
void
ApplyDiffMap(std::map<U, T>& baseMap, const std::map<U, T>& diffMap)
{
	typedef std::map<U, T> M;
	M diffsMap;
	
	for (M::const_iterator i = diffMap.begin(); i != diffMap.end(); i++) {
		baseMap[i->first] = i->second;
	}
}

void ConfirmFreeStackSpace();

template <class T>
void
SetupOneMenu(const std::vector<T>& scriptIDList, T scriptID, int extraItems, LPopupButton* thePopup)
{
	int index = 1;
	if (scriptID) {
		index = FindIndex(scriptIDList, scriptID) + extraItems + 1;
	}
	thePopup->SetValue(index);
}

inline
Boolean operator!=(
				const RGBColor&	inLhs,
				const RGBColor&	inRhs)
		{
			return inLhs.red != inRhs.red || inLhs.green != inRhs.green || inLhs.blue != inRhs.blue;
		}
