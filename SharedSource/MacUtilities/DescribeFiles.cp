#include "MoreFiles.h"
#include "MoreFilesExtras.h"
#include "DescribeFiles.h"
#include "Stringiness.h"
#include "Str255.h"
#include "FolderUtilities.h"

const ResIDT kFolderStrings = 8401;
const ResIDT kFileStrings = 8402;
enum { kFolderOnDiskIndex = 1, kDesktopFolderIndex, kRootFolderIndex};
enum {kFolderString = 4, kFileString};

string
DescribeFolder(const FSSpec& thisFolder)
{
	string folderIs;
	Str27 volName;
	short vRefNum;
	unsigned long freeBytes, totalBytes;
	ThrowIfOSErr_(HGetVInfo(thisFolder.vRefNum, volName, &vRefNum, &freeBytes, &totalBytes));
	long dirID = 0;
	Boolean isDir = false;
	OSErr err = FSpGetDirectoryID(&thisFolder, &dirID, &isDir);
	if (err == noErr)
		ThrowIfNot_(isDir);
	if (err == noErr && thisFolder.parID == fsRtParID) {
		folderIs = LoadString(kFolderStrings, kRootFolderIndex);
		Substitute(folderIs, "%%2", AsString(volName));
	} else {
		long foundDirID;
		short foundVRefNum;
		ThrowIfOSErr_(FindFolder(vRefNum, kDesktopFolderType, false, &foundVRefNum, &foundDirID));
		if (foundDirID == dirID) {
			folderIs = LoadString(kFolderStrings, kDesktopFolderIndex);
			Substitute(folderIs, "%%2", AsString(volName));
		} else {
			FSSpec thisFolderWithName(thisFolder);
			if (err == noErr)
				Folders::Normalize(thisFolder, &thisFolderWithName);
			folderIs = LoadString(kFolderStrings, kFolderOnDiskIndex);
			Substitute(folderIs, "%%1", AsString(thisFolderWithName.name));
			Substitute(folderIs, "%%2", AsString(volName));
		}
	}
	return folderIs;
}

string
DescribeFile(const FSSpec& thisFile, bool willBeFolder)
{
	FSSpec thisFolder;
	string folderIs;
	Str27 volName;
	short vRefNum;
	unsigned long freeBytes, totalBytes;
	ThrowIfOSErr_(HGetVInfo(thisFile.vRefNum, volName, &vRefNum, &freeBytes, &totalBytes));
	long dirID;
	Boolean isDir;
	ThrowIfOSErr_(FSMakeFSSpec(thisFile.vRefNum, thisFile.parID, 0, &thisFolder));
	ThrowIfOSErr_(FSpGetDirectoryID(&thisFolder, &dirID, &isDir));
	ThrowIfNot_(isDir);
	if (thisFolder.parID == fsRtParID) {
		folderIs = LoadString(kFileStrings, kRootFolderIndex);
		Substitute(folderIs, "%%3", AsString(volName));
	} else {
		long foundDirID;
		short foundVRefNum;
		ThrowIfOSErr_(FindFolder(vRefNum, kDesktopFolderType, false, &foundVRefNum, &foundDirID));
		if (foundDirID == dirID) {
			folderIs = LoadString(kFileStrings, kDesktopFolderIndex);
			Substitute(folderIs, "%%3", AsString(volName));
		} else {
			folderIs = LoadString(kFileStrings, kFolderOnDiskIndex);
			Substitute(folderIs, "%%2", AsString(thisFolder.name));
			Substitute(folderIs, "%%3", AsString(volName));
		}
	}
	Substitute(folderIs, "%%1", AsString(thisFile.name));
	if (willBeFolder) {
		Substitute(folderIs, "%%0", LoadString(kFileStrings, kFolderString));
	} else {
		Substitute(folderIs, "%%0", LoadString(kFileStrings, kFileString));
	}
	return folderIs;
}
