#include "ErrorCodes.h"
#include "SaveToFile.h"
#include "FolderUtilities.h"
#include "TempFileUtilities.h"

void WriteStringAsNewFile(const FSSpec& targetFile, OSType creator, OSType fileType, const std::string& contents)
{
	LFileStream aFile(targetFile);
	aFile.CreateNewDataFile(creator, fileType);
	aFile.OpenDataFork(fsRdWrPerm);
	aFile.WriteBlock(contents.data(), contents.length());
}

void WriteStringAsFile(const FSSpec& targetFile, OSType creator, OSType fileType, const std::string& contents)
{
	FSSpec dest;
	OSErr err = ::FSMakeFSSpec(targetFile.vRefNum, targetFile.parID, targetFile.name,
					&dest);

	if (err != noErr && err != fnfErr)
		Throw_(err);

	if (err == fnfErr) {
		WriteStringAsNewFile(dest, creator, fileType, contents);
		return;
	}
	
	if (Folders::IsFolder(dest)) {
		Throw_(kCantOverwriteFolderError);
	}

	GetTemporaryFileNear(targetFile, &dest);
	WriteStringAsNewFile(dest, creator, fileType, contents);
	ThrowIfOSErr_(FSpExchangeFiles(&targetFile, &dest));
	FSpDelete(&dest);
}

