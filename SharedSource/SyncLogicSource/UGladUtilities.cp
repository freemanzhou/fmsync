
/*
  File:		UGladUtilities.cp

  Contains:	xxx put contents here xxx

  Written by:	Chris LeCroy, Jeff Pritchard

  Copyright:	© 1997 by Apple Computer, Inc., all rights reserved.

  Change History (most recent first):

		 <3>	  9/5/97	csl		Ran it through CDent
  <2>	 8/22/97	csl		

  To Do:
*/

// UGladUtilities.cp
// Created by It doesn't have a name on Sun, Jan 15, 1995 @ 2:52 PM.

#include <PP_Prefix.h>


#ifndef __UGladUtilities__
#include "UGladUtilities.h"
#endif

#include <Palettes.h>
#include <TextEdit.h>
#include <Resources.h>
#include <LowMem.h>
#include <Errors.h>
#include <Files.h>
#include <Finder.h>
#include <Folders.h>
#include <Icons.h>
#include <Dialogs.h>
#include <Timer.h>

#include <PLStringFuncs.h>
#include <UDrawingState.h>
#include <UAppleEventsMgr.h>
#include <UTextTraits.h>
#include "StOpenFile.h"

//#include "CGADialogBox.h"


#include "MoreFiles.h"
#include "MoreFilesExtras.h"
#include "Search.h"


TEHandle gMATextBoxTE = NULL;
WordBreakUPP gTEDefaultWordBreak = NULL;
static QDRectUPP gStdNoRectProc = NULL;
Handle pSaveHText;
Handle pMATextBoxHText = NULL;


static Ptr gStrippedAddress = (Ptr)0x43524953;	//"magic" value

void InitMyPrivateTE(const Rect* box,
					 short heapSize);


//----------------------------------------------------------------------------------------
// StdNoRect: StdNoRect filters out the Rect drawing calls.
//----------------------------------------------------------------------------------------
#pragma segment MAUtilitiesRes

pascal void StdNoRect(GrafVerb /* verb */,
					  const Rect* /* r */ )
{
}

//----------------------------------------------------------------------------------------
// InitMyPrivateTE: 
//----------------------------------------------------------------------------------------
void InitMyPrivateTE(const Rect* box,
					 short heapSize)

{
	const short kZoneHeader = 52;				// 52 bytes for header 
	const short kZoneTrailer = 12;				// 12 bytes for trailer 
	const short kMPBlockHeader = 8;				// 8 bytes for Master Pointer block hdr 
	const short kInitialMstrPtrs = 2;			// 2 master pointers created initially 
	const short kSlop = 20;						// bytes of slop (just in case) 
	const short kZoneOverhead = kZoneHeader + kZoneTrailer + kMPBlockHeader + 4 * kInitialMstrPtrs + kSlop;// how large the zone overhead is 
	Ptr aTEZonePtr;

	gMATextBoxTE = TENew(box, box);
	if (gMATextBoxTE == NULL)					// can't allocate space for our terecord 
		return;

	// ¥ save off several items of interest
	gTEDefaultWordBreak = (*gMATextBoxTE)->wordBreak;
	pSaveHText = (*gMATextBoxTE)->hText;		// save the text handle 

	// Since TESetText (called near the end of MATextBox) hits the heap, we can speed this
	// hit to the heap for small text lengths (<= 255), by allocating a special text
	// handle in its own separate heap. We'll use this text handle whenever the text
	// length is <= 255.

	// ¥ create a separate heap
	aTEZonePtr = NewPtr(((heapSize + 7) & ~7) + kZoneOverhead);
	if (aTEZonePtr == NULL)						// can't allocate space for our heap 
		return;
	InitZone(NULL, kInitialMstrPtrs, aTEZonePtr + GetPtrSize(aTEZonePtr), aTEZonePtr);

	// ¥ InitZone sets the current zone to the newly created zone

	// ¥ allocate our new text handle in our new heap zone
	pMATextBoxHText = NewHandle(heapSize);		// the text handle 

	// ¥ restore the heap zone
	SetZone(ApplicationZone());
}												// InitMyPrivateTE 



SInt16 UGladUtilities::Int16Min(SInt16 inA,
							   SInt16 inB)
{
	if (inA < inB)
		return inA;
	else
		return inB;
}


SInt32 UGladUtilities::Int32Min(SInt32 inA,
							   SInt32 inB)
{
	if (inA < inB)
		return inA;
	else
		return inB;
}

//----------------------------------------------------------------------------------------
// IsColorPort: Returns TRUE if the GrafPort is a Color QD GrafPort test is:
// IsColorPort = CGrafPtr(port)->portversion == 0xC000
//----------------------------------------------------------------------------------------
#pragma segment MAUtilitiesRes

Boolean UGladUtilities::IsColorPort(GrafPtr port)
{
	if (port != NULL)
		return ((((CGrafPtr)port)->portVersion) & 0xC000) == 0x0000C000;
	else
		return FALSE;
}												// IsColorPort 

//----------------------------------------------------------------------------------------
// GetActualJustification: 
//----------------------------------------------------------------------------------------
#pragma segment MAUtilitiesRes

short UGladUtilities::GetActualJustification(short justification)

{
	if (justification == teFlushDefault)
		return GetSysDirection();
	else
		return justification;
}												// GetActualJustification 

//----------------------------------------------------------------------------------------
// MAGetFontInfo: 
//----------------------------------------------------------------------------------------
#pragma segment MAUtilitiesRes

short UGladUtilities::MAGetFontInfo(FontInfo* theFontInfo)
// MAGetFontInfo may be used in place of GetFontInfo since it also returns the font height. 

{
	::GetFontInfo(theFontInfo);					// get the current port's font info 
	return (theFontInfo->ascent + theFontInfo->descent + theFontInfo->leading);// returns the font height 
}												// MAGetFontInfo 


//----------------------------------------------------------------------------------------
// MATextBox: 
//----------------------------------------------------------------------------------------
void UGladUtilities::MATextBox(Ptr text,
							   long itsLength,
							   const Rect* box,
							   short itsJust,
							   Boolean autoWrap,
							   WordBreakUPP wordBreak,
							   Boolean eraseFirst,
							   Boolean spaceForCaret,
							   Boolean			/*preferOutline*/)

{
	union QuickDrawProcs
	{
		QDProcs theQDProcs;
		CQDProcs theCQDProcs;
	};

	const short kTextBoxCaretSlopSize = 1;		// Since TextBox uses TE to image the
	// text, we may need to adjust by 1 pixel.
	// Reason: TE draws beginning 1 pixel to
	// the right to allow for the insertion
	// point (which we won't have since this
	// is drawn text, not editable text).
	const short kMaxTEChars = 32000;			// Actually TE suffers some other
	// limitations as well. Such as
	// misbehaviour and or bombing when the
	// sum of the lineheights > 32k or a
	// linewidth > 32k (overflows QuickDraw
	// space) But these are _MUCH_ more
	// difficult to test for in a quick way
	const short kOurHeapSize = 256;				// our zone size 

	QuickDrawProcs myQDProcs;
	QDProcsPtr saveQDProcsPtr;
	QDRectUPP saveRectProc;
	GrafPtr curPort = nil;

	// Create my goodies if necessary 
	if (gMATextBoxTE == NULL)
	{
		InitMyPrivateTE(box, kOurHeapSize);

		if (gMATextBoxTE == NULL)				// couldn't allocate the TE handle 
		{
			::TETextBox(text, itsLength, box, itsJust);// default to TextBox in low memory 
			return;
		}
	}

	// Setup the work TE with the necessary parameters 
	FontInfo fInfo;
	short lineHeight = MAGetFontInfo(&fInfo);	// Need to get font's height and ascent. 

	::GetPort(&curPort);
	SectRect(&((**(curPort->clipRgn)).rgnBBox), box, &((**gMATextBoxTE).viewRect));
	((Rect &)(**gMATextBoxTE).destRect) = *box;
	if (!spaceForCaret)							// widen the destrect but not the visrect.
	// This lets the 1 pixel wide area to the
	// left of all text and the right of all
	// text go unshown.
	{
		(*gMATextBoxTE)->destRect.left = (*gMATextBoxTE)->destRect.left - kTextBoxCaretSlopSize;
		(*gMATextBoxTE)->destRect.right = (*gMATextBoxTE)->destRect.right + kTextBoxCaretSlopSize;
	}

	(*gMATextBoxTE)->inPort = curPort;			// Current port and its characteristics 

	(*gMATextBoxTE)->txSize = curPort->txSize;
	(*gMATextBoxTE)->txFont = curPort->txFont;
	(*gMATextBoxTE)->txFace = curPort->txFace;
	(*gMATextBoxTE)->fontAscent = fInfo.ascent;
	(*gMATextBoxTE)->lineHeight = lineHeight;
	(*gMATextBoxTE)->just = itsJust;

	if (autoWrap)
		(*gMATextBoxTE)->crOnly = 0;			//if >=0, word wrap
	else
		(*gMATextBoxTE)->crOnly = -1;			//if <0, new line at Return only

	(*gMATextBoxTE)->wordBreak = gTEDefaultWordBreak;

	if (wordBreak != NULL)
		TESetWordBreak(wordBreak, gMATextBoxTE);// set the word break routine 

	if (pMATextBoxHText != NULL)				// if our private heap is set up 
	{
		if (itsLength > kOurHeapSize)
			(*gMATextBoxTE)->hText = pSaveHText;
		else
			(*gMATextBoxTE)->hText = pMATextBoxHText;
	}

	TESetText(text, Int16Min(itsLength, kMaxTEChars), gMATextBoxTE);

	// if called with eraseFirst TRUE, then let TEUpdate image with its built-in EraseRect 
	if (eraseFirst)
		TEUpdate(box, gMATextBoxTE);
	else
	{
		/* replace the existing QD procs ( standard or externally supplied )
		  so that the EraseRect in TEUpdate is ignored */
		saveQDProcsPtr = curPort->grafProcs;

		if (gStdNoRectProc == nil)
			gStdNoRectProc = NewQDRectProc(&StdNoRect);

		if (saveQDProcsPtr == NULL)
		{
			if (IsColorPort(curPort))
			{
				SetStdCProcs(&(myQDProcs.theCQDProcs));
				myQDProcs.theCQDProcs.rectProc = gStdNoRectProc;
				curPort->grafProcs = (QDProcsPtr) & myQDProcs.theCQDProcs;
			}
			else
			{
				SetStdProcs(&(myQDProcs.theQDProcs));
				myQDProcs.theQDProcs.rectProc = gStdNoRectProc;
				curPort->grafProcs = &myQDProcs.theQDProcs;
			}
		}
		else
		{
			saveRectProc = curPort->grafProcs->rectProc;
			curPort->grafProcs->rectProc = gStdNoRectProc;
		}

		// Now do the imaging 
		TEUpdate(box, gMATextBoxTE);

		// Restore the QDProcs 
		if (saveQDProcsPtr == NULL)
			curPort->grafProcs = NULL;
		else
			curPort->grafProcs->rectProc = saveRectProc;

	}
}												// MATextBox 

//----------------------------------------------------------------------------------------
// MADrawString: 
//----------------------------------------------------------------------------------------
#pragma segment MAUtilitiesRes

void UGladUtilities::MADrawString(ConstStr255Param s,
								  const Rect* box,
								  short justification,
								  Boolean		/*preferOutline*/)

{
	FontInfo theFontInfo;
	short widthOfString;
	short boxWidth;
	Rect localBox = *box;

	//CWhileOutlinePreferred setOP(preferOutline);

	GetFontInfo(&theFontInfo);
	widthOfString = StringWidth(s);
	boxWidth = localBox.right - localBox.left;
	if (widthOfString < boxWidth)
	{
		switch (GetActualJustification(justification))
		{
			case teFlushDefault:
				break;

			case teCenter:
				localBox.left += (boxWidth - widthOfString) / 2;
				break;

			case teFlushRight:
				localBox.left += boxWidth - widthOfString;
				break;

			case teFlushLeft:
				break;
		}
	}

	MoveTo(localBox.left, localBox.top + theFontInfo.ascent);
	DrawString(s);
}												// MADrawString 

SignedByte UGladUtilities::LockHandleHigh(Handle h)
{
	SignedByte savedState = SignedByte(0);
	if (h)
	{
		savedState = HGetState(h);
		HLockHi(h);
	}
	return savedState;
}

short UGladUtilities::SBUseResFile(short refNum)
{
	short saveRefNum = CurResFile();
	UseResFile(refNum);
	return saveRefNum;
}

void UGladUtilities::BlockSet(void* destPtr,
							  long byteCount,
							  unsigned char setVal)
{
	char* endPtr = NULL;
	char* workDest = NULL;
	unsigned long longSetVal = 0;
	long* longEndPtr = NULL;
	long* longSetValPtr;

	workDest = (char*)StripLong(destPtr);
	endPtr = (workDest + byteCount);

	longEndPtr = (long*)((long)(workDest + byteCount) & 0xFFFFFFFC);//Trunc to nearest 4 bytes

	//We do longword assignments when we have a chance
	if (byteCount >= 4)
	{
		while (((long)workDest) & 0x00000003)
			//Starting on an odd byte boundry
			*workDest++ = setVal;

		longSetVal = (setVal << 24) + (setVal << 16) + (setVal << 8) + setVal;// Lets get a 4 byte 'punch'.

		//Assign in 4 byte chunks what we can
		for (longSetValPtr = (long*)workDest; longSetValPtr < longEndPtr;)
			*longSetValPtr++ = longSetVal;
		workDest = (Ptr)longSetValPtr;
	}

	//Now finish assigning odd bytes
	while (workDest < endPtr)
		*workDest++ = setVal;
}												// BlockSet 




#pragma segment MAMemoryRes

Handle UGladUtilities::AppGet1Resource(ResType rType,
									   short rID)
{
	short oldResFile = SBUseResFile(LMGetCurApRefNum());
	Handle resource = ::Get1Resource(rType, rID);
	SBUseResFile(oldResFile);

	return resource;
}

#pragma segment MAMemoryRes
Handle UGladUtilities::AppGet1NamedResource(ResType rType,
											ConstStr255Param name)
{
	short oldResFile = SBUseResFile(LMGetCurApRefNum());
	Handle namedResource = ::Get1NamedResource(rType, name);
	SBUseResFile(oldResFile);

	return namedResource;
}

#pragma segment MAMemoryRes
Handle UGladUtilities::AppGet1IndResource(ResType rType,
										  short index)
{
	short oldResFile = SBUseResFile(LMGetCurApRefNum());
	Handle indResource = ::Get1IndResource(rType, index);
	SBUseResFile(oldResFile);

	return indResource;
}

#pragma segment MAMemoryRes

short UGladUtilities::AppCount1Resources(ResType rType)
{
	short oldResFile = SBUseResFile(LMGetCurApRefNum());
	short resourceCnt = ::Count1Resources(rType);
	SBUseResFile(oldResFile);

	return resourceCnt;
}


#pragma segment MAMemoryRes
Handle UGladUtilities::AppGetResource(ResType rType,
									  short rID)
{
	short oldResFile = SBUseResFile(LMGetCurApRefNum());
	Handle h = ::GetResource(rType, rID);
	SBUseResFile(oldResFile);

	return h;
}

#pragma segment MAMemoryRes
Handle UGladUtilities::AppGetNamedResource(ResType rType,
										   ConstStr255Param name)
{
	short oldResFile = SBUseResFile(LMGetCurApRefNum());
	Handle h = ::GetNamedResource(rType, name);
	SBUseResFile(oldResFile);

	return h;
}

#pragma segment MAMemoryRes
Handle UGladUtilities::AppGetIndResource(ResType rType,
										 short index)
{
	short oldResFile = SBUseResFile(LMGetCurApRefNum());
	Handle h = ::GetIndResource(rType, index);
	SBUseResFile(oldResFile);

	return h;
}

#pragma segment MAMemoryRes
short UGladUtilities::AppCountResources(ResType rType)
{
	short oldResFile = SBUseResFile(LMGetCurApRefNum());
	short resourceCnt = ::CountResources(rType);
	SBUseResFile(oldResFile);

	return resourceCnt;
}

OSErr UGladUtilities::RemvResourceByID(ResType theType,
									   short theID)
{
	OSErr resErr = 0;
	Handle h = nil;
	Boolean oldResLoad = LMGetResLoad();

	SetResLoad(FALSE);
	h = Get1Resource(theType, theID);
	SetResLoad(oldResLoad);
	resErr = ResError();
	if (h != nil)
	{
		RemoveResource(h);
		resErr = ResError();
	}

	return resErr;
}

void UGladUtilities::PathNameFromDirID(long DirID,
									   short vRefNum,
									   CMyStr255& pathName)
{
	OSErr err;
	CInfoPBRec cInfoPB;
	CMyStr255 dirName = "";
	CMyStr255 delimiter;

	pathName = "";

	/*
	  if (gConfiguration.hasAUX)
	  delimiter = "/";
	  else
	*/
	delimiter = ":";

	cInfoPB.dirInfo.ioFDirIndex = -1;
	cInfoPB.dirInfo.ioVRefNum = vRefNum;
	cInfoPB.dirInfo.ioDrDirID = DirID;
	cInfoPB.dirInfo.ioNamePtr = (StringPtr)dirName;

	err = noErr;
	while (err == noErr)
	{
		err = PBGetCatInfoSync(&cInfoPB);
		pathName.Insert(dirName + delimiter, 1);
		if (cInfoPB.dirInfo.ioDrDirID == fsRtDirID)
			break;
		cInfoPB.dirInfo.ioDrDirID = cInfoPB.dirInfo.ioDrParID;
	}

	/*
	  if (gConfiguration.hasAUX)
	  pathName.Insert("/", 1);
	*/
}


/** PathNameFromWD ************************************************************/

void UGladUtilities::PathNameFromWD(long vRefNum,
									CMyStr255& pathName)
{
	WDPBRec myBlock;

	/*
	  PBGetWDInfo has a bug under A/UX 1.1.  If vRefNum is a real vRefNum
	  and not a wdRefNum, then it returns garbage.  Since A/UX has only 1
	  volume (in the Macintosh sense) and only 1 root directory, this can
	  occur only when a file has been selected in the root directory (/).
	  So we look for this and hard code the DirID and vRefNum. */

	/*
	  if (gConfiguration.hasAUX && (vRefNum == -1))
	  {
	  PathNameFromDirID(2, -1, pathName);
	  return;
	  }
	*/
	myBlock.ioNamePtr = nil;
	myBlock.ioVRefNum = (short)vRefNum;
	myBlock.ioWDIndex = 0;
	myBlock.ioWDProcID = 0;

	/* Change the Working Directory number in vRefnum into a real vRefnum */
	/* and DirID. The real vRefnum is returned in ioVRefnum, and the real */
	/* DirID is returned in ioWDDirID. */

	PBGetWDInfoSync(&myBlock);

	PathNameFromDirID(myBlock.ioWDDirID, myBlock.ioWDVRefNum, pathName);
}


void UGladUtilities::PathNameFromAlias(AliasHandle alias,
									   CMyStr255& pathName,
									   Boolean includeAliasName,
									   Boolean interactionAllowed)
{
	FSSpec fileSpec;
	Boolean wasChanged;
	OSErr err = noErr;

	pathName = "";

	if (alias != nil)
	{
		if (interactionAllowed)
			err = ResolveAlias(nil, alias, &fileSpec, &wasChanged);
		else
			err = ResolveAliasNoUI(nil, alias, &fileSpec, &wasChanged);
	}

	if (err == noErr)
		PathNameFromDirID(fileSpec.parID, fileSpec.vRefNum, pathName);

	if (includeAliasName)
	{
		CMyStr255 delimiter;
		/*		if (gConfiguration.hasAUX)
		  delimiter = "/";
		  else
		*/
		delimiter = ":";

		pathName.Insert(fileSpec.name, pathName.Length() + 1);
	}

	if (err != fnfErr && err != noErr)
		ThrowIfOSErr_(err);
}


long UGladUtilities::StripLong(void* address)
{
	// -1 == 0xFFFFFFFF, the largest 32 bit address. Our routine StripLong uses a
	// pre-stripped address gStrippedAddress to avoid the yucky MPW glue. (NOTE: need
	// gStrippedAddress in DefineConfiguration.)
	if (gStrippedAddress == (Ptr)0x43524953)
		gStrippedAddress = StripAddress((Ptr) - 1);

	return ((long)address & (long)gStrippedAddress);
}

OSErr UGladUtilities::ResolveAliasNoUI(const FSSpec* fromFile,
									   AliasHandle alias,
									   FSSpec* target,
									   Boolean* wasChanged)
{
	short aliasCount = 1;

	return MatchAlias(fromFile, kARMNoUI + kARMSearch + kARMMountVol + kARMMultVols, alias, &aliasCount, (FSSpecArrayPtr)target, wasChanged, nil, nil);
}


void UGladUtilities::PullApplicationToFront()

{
	EventRecord theEvent;

	// The "Programmer's guide to MultiFinder says make an event call several times. I
	// guess 3 calls counts as several. Also, it says call GetNextEvent but we don't want
	// to lose events on the floor so we use EventAvail since it seems to work OK
	for (short i = 1; i <= 3; ++i)
		EventAvail(everyEvent, &theEvent);
}

Handle UGladUtilities::DisposeIfHandle(Handle h)
{
	if (h != nil)								//debugging version should check handle for validity
		::DisposeHandle(h);

	return nil;
}

Ptr UGladUtilities::DisposeIfHandle(Ptr p)
{
	if (p != nil)								//debugging version should check Ptr for validity
		::DisposePtr(p);

	return nil;
}

Boolean UGladUtilities::SameFile(const FSSpec* file1,
								 const FSSpec* file2)
{
	if (file1->vRefNum != file2->vRefNum)
		return false;
	if (file1->parID != file2->parID)
		return false;
	if (PLstrcmp(file1->name, file2->name) != 0)
		return false;
	return true;
}

OSErr UGladUtilities::MakeCanonFSSpec(FSSpec* fSpec)
{
	CInfoPBRec infoPB;
	OSErr err;

	if (fSpec->name[0] != 0)
		return noErr;

	infoPB.dirInfo.ioNamePtr = fSpec->name;
	infoPB.dirInfo.ioVRefNum = fSpec->vRefNum;
	infoPB.dirInfo.ioDrDirID = fSpec->parID;
	infoPB.dirInfo.ioFDirIndex = -1;
	err = PBGetCatInfoSync((CInfoPBPtr) & infoPB);
	fSpec->parID = infoPB.dirInfo.ioDrParID;

	return err;
}



// take a canonical fsspec and return it ready for addition of a file name
OSErr UGladUtilities::MakeNonCanonFSSpec(FSSpec* fSpec)
{
	CInfoPBRec infoPB;
	OSErr err;

	if (fSpec->name[0] == 0)
		return noErr;

	infoPB.dirInfo.ioNamePtr = fSpec->name;
	infoPB.dirInfo.ioVRefNum = fSpec->vRefNum;
	infoPB.dirInfo.ioDrDirID = fSpec->parID;
	infoPB.dirInfo.ioFDirIndex = 0;
	err = PBGetCatInfoSync((CInfoPBPtr) & infoPB);
	fSpec->parID = infoPB.dirInfo.ioDrDirID;
	fSpec->name[0] = 0;

	return err;
}

OSErr UGladUtilities::GetDeskFolderSpec(FSSpec* fSpec,
										short vRefNum)
{
	OSErr err;

	fSpec->name[0] = 0;
	err = FindFolder(vRefNum, kDesktopFolderType, kDontCreateFolder, &fSpec->vRefNum, &fSpec->parID);
	if (err != noErr)
		return err;

	return MakeCanonFSSpec(fSpec);
}



#pragma segment FileUtils
void UGladUtilities::GetIndDirectory(short index,
									 short vRefNum,
									 long dirID,
									 MyFileFilterProcPtr fileFilter,
									 FSSpec& fileSpec)
{
	CMyStr255 fileName = "";
	CInfoPBRec infoPBRec;
	OSErr err;
	short x = 0;
	short actualIndex = 0;


	fileSpec.vRefNum = 0;
	fileSpec.parID = 0;
	*((CMyStr255 *)fileSpec.name) = fileName;

	infoPBRec.hFileInfo.ioCompletion = nil;
	infoPBRec.hFileInfo.ioNamePtr = (StringPtr) & fileName;
	infoPBRec.hFileInfo.ioVRefNum = vRefNum;

	// I have to loop because I want to scan past any files that may exist hereÉ

	do
	{
		x++;

		infoPBRec.hFileInfo.ioFDirIndex = x;
		infoPBRec.dirInfo.ioDrDirID = dirID;	//reset dirID; PBGetCatInfoSync may change it
		// ??? InsideMac??? infoPBRec.hFileInfo.ioACUser = 0;

		err = PBGetCatInfoSync(&infoPBRec);
		if ((!err) && (infoPBRec.hFileInfo.ioFlAttrib & kFolderMask) && (!fileFilter || fileFilter(&infoPBRec, nil)))
		{
			ThrowIfOSErr_(FSMakeFSSpec(vRefNum, dirID, fileName, &fileSpec));
			actualIndex++;
		}
	}
	while (!err && (actualIndex != index))
		;

	if (err != fnfErr)
		ThrowIfOSErr_(err);
}

#pragma segment FileUtils
void UGladUtilities::GetIndFileSpec(short index,
									short vRefNum,
									long dirID,
									MyFileFilterProcPtr fileFilter,
									FSSpec& fileSpec)
{
	CMyStr255 fileName = "";
	CInfoPBRec infoPBRec;
	OSErr err;
	short x = 0;
	short actualIndex = 0;


	fileSpec.vRefNum = 0;
	fileSpec.parID = 0;
	fileSpec.name[0]=0;

	infoPBRec.hFileInfo.ioCompletion = nil;
	infoPBRec.hFileInfo.ioNamePtr = (StringPtr) & fileName;
	infoPBRec.hFileInfo.ioVRefNum = vRefNum;

	// I have to loop because I want to scan past any directories that may exist hereÉ

	do
	{
		x++;

		infoPBRec.hFileInfo.ioFDirIndex = x;
		infoPBRec.dirInfo.ioDrDirID = dirID;	//reset dirID; PBGetCatInfoSync may change it
		// ??? InsideMac??? infoPBRec.hFileInfo.ioACUser = 0;

		err = PBGetCatInfoSync(&infoPBRec);
		if ((!err) && !(infoPBRec.hFileInfo.ioFlAttrib & kFolderMask) && (!fileFilter || fileFilter(&infoPBRec, nil)))
		{
			ThrowIfOSErr_(FSMakeFSSpec(vRefNum, dirID, fileName, &fileSpec));
			actualIndex++;
		}
	}
	while (!err && (actualIndex != index))
		;

	if (err != fnfErr)
		ThrowIfOSErr_(err);
}

#pragma segment FileUtils
short UGladUtilities::CountFiles(short vRefNum,
								 long dirID,
								 MyFileFilterProcPtr fileFilter)
{
	CMyStr255 fileName;
	CInfoPBRec infoPBRec;
	OSErr err;
	short x = 0;
	short fileCount = 0;

	infoPBRec.hFileInfo.ioCompletion = nil;
	infoPBRec.hFileInfo.ioNamePtr = (StringPtr) & fileName;
	infoPBRec.hFileInfo.ioVRefNum = vRefNum;


	do
	{
		x++;

		infoPBRec.hFileInfo.ioFDirIndex = x;
		infoPBRec.dirInfo.ioDrDirID = dirID;	//reset dirID; PBGetCatInfoSync may change it
		// ??? InsideMac??? infoPBRec.hFileInfo.ioACUser = 0;

		err = PBGetCatInfoSync(&infoPBRec);
		if ((!err) && !(infoPBRec.hFileInfo.ioFlAttrib & kFolderMask) && (!fileFilter || fileFilter(&infoPBRec, nil)))
		{
			fileCount++;
		}
	}
	while (!err)
		;

	if (err != fnfErr)
		ThrowIfOSErr_(err);

	return fileCount;
}

#pragma segment FileUtils
OSErr UGladUtilities::GetFileInfo(const FSSpec& fileSpec,
								  HParamBlockRec* pb)
{
	CMyStr255 itsName;

	itsName = fileSpec.name;
	pb->fileParam.ioNamePtr = (StringPtr)itsName;
	pb->fileParam.ioVRefNum = fileSpec.vRefNum;
	pb->fileParam.ioDirID = fileSpec.parID;
	pb->fileParam.ioFVersNum = 0;
	pb->fileParam.ioFDirIndex = 0;
	OSErr err = PBHGetFInfoSync(pb);
	pb->fileParam.ioNamePtr = NULL;
	return err;
}

OSErr UGladUtilities::SetFileInfo(const FSSpec& fileSpec,
								  HParamBlockRec* pb)
{
	CMyStr255 itsName;

	itsName = fileSpec.name;
	pb->fileParam.ioNamePtr = (StringPtr)itsName;
	pb->fileParam.ioVRefNum = fileSpec.vRefNum;
	pb->fileParam.ioDirID = fileSpec.parID;
	pb->fileParam.ioFVersNum = 0;
	pb->fileParam.ioFDirIndex = 0;
	OSErr err = PBHSetFInfoSync(pb);
	return err;
}


OSErr UGladUtilities::FSpGetPhysicalFileSize(const FSSpec& fileSpec,
												long *dataSize,
												long *rsrcSize)
{
	HParamBlockRec pb;
	OSErr error;
	CMyStr255 fileName = fileSpec.name;
	
	pb.fileParam.ioNamePtr = (StringPtr)fileName;
	pb.fileParam.ioVRefNum = fileSpec.vRefNum;
	pb.fileParam.ioFVersNum = 0;
	pb.fileParam.ioDirID = fileSpec.parID;
	pb.fileParam.ioFDirIndex = 0;
	error = PBHGetFInfoSync(&pb);
	if ( error == noErr )
	{
		*dataSize = pb.fileParam.ioFlPyLen;
		*rsrcSize = pb.fileParam.ioFlRPyLen;
	}
	
	return ( error );
}


OSErr UGladUtilities::FSpUnlockFile(const FSSpec& fileSpec)
{	
	OSErr error;
	HParamBlockRec pb;
	error = GetFileInfo(fileSpec,&pb);
	if(error) return error;
	pb.fileParam.ioFlAttrib &= ~0x01;	// turn off the locked bit
	error = SetFileInfo(fileSpec,&pb);
	return error;
}


void UGladUtilities::FindNewestFileByTypeAndCreator(short inVRefNum,  OSType inType,  OSType inCreator,  FSSpec* iofileSpec)
{
	/* Search for the most recent copy of the file with the requested 
	   type/creator that isn't in the trash or inside a hidden folder  */

	FSSpec fsSpecArray[5] = {};	//we'll grab 5 at a time (arbitrary value)
	OSErr err = noErr;
	long actMatchCount = 0;
	Boolean done = false;
	Boolean newSearch = true;
	Boolean foundOne = false;
	unsigned long latestModDateSoFar = 0;
	short trashVRefNum = 0;
	long trashDirID = 0;
	
	(void)::FindFolder(inVRefNum, kTrashFolderType, false, &trashVRefNum, &trashDirID);	//get the trash dirID
	
	*iofileSpec = fsSpecArray[0]; //clear out the input parameter
	
	do {
		err = ::CreatorTypeFileSearch(nil, inVRefNum, inCreator, inType, fsSpecArray, 5, &actMatchCount, newSearch);
		newSearch = false;	//susequent calls should start where we left off with the prior call
		if (err != eofErr)
			ThrowIfOSErr_(err);
		
		if (err == eofErr)	//have we seen the entire volume?
		{
			done = true;
			err = noErr;
		}
		
		for (short i = 0; i<actMatchCount; i++)
		{
			if ( fsSpecArray[i].parID != trashDirID )	//ignore files that are in the trash
			{
				CInfoPBRec pb = {};
		
				err = ::GetCatInfoNoName (fsSpecArray[i].vRefNum, fsSpecArray[i].parID, fsSpecArray[i].name, &pb);
				
				if ( err == noErr )
				{
					unsigned long curFileModDate = pb.hFileInfo.ioFlMdDat;	//cache the current file's mod date (cuz I'm going to reuse pb for the dir)
					
					if ( curFileModDate > latestModDateSoFar ) //try to find the latest version
					{
						err = ::GetCatInfoNoName (fsSpecArray[i].vRefNum, fsSpecArray[i].parID, nil, &pb); //grab info about the containing dir

						if ((pb.hFileInfo.ioFlFndrInfo.fdFlags & fInvisible) == 0)	//is it in a visible folder?
						{
							latestModDateSoFar = curFileModDate;
							*iofileSpec = fsSpecArray[i];
							foundOne = true;
						}
					}
				}
			}
		}
		
	} while (!done);


	if (!foundOne)	//if we didn't find any usable files, return a "file not found" err
	{
		Throw_(fnfErr);
	}
}


void UGladUtilities::ReadFileChunk(const FSSpec& fileSpec,
								   void* buffer,
								   long offset,
								   long* count,
								   short inDataRefNum)
{
	short dfRefNum = inDataRefNum;
	SInt32 savedFPos = 0;


	try
	{
		if (inDataRefNum == refNum_Undefined)
			ThrowIfOSErr_(::FSpOpenDF(&fileSpec, fsRdPerm, &dfRefNum));
		else
			ThrowIfOSErr_(::GetFPos(dfRefNum, &savedFPos));

		ThrowIfOSErr_(::SetFPos(dfRefNum, fsFromStart, offset));
		ThrowIfOSErr_(::FSRead(dfRefNum, count, buffer));
		if (inDataRefNum == refNum_Undefined)
			ThrowIfOSErr_(::FSClose(dfRefNum));
		else
			ThrowIfOSErr_(::SetFPos(dfRefNum, fsFromStart, savedFPos));
	}
	catch (const LException& inErr)
	{
		if (inDataRefNum == refNum_Undefined)
		{
			if (dfRefNum != 0)
				::FSClose(dfRefNum);
		}
		else
			ThrowIfOSErr_(::SetFPos(dfRefNum, fsFromStart, savedFPos));

		throw;
	}


	
}


void UGladUtilities::WriteFileChunk(const FSSpec& fileSpec,
									void* buffer,
									long offset,
									long* count,
									short inDataRefNum)
{
	short dfRefNum = inDataRefNum;
	SInt32 savedFPos = 0;


	try
	{
		if (inDataRefNum == refNum_Undefined)
			ThrowIfOSErr_(::FSpOpenDF(&fileSpec, fsWrPerm, &dfRefNum));
		else
			ThrowIfOSErr_(::GetFPos(dfRefNum, &savedFPos));

		ThrowIfOSErr_(::SetFPos(dfRefNum, fsFromStart, offset));
		ThrowIfOSErr_(::FSWrite(dfRefNum, count, buffer));

		if (inDataRefNum == refNum_Undefined)
			ThrowIfOSErr_(::FSClose(dfRefNum));
		else
			ThrowIfOSErr_(::SetFPos(dfRefNum, fsFromStart, savedFPos));
	}
	catch (const LException& inErr)
	{
		if (inDataRefNum == refNum_Undefined)
		{
			if (dfRefNum != 0)
				::FSClose(dfRefNum);
		}
		else
			ThrowIfOSErr_(::SetFPos(dfRefNum, fsFromStart, savedFPos));

		throw;
	}

	
}

#pragma segment Main
void UGladUtilities::CenterRect(Rect* ioRect,
								const Rect& anchorRect,
								IconAlignmentType align)
{
	Point rectSize = {ioRect->bottom - ioRect->top, ioRect-> right- ioRect-> left};
	Point anchorSize = { anchorRect.bottom - anchorRect.top, anchorRect. right- anchorRect. left};

	if (align & kAlignHorizontalCenter)
		ioRect->left = anchorRect.left + (anchorSize.h - rectSize.h) / 2;
	else if (align & atLeft)
		ioRect->left = anchorRect.left;
	else if (align & atRight)
		ioRect->left = anchorRect.left + (anchorSize.h - rectSize.h);

	if (align & kAlignVerticalCenter)
		ioRect->top = anchorRect.top + (anchorSize.v - rectSize.v) / 2;
	else if (align & atTop)
		ioRect->top = anchorRect.top;
	else if (align & atBottom)
		ioRect->top = 2 * (anchorRect.top + (anchorSize.v - rectSize.v));


	ioRect->right = ioRect->left + rectSize.h;
	ioRect->bottom = ioRect->top + rectSize.v;
}

void UGladUtilities::SetStackSpace(long numBytes)
{
	long newLimit;

	newLimit = (long)LMGetCurStackBase() - numBytes;
	if ((long)LMGetApplLimit() > newLimit)
		LMSetCurStackBase((Ptr)newLimit);
}

OSErr UGladUtilities::IncreaseApplicationStack(Size incrementSize)
{
	OSErr retCode;

	// Increase the stack size by lowering the heap limit.
	LMSetApplLimit((Ptr)((unsigned long)LMGetApplLimit() - incrementSize));
	retCode = MemError();
	if (retCode == noErr)
		MaxApplZone();

	return retCode;
}


Boolean UGladUtilities::IsThisKeyDown(const short theKey)
{
	union
	{
		KeyMap asMap;
		Byte asBytes[16];
	};

	GetKeys(asMap);
	return asBytes[theKey >> 3] & (1 << (theKey & 0x07)) ? TRUE : FALSE;
}

#pragma segment Main
void UGladUtilities::ErrorAlert(OSErr inErr,
								char* inErrMsg)
{
	const short ALRT_ErrorMsg = 900;

	char ASErrCStr[255];
	char errNumCStr[16];

	//		if (UAppleScriptSupport::IsAppleScriptError(inErr))
	//		{
	//			UAppleScriptSupport::OSAErrorToString(inErr, ASErrCStr);
	//			inErr = UAppleScriptSupport::OSAErrorToOSErr(inErr);
	//		}

	::numtostring(inErr, errNumCStr);

	::paramtext(errNumCStr, "", inErrMsg, ASErrCStr);
	::StopAlert(ALRT_ErrorMsg, nil);
}

#pragma segment Main
void UGladUtilities::ZeroBlock(void* block,
							   long blockSize)
{
	BlockSet(block, blockSize, 0);
}

#pragma segment Main
OSErr UGladUtilities::PlotIconID(const Rect& theRect,
								 IconAlignmentType align,
								 IconTransformType transform,
								 short theResID,
								 const RGBColor& backColor)
{
	OSErr err = noErr;
	RGBColor saveBackColor;
	AuxWinHandle auxWin = nil;
	WindowPtr window = ::FrontWindow();
	if (window != nil)
	{
		::GetAuxWin(window, &auxWin);
		if (auxWin != nil)
		{
			//::HandToHand(&auxWin);
			saveBackColor = (**(**auxWin).awCTable).ctTable[0].rgb;
			(**(**auxWin).awCTable).ctTable[0].rgb = backColor;
			//::SetWinColor (window, auxWin);
		}
	}
	err = ::PlotIconID(&theRect, align, transform, theResID);

	if (window != nil && auxWin != nil)
		(**(**auxWin).awCTable).ctTable[0].rgb = saveBackColor;

	return err;
}

#pragma segment Main
void UGladUtilities::Frame3DRect(Rect* inRect)
{
	StColorPenState saveState;

	GDHandle hThisDevice;
	RGBColor saveBgColor;
	const RGBColor whiteColor = { 65535, 65535, 65535 };
	RGBColor grayColor = { 0, 0, 0 };
	Boolean bgInColor = false;

	::GetBackColor(&saveBgColor);
	::GetForeColor(&grayColor);
	bgInColor = !(saveBgColor.red == 65535 && saveBgColor.green == 65535 && saveBgColor.blue == 65535);

	hThisDevice = ::GetGDevice();
	::GetGray(hThisDevice, &saveBgColor, &grayColor);

	if (bgInColor)
	{
		::RGBForeColor(&whiteColor);
		::MoveTo(inRect->right, inRect->top);
		::LineTo(inRect->right, inRect->bottom);
		::LineTo(inRect->left, inRect->bottom);

		::RGBForeColor(&grayColor);
		::LineTo(inRect->left, inRect->top);
		::LineTo(inRect->right, inRect->top);
	}
	else
		::FrameRect(inRect);					// frame the groupbox
}

#pragma segment Main
OSErr UGladUtilities::GenerateUniqueFileName(short volume,
											 long dirID,
											 Str63 ioFileName)
{
	// assumes that volRefNum & dirID are valid
	OSErr err;
	CInfoPBRec cinfo;
	Str31 fileExt;
	Str31 baseName;
	long attemptNum = 1;

		// limit input name to max size for a legal file/folder name
	if(ioFileName[0] > (sizeof(Str31) - 1)) ioFileName[0] = sizeof(Str31) - 1;
	
	CMyStr255::CopyPStr(ioFileName, baseName, sizeof(Str31));//save off the base name


	do
	{
		cinfo.hFileInfo.ioVRefNum = volume;
		cinfo.hFileInfo.ioFDirIndex = 0;
		cinfo.hFileInfo.ioNamePtr = ioFileName;
		cinfo.hFileInfo.ioDirID = dirID;
		if (attemptNum > 1)
		{
			::NumToString(attemptNum, fileExt);
			CMyStr255::CopyPStr(baseName, ioFileName, sizeof(Str31));
			ioFileName[0] -= (ioFileName[0] + fileExt[0] + 1) > 31 ? (ioFileName[0] + fileExt[0] + 1) - 31 : 0;
			CMyStr255::AppendPStr(ioFileName, "\p ", sizeof(Str31));
			CMyStr255::AppendPStr(ioFileName, fileExt, sizeof(Str31));
		}
		err = ::PBGetCatInfoSync(&cinfo);
		attemptNum++;
	}
	while (err == noErr)
		;

	if (err == fnfErr)
		err = noErr;

	return err;
}

#pragma segment Main
Boolean UGladUtilities::ConfirmAlert(char* inConfirmText)
{
	const short ALRT_ConfirmMsg = 910;

	::paramtext(inConfirmText, "", "", "");
	return (::NoteAlert(ALRT_ConfirmMsg, nil) == 1);
}


void UGladUtilities::CopyPStrToCStr(ConstStr255Param inPStr,
									char* ioCStrBuffer,
									short bufferLen)
{
#define Min(a,b) (a < b ? a : b)

	::BlockMove(&inPStr[1], ioCStrBuffer, Min(inPStr[0], bufferLen - 1));
	ioCStrBuffer[Min(inPStr[0], bufferLen - 1)] = 0;
}



void UGladUtilities::GetProcessFSSpec(FSSpec* ioProcessFSSpec)
{

	ProcessInfoRec processInfoRec;
	ProcessSerialNumber psn = { 0, kCurrentProcess };

	processInfoRec.processInfoLength = sizeof(processInfoRec);
	processInfoRec.processName = nil;
	processInfoRec.processAppSpec = ioProcessFSSpec;

	ThrowIfOSErr_(::GetProcessInformation(&psn, &processInfoRec));
}



void UGladUtilities::LaunchApp(FSSpec& applFSSpec)
{
    LaunchParamBlockRec launchParams = {};

    launchParams.launchAppSpec                      = &applFSSpec;
    launchParams.launchBlockID                      = extendedBlock;
    launchParams.launchEPBLength            = extendedBlockLen;
    launchParams.launchFileFlags            = nil;
    launchParams.launchControlFlags         = launchContinue + launchNoFileFlags;
    launchParams.launchAppParameters        = nil;

    ThrowIfOSErr_(::LaunchApplication( &launchParams ));
}


OSErr UGladUtilities::FindRunningProcess(OSType inProcessSig, OSType inProcessType, 
											ProcessSerialNumber* ioProcessSN)
{
    OSErr err = noErr;
    Boolean match;
    ProcessInfoRec processInfo ={};
    ProcessSerialNumber tempPSN = {kNoProcess, kNoProcess};
    
    *ioProcessSN = tempPSN; //"null" the param out

    processInfo.processInfoLength = sizeof(processInfo);
        
    do
    {
    	match = true;
        err = ::GetNextProcess(&tempPSN);
        if (err == noErr)
        {
            err = ::GetProcessInformation(&tempPSN, &processInfo);
        }

        if(processInfo.processSignature != inProcessSig) match = false;
        if(inProcessType && (processInfo.processType != inProcessType)) match = false;

    } while ( err == noErr && !match);

    if (err == noErr)
            *ioProcessSN = tempPSN;

    return err;
}

void UGladUtilities::SendQuitAE(const ProcessSerialNumber& inTargetPSN,Boolean waitForIt)
{
        StAEDescriptor  targetAdressDesc;
        StAEDescriptor  quitEvent;
        StAEDescriptor  aeReply;
        OSErr error;

        
        ThrowIfOSErr_(::AECreateDesc(typeProcessSerialNumber, (Ptr)&inTargetPSN,
                                   sizeof(inTargetPSN), &targetAdressDesc.mDesc));

        ThrowIfOSErr_(::AECreateAppleEvent(kCoreEventClass, kAEQuitApplication,
                                          &targetAdressDesc.mDesc,kAutoGenerateReturnID,
                                          kAnyTransactionID,&quitEvent.mDesc));

        error = ::AESend(&quitEvent.mDesc, &aeReply.mDesc, waitForIt ? kAEWaitReply : kAENoReply, 
								kAENormalPriority, kAEDefaultTimeout, nil, nil);
								
		if((error == -908) || (error == -917)) return;	// if victim is already dead, don't sweat it
								
		ThrowIfOSErr_(error);
}



OSType UGladUtilities::GetOurCreator()
{
	ProcessSerialNumber psn;
	ProcessInfoRec info;
	ThrowIfOSErr_(::GetCurrentProcess(&psn));
	info.processInfoLength = sizeof(info);
	info.processName = nil;
	info.processAppSpec = nil;
	ThrowIfOSErr_(::GetProcessInformation(&psn,&info));
	return info.processSignature;
}


OSErr UGladUtilities::GetFSSpecForCreator(FSSpec *spec,OSType creator,short vRefNum,long dirID)
{
	CInfoPBRec pb;
	CMyStr255 name;

	pb.hFileInfo.ioCompletion = nil;
	pb.hFileInfo.ioNamePtr = name;
	pb.hFileInfo.ioVRefNum = vRefNum;
	pb.hFileInfo.ioFDirIndex = 1;
	pb.hFileInfo.ioDirID = dirID;

	while(!PBGetCatInfoSync(&pb))
	{
		if(pb.hFileInfo.ioFlFndrInfo.fdCreator == creator)	// found it
		{
			spec->vRefNum = vRefNum;
			spec->parID = dirID;
			BlockMoveData(name,spec->name,sizeof(spec->name));
			return noErr;
		}
		pb.hFileInfo.ioFDirIndex++;
		pb.hFileInfo.ioDirID = dirID;
	}
	
	return fnfErr;
}


void UGladUtilities::PoseSplashWindow(short inDialogID,
									  short inColorSplashPictItemNum,
									  short inBWSplashPictItemNum)
{
	DialogPtr splashDialog = ::GetNewDialog(inDialogID, nil, (GrafPtr)-1);
	
	if (splashDialog != nil)
	{	
		GDHandle mainDevice = ::GetMainDevice();	//splash dialog comes up on the main device
		
		if ((**((**mainDevice).gdPMap)).pixelSize <= 4)
		{
			if (inColorSplashPictItemNum != -1)
			{
				::HideDialogItem(splashDialog, inColorSplashPictItemNum);
			}
		}
		else
		{
			if (inBWSplashPictItemNum != -1)
			{
				::HideDialogItem(splashDialog, inBWSplashPictItemNum);
			}
		}
	
		::ShowWindow(splashDialog);
		::DrawDialog(splashDialog);
		
		EventRecord theEvent = {};

		
		unsigned long splashStart = ::TickCount();
		while ((::TickCount()-splashStart < (60*4)))
		{
			if (::WaitNextEvent(mDownMask+mUpMask+keyDownMask+updateMask, &theEvent, 20, nil ))
			{
				if (theEvent.what == mouseDown || theEvent.what == keyDown)
				{
					break;
				}
				else if (theEvent.what == updateEvt && (DialogPtr)theEvent.message == splashDialog)
				{
					::SetPort(splashDialog);
					::BeginUpdate(splashDialog);
					::UpdateDialog(splashDialog, splashDialog->visRgn);
					::EndUpdate(splashDialog);
				}
			}
		}
		
		::DisposeDialog(splashDialog);
	}
}


Boolean UGladUtilities::FileExists( const FSSpec& inFileSpec )
{
	FSSpec tmpFSSpec;
	OSErr err = ::FSMakeFSSpec( inFileSpec.vRefNum, inFileSpec.parID, inFileSpec.name, &tmpFSSpec );
	if (err != fnfErr)
		ThrowIfOSErr_(err);

	return (err != fnfErr);		
}


Boolean UGladUtilities::FileHasResFork( const FSSpec& inFileSpec)
{
	OSErr error;
	short refNum = ::FSpOpenResFile(&inFileSpec, fsRdPerm);
	if(refNum == -1) return false;
	error = ResError();
	if(ResError() == eofErr) return false;
	ThrowIfOSErr_(error);
	::CloseResFile(refNum);
	return true;
}

Boolean UGladUtilities::ProcessIsRunning(OSType inProcessSig, Boolean includeCurrentProcess)
{
    OSErr err = noErr;
    Boolean sameProcess = false;
    ProcessInfoRec processInfo ={};
    ProcessSerialNumber psn = {kNoProcess, kNoProcess};
    ProcessSerialNumber currentPSN = {kNoProcess, kNoProcess};
    
    processInfo.processInfoLength = sizeof(processInfo);

	if (!includeCurrentProcess)
	{
		ThrowIfOSErr_(::GetCurrentProcess(&currentPSN));
	}

    do
    {
        err = ::GetNextProcess(&psn);
		if (err == procNotFound)
		{
			return false;
		}
		else
		{
			ThrowIfOSErr_(err);
		}

		if (!includeCurrentProcess)
		{
			ThrowIfOSErr_(::SameProcess(&psn, &currentPSN, &sameProcess));
		}
		else
			sameProcess = false;
		
		if ( includeCurrentProcess || !sameProcess) 
		{
			ThrowIfOSErr_(::GetProcessInformation(&psn, &processInfo));
		}

    } while ( processInfo.processSignature != inProcessSig );

    return true;
}

void UGladUtilities::FSpMakeAlias(FSSpec *file_location,FSSpec *alias_location)
{
	AliasHandle alias;
	FInfo	fi;	
	
	// create an alias handle for our new copy of the file
	ThrowIfOSErr_(::NewAlias(nil,file_location,&alias));

	ThrowIfOSErr_(FSpGetFInfo(file_location,&fi));
		
	// make a file for it
	// creator is same as file; type is same unless original is app, then it's special
	::FSpCreateResFile(alias_location,fi.fdCreator,(fi.fdType == 'APPL') ? 'adrp' : fi.fdType,smSystemScript);
	ThrowIfResError_();

	LFile theFile(*alias_location);
	StOpenFileRsrcFork openit(&theFile,fsRdWrPerm);
	{	// scope the open
		AddResource((Handle)alias,'alis',0,"\p");
		ThrowIfResError_();

		WriteResource((Handle)alias);
		ThrowIfResError_();
		ReleaseResource((Handle)alias);
	}

		// set up the correct finder info for our alias file
	ThrowIfOSErr_(FSpGetFInfo(alias_location,&fi));
	fi.fdFlags |= kIsAlias;	
	fi.fdFlags &= ~kHasBeenInited;
	ThrowIfOSErr_(FSpSetFInfo(alias_location,&fi));
}


//inFileSize is the size of the .prc data fork
Boolean UGladUtilities::IsValidDBHeader(const DatabaseHdrType& inDbHdr,long inFileSize)
{
	Boolean retval = true;

    if( (inDbHdr.creationDate == 0) ||
      ( inDbHdr.creationDate > inDbHdr.modificationDate) ||
      ( inDbHdr.appInfoID > inFileSize) ||
      ( inDbHdr.sortInfoID > inFileSize) )
    {
		retval = false;
    }

	return retval;          
}

#pragma segment Main
void UGladUtilities::FindFileByTypeAndCreator(  short inVRefNum, 
												long inStartingDirID, 
												OSType inFileType,  
												OSType inCreator,  
												FSSpec* iofileSpec)
{
	CInfoPBRec				searchInfo1, searchInfo2 = {};
	HParamBlockRec			pb = {};
	OSErr					error = noErr;
	static CatPositionRec	catPosition = {};
	long 					actMatchCount = 0;
	static short			lastVRefNum = 0;
	
	pb.csParam.ioNamePtr = NULL;
	pb.csParam.ioVRefNum = inVRefNum;
	pb.csParam.ioMatchPtr = iofileSpec;
	pb.csParam.ioReqMatchCount = 1;
	pb.csParam.ioSearchBits = fsSBFlAttrib + fsSBFlFndrInfo;	/* Looking for finder info file matches */
	pb.csParam.ioSearchInfo1 = &searchInfo1;
	pb.csParam.ioSearchInfo2 = &searchInfo2;
	pb.csParam.ioSearchTime = 0;
	catPosition.initialize = 0;	/* then search from beginning of catalog */
	
	pb.csParam.ioCatPosition = catPosition;
	pb.csParam.ioOptBuffer = GetTempBuffer(0x00004000, &pb.csParam.ioOptBufSize);

	/* no fileName */
	searchInfo1.hFileInfo.ioNamePtr = NULL;
	searchInfo2.hFileInfo.ioNamePtr = NULL;
	
	/* only match files (not directories) */
	searchInfo1.hFileInfo.ioFlAttrib = 0x00;
	searchInfo2.hFileInfo.ioFlAttrib = ioDirMask;
	
	/* search for creator; if creator = 0x00000000, ignore creator */
	searchInfo1.hFileInfo.ioFlFndrInfo.fdCreator = inCreator;
	if ( inCreator == (OSType)0x00000000 )
	{
		searchInfo2.hFileInfo.ioFlFndrInfo.fdCreator = (OSType)0x00000000;
	}
	else
	{
		searchInfo2.hFileInfo.ioFlFndrInfo.fdCreator = (OSType)0xffffffff;
	}
	
	/* search for fileType; if fileType = 0x00000000, ignore fileType */
	searchInfo1.hFileInfo.ioFlFndrInfo.fdType = inFileType;
	if ( inFileType == (OSType)0x00000000 )
	{
		searchInfo2.hFileInfo.ioFlFndrInfo.fdType = (OSType)0x00000000;
	}
	else
	{
		searchInfo2.hFileInfo.ioFlFndrInfo.fdType = (OSType)0xffffffff;
	}
	
	/* zero all other FInfo fields */
	searchInfo1.hFileInfo.ioFlFndrInfo.fdFlags = 0;
	searchInfo1.hFileInfo.ioFlFndrInfo.fdLocation.v = 0;
	searchInfo1.hFileInfo.ioFlFndrInfo.fdLocation.h = 0;
	searchInfo1.hFileInfo.ioFlFndrInfo.fdFldr = 0;
	
	searchInfo2.hFileInfo.ioFlFndrInfo.fdFlags = 0;
	searchInfo2.hFileInfo.ioFlFndrInfo.fdLocation.v = 0;
	searchInfo2.hFileInfo.ioFlFndrInfo.fdLocation.h = 0;
	searchInfo2.hFileInfo.ioFlFndrInfo.fdFldr = 0;

	error = IndexedSearch((CSParamPtr)&pb, inStartingDirID);
	
	if ( (error == noErr) ||							/* If no errors or the end of catalog was */
		 (error == eofErr) )							/* found, then the call was successful so */
	{
		actMatchCount = pb.csParam.ioActMatchCount;	/* return the match count */
	}
	else
	{
		actMatchCount = 0;							/* else no matches found */
	}
	
	if ( (error == noErr) ||						/* If no errors */
		 (error == catChangedErr) )					/* or there was a change in the catalog */
	{
		catPosition = pb.csParam.ioCatPosition;
		lastVRefNum = inVRefNum;
			/* we can probably start the next search where we stopped this time */
	}
	else
	{
		catPosition.initialize = 0;
			/* start the next search from beginning of catalog */
	}
	
	if ( pb.csParam.ioOptBuffer != NULL )
	{
		DisposePtr(pb.csParam.ioOptBuffer);
	}
		
	ThrowIfOSErr_(error);
}


Boolean UGladUtilities::ReadFileLine( short fileRef, Str255 outString, UInt8 blockSize )
{	//reads a line from a file. returns true if the line is longer than the amount passed into blockSize, false otherwise
	if( blockSize > 254 ) blockSize = 254;

	long startFPos;
	ThrowIfOSErr_( ::GetFPos( fileRef, &startFPos ) );

	long readLen = (unsigned long)blockSize+1;
	OSErr err = ::FSRead( fileRef, &readLen, &outString[1] );
	switch( err )
	{
		case noErr:
		case eofErr:
			break;
		default:
			ThrowOSErr_( err );
			break;
	}
	short i;
	for( i=1; i<=readLen && outString[i] != '\r'; i++ )
		;
	Boolean foundNewline = outString[i] == '\r';

	//make it a null-terminated Str255 so we can pass it as either a c string or a p string
	outString[i] = '\0';
	outString[0] = i-1;


	//if we found a newline or the line was longer than the blockSize, then set
	//the file marker to be right after the newline, or wherever we stopped reading
	if( !err || ( err == eofErr && i <= readLen ) )
		ThrowIfOSErr_( ::SetFPos( fileRef, fsFromStart, startFPos+outString[0]+1) );

	if( foundNewline || err == eofErr )
		return( false );
	else
		return( true );
}

//Anologous to TickCount(), but millsecond granularity
UInt64 UGladUtilities::MilliSecondCount()
{	
	UInt64 now = 0;
	
	::Microseconds((UnsignedWide*)&now);
	now /= 1000;
	
	return now;	
}


