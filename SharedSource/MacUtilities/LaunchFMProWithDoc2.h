#pragma once

OSErr OpenSpecifiedDocumentWithFMPro(OSType creator, const FSSpec * documentFSSpecPtr, ProcessSerialNumber *psn, Boolean *wasRunning);
OSErr FindFileMaker(OSType creator, const FSSpec * documentFSSpecPtr,
	FSSpecPtr applicationFSSpecPtr);
OSErr SendOpenDocumentEventToProcess(ProcessSerialNumber *targetPSN,
			const FSSpec *theSpecArray, const short numOfSpecs);
OSErr LaunchApplicationWithDocument(const FSSpec *applicationFSSpecPtr,
			const FSSpec *theSpecArray, const short numOfSpecs, ProcessSerialNumber*  psn);

