#include <Math64.h>
#include <LString.h>

#include "IterateDirectory.h"
#include "FolderUtilities.h"
#include "FullPath.h"
#include "FileCopy.h"
#include "MoreDesktopMgr.h"
#include "MoreFilesExtras.h"
#include "Stringiness.h"
#include "Str255.h"

namespace Folders {

class FolderIterator {
public:
	FolderIterator(OSType fileType, const std::string& extension);
	
	static pascal void Wrapper(const CInfoPBRec * const cpbPtr, Boolean *quitFlag, void *yourDataPtr);
	
	bool HandleFile(const CInfoPBRec * const cpbPtr);
	
	Str255 fName;
	OSType		fType;
	std::vector<FSSpec> fContents;
};

FolderIterator::FolderIterator(OSType fileType, const std::string& extension)
	: fType(fileType)
{
	LString::CopyPStr(AsStr255(extension), fName);
}

pascal void FolderIterator::Wrapper(const CInfoPBRec * const cpbPtr, Boolean *quitFlag, void *yourDataPtr)
{
	ThrowIfNil_(yourDataPtr);
	*quitFlag = reinterpret_cast<FolderIterator*>(yourDataPtr)->HandleFile(cpbPtr);
}

bool FolderIterator::HandleFile(const CInfoPBRec * const cpbPtr)
{
	if ( (cpbPtr->hFileInfo.ioFlAttrib & ioDirMask) != 0 )
		return false;
		
	if (fType && fType != cpbPtr->hFileInfo.ioFlFndrInfo.fdType)
		return false;
		
	if (fName[0]) {
		LStr255 nameStr(cpbPtr->hFileInfo.ioNamePtr);
		nameStr.SetCompareFunc(LString::CompareIgnoringCase);
		if (!nameStr.EndsWith(fName))
			return false;
	}
	
	FSSpec spec;
	FSMakeFSSpec(cpbPtr->hFileInfo.ioVRefNum, cpbPtr->hFileInfo.ioFlParID, cpbPtr->hFileInfo.ioNamePtr, &spec);
	fContents.push_back(spec);
	return false;
}

std::vector<FSSpec> GetFolderContents(const FSSpec& target, OSType fileType, const std::string& extension, int maxLevels)
{
	FolderIterator i(fileType, extension);
	FSpIterateDirectory(&target, maxLevels, FolderIterator::Wrapper, &i);
	return i.fContents;
}

long FindOrCreateFolder(short vRefNum, long parID, ConstStringPtr folderName)
{
	FSSpec spec;
	FSMakeFSSpec(vRefNum, parID, folderName, &spec);
	long theID;
	Boolean isDir;
	OSErr err = FSpGetDirectoryID(&spec, &theID, &isDir);
	if (err == fnfErr) {
		ThrowIfOSErr_(FSpDirCreate(&spec, 0, &theID));
	} else {
		if (!isDir)
			Throw_(dupFNErr);
	}
	return theID;
}

std::string FullPathAsString(const FSSpec& f)
{
	Handle h;
	short handleLength;
	FSpGetFullPath(&f, &handleLength, &h);
	StHandleBlock hb(h);
	return std::string(*h, handleLength);
}

FSSpec MakeFSSpec(const std::string& fName, short vr, long parID, bool mustExist)
{
	FSSpec spec;
	OSErr err = FSMakeFSSpec(vr, parID, AsStr255(fName), &spec);
	if (mustExist)
		ThrowIfOSErr_(err);
	return spec;
}


OSType GetFileType(const FSSpec& inSpec)
{
	HParamBlockRec pb;
	
	pb.fileParam.ioNamePtr = (StringPtr)inSpec.name;
	pb.fileParam.ioVRefNum = inSpec.vRefNum;
	pb.fileParam.ioFVersNum = 0;
	pb.fileParam.ioDirID = inSpec.parID;
	pb.fileParam.ioFDirIndex = 0;
	ThrowIfOSErr_(PBHGetFInfoSync(&pb));
	return ( pb.fileParam.ioFlFndrInfo.fdType );
}

OSType GetFileCreator(const FSSpec& inSpec)
{
	HParamBlockRec pb;
	
	pb.fileParam.ioNamePtr = (StringPtr)inSpec.name;
	pb.fileParam.ioVRefNum = inSpec.vRefNum;
	pb.fileParam.ioFVersNum = 0;
	pb.fileParam.ioDirID = inSpec.parID;
	pb.fileParam.ioFDirIndex = 0;
	ThrowIfOSErr_(PBHGetFInfoSync(&pb));
	return ( pb.fileParam.ioFlFndrInfo.fdCreator );
}

unsigned long GetFileModificationDate(const FSSpec& inSpec)
{
	HParamBlockRec pb;
	
	pb.fileParam.ioNamePtr = (StringPtr)inSpec.name;
	pb.fileParam.ioVRefNum = inSpec.vRefNum;
	pb.fileParam.ioFVersNum = 0;
	pb.fileParam.ioDirID = inSpec.parID;
	pb.fileParam.ioFDirIndex = 0;
	if (PBHGetFInfoSync(&pb) != noErr)
		return 1;
	return ( pb.fileParam.ioFlMdDat );
}

void SetFileModificationDate(const FSSpec& inSpec, unsigned long d)
{
	HParamBlockRec pb;
	
	pb.fileParam.ioNamePtr = (StringPtr)inSpec.name;
	pb.fileParam.ioVRefNum = inSpec.vRefNum;
	pb.fileParam.ioFVersNum = 0;
	pb.fileParam.ioDirID = inSpec.parID;
	pb.fileParam.ioFDirIndex = 0;
	ThrowIfOSErr_(PBHGetFInfoSync(&pb));

	pb.fileParam.ioFlMdDat = d;
	pb.fileParam.ioNamePtr = (StringPtr)inSpec.name;
	pb.fileParam.ioVRefNum = inSpec.vRefNum;
	pb.fileParam.ioFVersNum = 0;
	pb.fileParam.ioDirID = inSpec.parID;
	pb.fileParam.ioFDirIndex = 0;
	ThrowIfOSErr_(PBHSetFInfoSync(&pb));
}

AliasHandle AliasFromFSSpec(const FSSpec& f)
{
	AliasHandle alias;
	ThrowIfOSErr_(::NewAlias(0, &f, &alias));
	return alias;
}

std::string NameFromAlias(AliasHandle alias)
{
	Str255 theName;
	ThrowIfOSErr_(GetAliasInfo(alias, asiAliasName, theName));
	return AsString(theName);
}

FSSpec FSSpecFromAlias(AliasHandle h)
{
	FSSpec f;
	Boolean wasChanged;
	ThrowIfOSErr_(FSSpecFromAlias(h, f, wasChanged));
	return f;
}

OSErr FSSpecFromAlias(AliasHandle h, FSSpec& f, Boolean& wasChanged)
{
	short fileCount = 1;
	int rulesMask = kARMNoUI | kARMSearch;
	return ::MatchAlias(0, rulesMask, h, &fileCount, &f, &wasChanged, 0, 0);
}

bool FileExists(const FSSpec& f)
{
	FSSpec s;
	return FSMakeFSSpec(f.vRefNum, f.parID, f.name, &s) == noErr;
}

bool IsDirectory(const FSSpec& f)
{
	long parID;
	Boolean isDirectory;
	OSErr err = FSpGetDirectoryID(&f, &parID, &isDirectory);
	return (err == noErr) && isDirectory;
}

void MakeDirectory(const FSSpec& inFileSpec)
{
	long createdDirID;
	ThrowIfOSErr_(FSpDirCreate(&inFileSpec, 0, &createdDirID));
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

short GetPreferencesFolderVolume()
{
	short v;
	long p;
	ThrowIfOSErr_(::FindFolder(kOnSystemDisk, kPreferencesFolderType, true, 
				&v, &p));
	return v;
}

long GetPreferencesFolderID()
{
	short v;
	long p;
	ThrowIfOSErr_(::FindFolder(kOnSystemDisk, kPreferencesFolderType, true, 
				&v, &p));
	return p;
}

void Normalize(const FSSpec& inFileSpec, FSSpec* outSpec)
{
	ThrowIfOSErr_(::FSMakeFSSpec(inFileSpec.vRefNum, inFileSpec.parID, 
			inFileSpec.name, outSpec));
}

bool IsStationery(const FSSpec& inSpec)
{
	HParamBlockRec pb;
	
	pb.fileParam.ioNamePtr = (StringPtr)inSpec.name;
	pb.fileParam.ioVRefNum = inSpec.vRefNum;
	pb.fileParam.ioFVersNum = 0;
	pb.fileParam.ioDirID = inSpec.parID;
	pb.fileParam.ioFDirIndex = 0;
	ThrowIfOSErr_(PBHGetFInfoSync(&pb));
	return ( pb.fileParam.ioFlFndrInfo.fdFlags & kIsStationery);
}

bool IsFolder(const FSSpec& f)
{
	long theID;
	Boolean isDir;
	OSErr err = FSpGetDirectoryID(&f, &theID, &isDir);
	return (err != noErr) && isDir;
}

FSSpec GetHelpFile(const string& folderName, const string& fileName)
{
	short v;
	long p;
	ThrowIfOSErr_(::FindFolder(kOnSystemDisk, kHelpFolderType, true, 
				&v, &p));
	
	return MakeFSSpec(":" + folderName + ":" + fileName, v, p);
}

void CopyFile(const FSSpec& s, const string& newName)
{
	FSSpec dirSpec(MakeFSSpec(""));
	ThrowIfOSErr_(FSpFileCopy(&s, &dirSpec, AsStr255(newName), 0, 0, 0));
}

std::string VolumeSpaceFree(short vRefNum)
{
	std::string s("Disk space check failed.");
	UInt64 freeBytes, totalBytes;
	LStr255 vName;
	short v;
	if (XGetVInfo(vRefNum, vName, &v, &freeBytes, &totalBytes) == noErr) {
		s = LottaBytesAsString(UnsignedWideToUInt64(freeBytes));
		s += " free on ";
		s += AsString(vName);
		s += " (";
		s += LottaBytesAsString(UnsignedWideToUInt64(totalBytes));
		s += ")";
	}
	return s;
}

std::string VolumeName(short vRefNum)
{
	UInt64 freeBytes, totalBytes;
	LStr255 vName;
	short v;
	std::string s("Volume does not exist.");
	if (XGetVInfo(vRefNum, vName, &v, &freeBytes, &totalBytes) == noErr) {
		s = AsString(vName);
	}
	return s;
}

bool FindApplication(OSType creator, FSSpec& fs)
{
	for (int vRefNum = -1; vRefNum > -32; vRefNum -= 1) {
		OSErr err = FSpDTXGetAPPL(0, vRefNum, creator, false, &fs);
		if (err == afpItemNotFound)
			continue;

		if (err != noErr)
			return false;
			
		return true;
	}
	return false;
}

}

