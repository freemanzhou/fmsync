#include "Utilities.h"
#include "FolderUtilities.h"
#include "Stringiness.h"
#include "Str255.h"
#include "LErrorMsg.h"
#include "CFileMaker.h"
#ifdef CONDUIT
#include "SyncMgr.h"
#endif

#include <LPopupButton.h>
#include <LFile.h>
#include <locale>

void
AppendToPopupButton(LPopupButton *button, bool addSeparator, const vector<string>& namesToAppend)
{
	ThrowIfNil_(button);
	int totalToAdd = namesToAppend.size();
	if (totalToAdd > 0 && addSeparator) {
		button->AppendMenu("\p-", false);
	}
	for (int i = 0; i < totalToAdd; i += 1) {
		button->AppendMenu(AsStr255(namesToAppend[i]), true);
	}
}

void
ClearBlock(void *p, int length)
{
	char *ptr = (char *)p;
	char *limitPtr = ptr + length;
	
	while (ptr < limitPtr)
		*ptr++ = 0;
}

void GetPreferencesFolder(FSSpec& spec)
{
	ThrowIfOSErr_(::FindFolder(kOnSystemDisk, kPreferencesFolderType, true, 
				&spec.vRefNum, &spec.parID));
}

CFSSpec::CFSSpec()
{
	fSpec.name[0] = 0;
	fSpec.vRefNum = 0;
	fSpec.parID = 0;
}

CFSSpec::CFSSpec(const FSSpec& spec)
	: fSpec(spec)
{
}

Ptr LockGWorldPixels(LGWorld* world)
{
	GWorldPtr gworld = world->GetMacGWorld();
	PixMapHandle aPixMap = GetGWorldPixMap(gworld);
	ThrowIfNot_(LockPixels(aPixMap));
	return GetPixBaseAddr(aPixMap);
}

void UnlockGWorldPixels(LGWorld* world)
{
	GWorldPtr gworld = world->GetMacGWorld();
	PixMapHandle aPixMap = GetGWorldPixMap(gworld);
	UnlockPixels(aPixMap);
}

int GWorldRowBytes(LGWorld* world)
{
	GWorldPtr gworld = world->GetMacGWorld();
	PixMapHandle aPixMap = GetGWorldPixMap(gworld);
	return (**aPixMap).rowBytes & 0x7fff;
}

void Normalize(const FSSpec& inFileSpec, FSSpec* outSpec)
{
	ThrowIfOSErr_(::FSMakeFSSpec(inFileSpec.vRefNum, inFileSpec.parID, 
			inFileSpec.name, outSpec));
}

bool FileExists(const FSSpec& inFileSpec)
{
	FSSpec outSpec;
	OSErr err =::FSMakeFSSpec(inFileSpec.vRefNum, inFileSpec.parID, 
			inFileSpec.name, &outSpec);
	return err == noErr;
}

#if 0

void WriteString(LStream& inStream, const string& theString)
{
	long stringLength = theString.length();
	inStream << stringLength;
	inStream.WriteBlock(theString.data(), stringLength);
	
}

void WriteMap(LStream& inStream, const map<int, int>& theMap)
{
	long mapLength = theMap.size();
	inStream << mapLength;
	map<int, int>::const_iterator i = theMap.begin();
	while(i != theMap.end()) {
		inStream << (long)i->first;
		inStream << (long)i->second;
		++i;
	}
}

void WriteMap(LStream& inStream, const map<int, string>& theMap)
{
	long mapLength = theMap.size();
	inStream << mapLength;
	map<int, string>::const_iterator i = theMap.begin();
	while(i != theMap.end()) {
		inStream << (long)i->first;
		WriteString(inStream, i->second);
		++i;
	}
}

void ReadMap(LStream& inStream, map<int, int>&theResult)
{
	long mapLength;
	inStream >> mapLength;
	
	while (mapLength > 0) {
		long first, second;
		inStream >> first;
		inStream >> second;
		theResult[first] = second;
		--mapLength;
	}
}

void ReadMap(LStream& inStream, map<int, string>&theResult)
{
	long mapLength;
	inStream >> mapLength;
	
	while (mapLength > 0) {
		long first;
		string second;
		inStream >> first;
		second = ReadString(inStream);
		theResult[first] = second;
		--mapLength;
	}
}

void WriteVector(LStream& inStream, const vector<int>& theVector)
{
	long vectorLength = theVector.size();
	inStream << vectorLength;
	vector<int>::const_iterator i = theVector.begin();
	while(i != theVector.end()) {
		inStream << (long)*i;
		++i;
	}
}

void WriteVector(LStream& inStream, const vector<string>& theVector)
{
	long vectorLength = theVector.size();
	inStream << vectorLength;
	vector<string>::const_iterator i = theVector.begin();
	while(i != theVector.end()) {
		WriteString(inStream, *i);
		++i;
	}
}

void ReadVector(LStream& inStream, vector<int>&theResult)
{
	long vectorLength;
	inStream >> vectorLength;
	
	while (vectorLength > 0) {
		long first;
		inStream >> first;
		theResult.push_back(first);
		--vectorLength;
	}
}

void ReadVector(LStream& inStream, vector<string>&theResult)
{
	long vectorLength;
	inStream >> vectorLength;
	
	while (vectorLength > 0) {
		theResult.push_back(ReadString(inStream));
		--vectorLength;
	}
}

string ReadString(LStream& inStream)
{
	long stringLength;
	inStream >> stringLength;
	StPointerBlock block(stringLength);
	inStream.ReadBlock(block, stringLength);
	return string(block, stringLength);
}
#endif

void WriteStringAsText(LStream& inStream, const string& theString)
{
	long stringLength = theString.length();
	inStream.WriteBlock(theString.data(), stringLength);
}


void
FillListBox(LListBox* theBox, const vector<string>& theStrings)
{
	ListHandle lh = theBox->GetMacListH();
	ThrowIfNil_(lh);
	int stringCount = theStrings.size();
	int colCount = ((**lh).dataBounds.right - (**lh).dataBounds.left);
	int rowCount = (stringCount + colCount - 1)/colCount;
	int delta = (**lh).dataBounds.bottom - rowCount;
	if (delta > 0) {
		::LDelRow(delta, 0, lh);
	} else if (delta < 0) {
		::LAddRow(-delta, 0, lh);
	}
	
	for (int i = 0; i < stringCount; i +=1 ) {
		Point theCell;
		theCell.h = i % colCount;
		theCell.v = i / colCount;
		::LSetCell(theStrings[i].data(), theStrings[i].length(), theCell, lh);
	}
}

int cmp_nocase(const string &s, const string& s2)
{
	string::const_iterator p = s.begin();
	string::const_iterator p2 = s2.begin();
	
	while (p != s.end() && p2 != s2.end()) {
		if (toupper(*p) != toupper(*p2))
			return (toupper(*p) < toupper(*p2)) ? -1: 1;
		++p;
		++p2;
	}
	return s2.size() - s.size();
}
#ifdef _DEBUG
#define LOG_STACK_SPACE 0
#endif

const int kMinFreeStackSpace = 4096;
void ConfirmFreeStackSpace()
{
#ifdef CONDUIT
	unsigned long stackFreeSpace;
	ThreadCurrentStackSpace(kCurrentThreadID, &stackFreeSpace);
	if (stackFreeSpace < kMinFreeStackSpace)
		Throw_(insufficientStackErr); 
#if LOG_STACK_SPACE
		DebugOutput::Output( "free stack space = " );
		DebugOutput::Output( AsString((int)stackFreeSpace).c_str());
		DebugOutput::Output( "\r" );
#endif
#endif
}

string
GetText(LTextEditView* view)
{
	Handle h = view->GetTextHandle();
	StHandleLocker locker(h);
	string s(*h, GetHandleSize(h));
	return s;
}

OSErr
ResolveAlias(AliasHandle aliasH, FSSpec& outFileSpec)
{
	Boolean wasChanged;
	short fileCount = 1;
	int rulesMask = kARMNoUI | kARMSearch;
	return ::MatchAlias(0, rulesMask, aliasH, &fileCount, &outFileSpec, &wasChanged, 0, 0);
}

bool
AliasHandlesEquivalent(AliasHandle a, AliasHandle b)
{
	FSSpec specA, specB;
	ResolveAlias(a, specA);
	ResolveAlias(a, specB);
	
	return LFile::EqualFileSpec(specA, specB);
}

short GetBootDisk()
{
	short foundVRefNum;
	long foundDirID;
	ThrowIfOSErr_(::FindFolder(kOnSystemDisk, kSystemFolderType, true, 
				&foundVRefNum, &foundDirID));
	return foundVRefNum;
}

long GetDesktopFolderOf(short vRefNum)
{
	short foundVRefNum;
	long foundDirID;
	ThrowIfOSErr_(::FindFolder(vRefNum, kDesktopFolderType, true, 
				&foundVRefNum, &foundDirID));
	return foundDirID;
}


long GetBootDiskDesktopFolder()
{
	short foundVRefNum;
	long foundDirID;
	ThrowIfOSErr_(::FindFolder(kOnSystemDisk, kDesktopFolderType, true, 
				&foundVRefNum, &foundDirID));
	return foundDirID;
}
