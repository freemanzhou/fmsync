#pragma once

#include <string>
#include <vector>

#include "BinaryFormat.h"


namespace Folders {

inline
Boolean operator==(
				const FSSpec&	inLhs,
				const FSSpec&	inRhs)
		{
			return LFile::EqualFileSpec(inLhs, inRhs);
		}

inline
Boolean operator!=(
				const FSSpec&	inLhs,
				const FSSpec&	inRhs)
		{
			return !LFile::EqualFileSpec(inLhs, inRhs);
		}

inline
Boolean operator<(
				const FSSpec&	inLhs,
				const FSSpec&	inRhs)
		{
			LStr255 inLhsStr(inLhs.name);
			inLhsStr.SetCompareFunc(LString::CompareIgnoringCase);
			int v = inLhsStr.CompareTo(inRhs.name);
			if (v != 0)
				return v <0;
			if (inLhs.parID != inRhs.parID)
				return inLhs.parID < inRhs.parID;
			return inLhs.vRefNum < inRhs.vRefNum;
		}


std::vector<FSSpec> GetFolderContents(const FSSpec& target, OSType fileType = 0, const std::string& extension = "", int maxLevels = 1);
std::string FullPathAsString(const FSSpec& f);
long FindOrCreateFolder(short vRefNum, long parID, ConstStringPtr folderName);
OSType GetFileType(const FSSpec& inSpec);
OSType GetFileCreator(const FSSpec& inSpec);

FSSpec MakeFSSpec(const std::string& fName = "", short vr = 0, long parID = 0, bool mustExist = false);
FSSpec FSSpecFromAlias(AliasHandle h);
std::string NameFromAlias(AliasHandle alias);
OSErr FSSpecFromAlias(AliasHandle h, FSSpec& f, Boolean& wasChanged);
AliasHandle AliasFromFSSpec(const FSSpec&);
bool FileExists(const FSSpec& f);
bool IsFolder(const FSSpec& f);
void CopyFile(const FSSpec& s, const string& newName);
unsigned long GetFileModificationDate(const FSSpec& f);
void SetFileModificationDate(const FSSpec& f, unsigned long);
FSSpec GetPreferencesFolder();
FSSpec GetHelpFolder();
FSSpec GetHelpFile(const string& folderName, const string& fileName);
short GetBootDisk();
long GetDesktopFolderOf(short);
long GetBootDiskDesktopFolder();
short GetPreferencesFolderVolume();
long GetPreferencesFolderID();
void Normalize(const FSSpec& inFileSpec, FSSpec* outSpec);
bool IsDirectory(const FSSpec& inFileSpec);
void MakeDirectory(const FSSpec& inFileSpec);
bool IsStationery(const FSSpec& inFileSpec);
std::string VolumeSpaceFree(short vRefNum);
std::string VolumeName(short vRefNum);
bool FindApplication(OSType creator, FSSpec& fs);
}

namespace BinaryFormat {

template <>
inline void Write(LStream& s, const FSSpec& f)
{
	AliasHandle h = Folders::AliasFromFSSpec(f);
	Write(s, h);
	DisposeHandle((Handle)h);
}

template <>
inline void Read(LStream& s, FSSpec& f)
{
	AliasHandle h(0);
	Read(s, h);
	StHandleBlock b((Handle)h);
	f = Folders::FSSpecFromAlias(h);
}

}