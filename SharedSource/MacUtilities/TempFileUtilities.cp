#include "TempFileUtilities.h"

void GetTemporaryFileOnVolume(short vRefNum, FSSpec *outSpec)
{
	short foundVRefNum;
	long foundDirID;
	OSErr err = ::FindFolder(vRefNum, kChewableItemsFolderType, true, 
				&foundVRefNum, &foundDirID);
	if (err != noErr)
		ThrowIfOSErr_(::FindFolder(vRefNum, kTemporaryFolderType, true, 
					&foundVRefNum, &foundDirID));
				
	long fileName = Random();
	do {
		LStr255 tempName("\pixt-");
		LStr255 number(fileName);
		tempName.Append(number);
		fileName += 1;
		err = ::FSMakeFSSpec(foundVRefNum, foundDirID, tempName, outSpec);
	} while (err == noErr);
	if (err != fnfErr)
		Throw_(err);
}

void GetTemporaryFile(FSSpec *outSpec)
{
	GetTemporaryFileOnVolume(kOnSystemDisk, outSpec);
}

void
GetTemporaryFileNear(const FSSpec& inSpec, FSSpec *outSpec)
{
	GetTemporaryFileOnVolume(inSpec.vRefNum, outSpec);
}

bool
GetTempFileNamed(ConstStringPtr fileName, FSSpec *outSpec)
{
	short foundVRefNum;
	long foundDirID;
	OSErr err = ::FindFolder(kOnSystemDisk, kChewableItemsFolderType, true, 
				&foundVRefNum, &foundDirID);
	if (err != noErr)
		ThrowIfOSErr_(::FindFolder(kOnSystemDisk, kTemporaryFolderType, true, 
					&foundVRefNum, &foundDirID));
	err = ::FSMakeFSSpec(foundVRefNum, foundDirID, fileName, outSpec);
	if (err != noErr && err != fnfErr)
		Throw_(err);
	return err == noErr;
}

void SwapFiles(const FSSpec& inSpecA, const FSSpec& inSpecB)
{
	ThrowIfOSErr_(FSpExchangeFiles(&inSpecA, &inSpecB));
}