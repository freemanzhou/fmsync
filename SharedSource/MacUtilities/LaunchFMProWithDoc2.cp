/*
 *   LaunchWithDoc v 2.1
 *
 *   Greg Robbins  August 1993, 
 *   modified by Nitin Ganatra, July 1994
 *
 *   Document-launching sample program
 *   Loosely based on C.K. Haun's LaunchWithDoc
 *
 *   This snippet includes these useful routines:
 *
 *      OpenSpecifiedDocument
 *        finds the creator application for a document, whether or not
 *        the app is running, launches the app if necessary, and sends
 *        the Apple Event necessary to get the app to open the document
 *
 *      FindApplicationFromDocument
 *        searches the mounted volumes for the application which
 *        created a document
 *
 *      LaunchApplicationWithDocument
 *        launches an application which is not running and passes it
 *        the OpenDocuments event as part of the launch parameters
 *
 *      SendOpenDocumentEventToProcess
 *        sends an OpenDocuments Apple event to a running program
 *
 *      BuildOpenDocumentsEvent
 *        utility function to build an 'odoc' event from a list of 
 *        FSSpecs.
 *
 *  Remember that a target application need not be Apple event aware
 *  in order for the OpenDocuments event to succeed (the System will
 *  pull "puppet strings", simulating the events necessary to make the
 *  target app open the document)
 *
 *  However, LaunchWithDoc must be high level event aware (as set in the
 *  SIZE resource) in order to send Apple events using AESend
 *
 */

#include <QuickDraw.h>
#include <StandardFile.h>
#include <Fonts.h>
#include <Menus.h>
#include <Dialogs.h>
#include <Events.h>
#include <Files.h>
#include <TextEdit.h>
#include <Memory.h>
#include <Errors.h>
#include <Processes.h>
#include <AppleEvents.h>
#include <Aliases.h>

#include "LaunchFMProWithDoc2.h"
#include "StAppleEvent.h"
#include "Utilities.h"

// prototypes

void ReportError(StringPtr messageStr);

#define true 1
#define false 0

// what functions changed since LaunchWithDoc2

OSErr BuildOpenDocumentEvent(ProcessSerialNumber *targetPSN, 
			const FSSpec *theSpecArray, const short numOfSpecs, AppleEvent *odocAppleEvent);
OSErr LaunchApplicationWithDocument(const FSSpec *applicationFSSpecPtr,
			const FSSpec *theSpecArray, const short numOfSpecs, ProcessSerialNumber*  psn);


void ReportError(StringPtr messageStr)
{
	DebugStr(messageStr);
}


// OpenSpecifiedDocument searches to see if the application which
// created the document is already running.  If so, it sends
// an OpenSpecifiedDocuments Apple event to the target application
// (remember that, because of puppet strings, this works even
// if the target application is not Apple event-aware.)

OSErr OpenSpecifiedDocumentWithFMPro(OSType creator, const FSSpec * documentFSSpecPtr, ProcessSerialNumber *psn, Boolean *wasRunning)
{
	OSErr retCode;
	ProcessSerialNumber currPSN;
	ProcessInfoRec currProcessInfo;
	FSSpec applicationSpec;
	FInfo documentFInfo;
	Boolean foundRunningProcessFlag;
	
	// verify the document file exists and get its creator type
	
	retCode = FSpGetFInfo(documentFSSpecPtr, &documentFInfo);
	if (retCode != noErr) goto Bail;
	
	// check the current processes to see if the creator app is already
	// running, and get its process serial number (as currPSN)
	
	currPSN.lowLongOfPSN = kNoProcess;
	currPSN.highLongOfPSN = 0;
	
	currProcessInfo.processInfoLength = sizeof(ProcessInfoRec);
	currProcessInfo.processName = nil;
	currProcessInfo.processAppSpec = &applicationSpec;
	
	foundRunningProcessFlag = false;
	while (GetNextProcess(&currPSN) == noErr) {
		if (GetProcessInformation(&currPSN, &currProcessInfo) == noErr) {
			if (currProcessInfo.processSignature == creator) {
				foundRunningProcessFlag = true;
				break;
			}
		}
	}
	
	// if the creator is running, send it an OpenDocuments Apple event
	// since there is no need to launch it
	
	if (foundRunningProcessFlag) {
		retCode = SendOpenDocumentEventToProcess(&currPSN, documentFSSpecPtr, 1);
		if (psn)
			*psn = currPSN;
	}

	
	// else if the creator is not running, find it on disk and launch
	// it with the OpenDocuments event included as a part of the
	// launch parameters
	
	else {
		retCode = FindFileMaker(creator, documentFSSpecPtr, &applicationSpec);
		
		if (retCode == noErr) {
			retCode = LaunchApplicationWithDocument(&applicationSpec, 0, 0, psn);
			if (retCode == noErr) {
				retCode = SendOpenDocumentEventToProcess(psn, documentFSSpecPtr, 1);
			}
		}
	}
	*wasRunning = foundRunningProcessFlag;
Bail:
	return retCode;
}


//----------------------------------------------------------------------------
// LaunchApplicationWithDocument
//
// given an application and any number of documents, 
// LaunchApplicationWithDocument launches the application and passes the 
// application an OpenDocuments event for the document(s)
//----------------------------------------------------------------------------
OSErr LaunchApplicationWithDocument(
	const FSSpec		*applicationFSSpecPtr,
	const FSSpec 		*theSpecArray,
	const short			numOfSpecs,
	ProcessSerialNumber*  psn)
{
	OSErr retCode;
	LaunchParamBlockRec launchParams;
	AppleEvent theAppleEvent;
	AEDesc launchParamDesc;
	ProcessSerialNumber targetPSN;
	
	// to simplify cleanup, ensure that handles are nil to start
	launchParams.launchAppParameters	= nil;
	theAppleEvent.dataHandle			= nil;
	launchParamDesc.dataHandle			= nil;
	
	if (theSpecArray != nil) {

		// because 'odoc' events require a address descriptor, I just 
		// grab the PSN for the current process.  It doesn't matter what
		// it is, because it's never used.
		(void) GetCurrentProcess(&targetPSN);
		
		// build an 'odoc' event given the array of FSSpecs and the ProcessSerialNumber
		retCode = BuildOpenDocumentEvent(&targetPSN, theSpecArray, numOfSpecs, &theAppleEvent);
		
		if (retCode == noErr) {
		
			// coerce the AppleEvent into app parameters, for _LaunchApplication
			retCode = AECoerceDesc(&theAppleEvent, typeAppParameters, &launchParamDesc);
			if (retCode != noErr) goto Bail;
			
			// fill in the launch parameter block, including the
			// Apple event, and make the launch call
			HLock((Handle) launchParamDesc.dataHandle);
			launchParams.launchAppParameters =
				(AppParametersPtr) *(launchParamDesc.dataHandle);

		}

	}

	launchParams.launchBlockID		= extendedBlock;
	launchParams.launchEPBLength	= extendedBlockLen;
	launchParams.launchFileFlags	= 0;
	launchParams.launchControlFlags	= launchContinue | launchNoFileFlags | launchDontSwitch;
	//launchParams.launchControlFlags	= launchContinue | launchNoFileFlags;
	launchParams.launchAppSpec		= (FSSpecPtr)applicationFSSpecPtr;

	retCode = LaunchApplication(&launchParams);
	if (psn)
		*psn = launchParams.launchProcessSN;
	
Bail:
	// dispose of everything that was allocated
	if (theAppleEvent.dataHandle != nil)     (void) AEDisposeDesc(&theAppleEvent);
	if (launchParamDesc.dataHandle != nil)   (void) AEDisposeDesc(&launchParamDesc);
	
	return retCode;

}


//----------------------------------------------------------------------------
// SendOpenDocumentEventToProcess
//
// given an application's serial number and any number of documents, 
// SendOpenDocumentEventToProcess passes 
// the application an OpenDocuments event for the document
//----------------------------------------------------------------------------
OSErr SendOpenDocumentEventToProcess(
	ProcessSerialNumber			*targetPSN,
	const FSSpec 				*theSpecArray,
	const short					numOfSpecs)
{
	OSErr retCode;
	StAppleEvent theAppleEvent, theReplyEvent;

	retCode = BuildOpenDocumentEvent(targetPSN, theSpecArray, numOfSpecs, theAppleEvent);

	if (retCode == noErr) {
		try {
			theAppleEvent.Send(theReplyEvent, kAEWaitReply);
		} catch (const LException& inErr) {
			retCode = inErr.GetErrorCode();
		}
	}
	
	return retCode;

}


// FindApplicationFromDocument uses the Desktop Database to
// locate the creator application for the given document
//
// this routine will first check the desktop database of the disk
// containing the document, then the desktop database of all local
// disks, then the desktop databases of all server volumes
// (so up to three passes will be made)

OSErr FindFileMaker(OSType creator, const FSSpec * documentFSSpecPtr,
	FSSpecPtr applicationFSSpecPtr)
{
	enum { documentPass, localPass, remotePass, donePass } ;
	int volumePass;
	DTPBRec desktopParams;
	HParamBlockRec hfsParams;
	FInfo documentFInfo;
	short volumeIndex;
	Boolean foundFlag;
	GetVolParmsInfoBuffer volumeInfoBuffer;
	OSErr retCode;
	
// dkj 12/94 initialize flag to false (thanks to Peter Baral for pointing out this bug)
	foundFlag = false;

	// verify the document file exists and get its creator type
	
	retCode = FSpGetFInfo(documentFSSpecPtr, &documentFInfo);
	if (retCode != noErr) goto Bail;
	
	volumePass = documentPass;
	volumeIndex = 0;
	
	do {
		
		// first, find the vRefNum of the volume whose Desktop Database
		// we're checking this time
		
		// if we're on the initial pass (documentPass) just use
		// the vRefNum of the document itself
		
		if (volumePass == documentPass)
		
			desktopParams.ioVRefNum = documentFSSpecPtr->vRefNum;
		
		// otherwise, find the vRefNum of the next volume appropriate
		// for this pass
		
		else {
			
			volumeIndex++;
			
			// convert the volumeIndex into a vRefNum
			
			hfsParams.volumeParam.ioNamePtr = nil;
			hfsParams.volumeParam.ioVRefNum = 0;
			hfsParams.volumeParam.ioVolIndex = volumeIndex;
			retCode = PBHGetVInfoSync(&hfsParams);
			
			// a nsvErr indicates that the current pass is over
			if (retCode == nsvErr) goto SkipThisVolume;
			if (retCode != noErr) goto Bail;
			
			// since we handled the document volume during the documentPass,
			// skip it if we have hit that volume again
			
			if (hfsParams.volumeParam.ioVRefNum == documentFSSpecPtr->vRefNum)
				goto SkipThisVolume;
			
			// call GetVolParms to determine if this volume is a server
			// (a remote volume)
			
			hfsParams.ioParam.ioBuffer = (Ptr) &volumeInfoBuffer;
			hfsParams.ioParam.ioReqCount = sizeof(GetVolParmsInfoBuffer);
			retCode = PBHGetVolParmsSync(&hfsParams);
			if (retCode != noErr) goto Bail;
			
			// if the vMServerAdr field of the volume information buffer
			// is zero, this is a local volume; skip this volume
			// if it's local on a remote pass or remote on a local pass
			
			if ((volumeInfoBuffer.vMServerAdr != 0) !=
				(volumePass == remotePass)) goto SkipThisVolume;
			
			// okay, now we've found the vRefNum for our desktop database call
			
			desktopParams.ioVRefNum = hfsParams.volumeParam.ioVRefNum;
		}
		
		// find the path refNum for the desktop database for
		// the volume we're interested in
		
		desktopParams.ioNamePtr = nil;
		
		retCode = PBDTGetPath(&desktopParams);
		if (retCode == noErr && desktopParams.ioDTRefNum != 0) {
		
			// use the GetAPPL call to find the preferred application
			// for opening any document with this one's creator
			
			desktopParams.ioIndex = 0;
			desktopParams.ioFileCreator = creator;
			desktopParams.ioNamePtr = applicationFSSpecPtr->name;
			retCode = PBDTGetAPPLSync(&desktopParams);
			
			if (retCode == noErr) {
			
				// okay, found it; fill in the application file spec
				// and set the flag indicating we're done
				
				applicationFSSpecPtr->parID = desktopParams.ioAPPLParID;
				applicationFSSpecPtr->vRefNum = desktopParams.ioVRefNum;
				foundFlag = true;
				
			}
		}
		
	SkipThisVolume:
	
		// if retCode indicates a no such volume error or if this
		// was the first pass, it's time to move on to the next pass
		
		if (retCode == nsvErr || volumePass == documentPass) {
			volumePass++;
			volumeIndex = 0;
		}
		
	} while (foundFlag == false && volumePass != donePass);
	
Bail:
	return retCode;
}


//----------------------------------------------------------------------------
// BuildOpenDocumentsEvent
//
// General utility to turn a ProcessSerialNumber and a list of FSSpecs into
// an 'odoc' AppleEvent with the ProcessSerialNumber as the target
// application.  Used by SendOpenDocumentEventToProcess, and
// LaunchApplicationWithDocument.
//----------------------------------------------------------------------------
OSErr BuildOpenDocumentEvent(
	ProcessSerialNumber		*targetPSN, 
	const FSSpec 			*theSpecArray, 
	const short				numOfSpecs,
	AppleEvent				*odocAppleEvent)
{
	OSErr			retCode;
	AppleEvent		theAppleEvent;
	AEDesc			targetAddrDesc, docDesc;
	AEDescList		docDescList;
	AliasHandle		docAlias;
	short			counter;
	FSSpecPtr		specIterator;

	// to simplify cleanup, ensure that handles are nil to start
	targetAddrDesc.dataHandle	= nil;
	theAppleEvent.dataHandle	= nil;
	docDescList.dataHandle		= nil;
	docDesc.dataHandle			= nil;
	docAlias					= nil;

	// create an address descriptor based on the serial number of
	// the target process
	retCode = AECreateDesc(typeProcessSerialNumber, (Ptr) targetPSN,
		sizeof(ProcessSerialNumber), &targetAddrDesc);
	if (retCode != noErr) goto Bail;
	
	// make a descriptor list containing just a descriptor with an
	// alias to the document
	retCode = AECreateList(nil, 0, false, &docDescList);
	if (retCode != noErr) goto Bail;

	// start at the beginning of the FSSpec list, and start adding
	// them to the document list descriptor

	// NOTE: we need to make sure we dispose of the aliases and the
	// AE descriptor in this loop, otherwise there will be memory
	// leaks.
	specIterator = (FSSpecPtr)theSpecArray;
	for (counter = 0; counter < numOfSpecs; counter ++) {	

		retCode = NewAlias(nil, &specIterator[counter], &docAlias);
		if (retCode != noErr) goto Bail;
		
		HLock((Handle) docAlias);
		retCode = AECreateDesc(typeAlias, (Ptr) *docAlias, 
			GetHandleSize((Handle) docAlias), &docDesc);
		HUnlock((Handle) docAlias);
		
		if (retCode != noErr) goto Bail;
		// the alias is now in the AEDescriptor, so dispose of it
		DisposeHandle((Handle)docAlias); docAlias = nil;
		
		retCode = AEPutDesc(&docDescList, 0, &docDesc);
		if (retCode != noErr) goto Bail;

		// the alias is now in the AE document list, so dispose of it
		retCode = AEDisposeDesc(&docDesc);
		if (retCode != noErr) goto Bail;

	}
	
	// now make the 'odoc' AppleEvent descriptor and insert the 
	// document descriptor list as the direct object
	retCode = AECreateAppleEvent(kCoreEventClass, kAEOpenDocuments,
		&targetAddrDesc, kAutoGenerateReturnID, kAnyTransactionID,
		&theAppleEvent);
	if (retCode != noErr) goto Bail;
	
	retCode = AEPutParamDesc(&theAppleEvent, keyDirectObject, &docDescList);
	if (retCode != noErr) goto Bail;
	
	*odocAppleEvent = theAppleEvent;

Bail:
	// dispose of everything that was allocated, except the return AE
	if (targetAddrDesc.dataHandle != nil)  (void) AEDisposeDesc(&targetAddrDesc);
	if (docDescList.dataHandle != nil)  (void) AEDisposeDesc(&docDescList);
	if (docDesc.dataHandle != nil)  (void) AEDisposeDesc(&docDesc);
	if (docAlias != nil)  DisposeHandle((Handle) docAlias);
	
	return retCode;

}
