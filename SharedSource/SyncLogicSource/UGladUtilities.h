
/*
  File:		UGladUtilities.h

  Contains:	A bunch of useful (and not so useful) utility functions packaged up as
  a "static" class.  Simply a way to package up gloabal functions in such
  a way that their origin is obvious (also helps to eliminate namespace 
  pollution).

  Written by:	Chris LeCroy, Jeff Pritchard

  Copyright:	© 1997 by Apple Computer, Inc., all rights reserved.

  Change History (most recent first):

		 <4>	  9/5/97	csl		Ran it through CDent
  <2>	 8/22/97	csl		

  To Do:	Remove the methods that are either broken or useless.  some of this stuff
  is really ancient.
*/

// UGladUtilities.h
// Created by It doesn't have a name on Sun, Jan 15, 1995 @ 2:52 PM.

#pragma once

#ifndef __UGladUtilities__
#define __UGladUtilities__

#include <PP_Types.h>
//#include <PascalString.h>
#include <TextEdit.h>
#include <Aliases.h>
#include <Icons.h>
//#include <LString.h>
#include "CMyStr255.h"
#include <LFile.h>

#include "DATAPRV.H"

//#include <SOBE_Types.h>

typedef Boolean(* MyFileFilterProcPtr)(CInfoPBPtr infoPBRecPtr,
									   void* userData);
										 
typedef void(* MyFSSpecActionProcPtr)(FSSpec& fileSpec);

typedef unsigned long long UInt64;
typedef	long long Int64;

const SInt16 refNum_Undefined = -1;

const char kFolderBit = 4;

const unsigned char kFolderMask = 1 << kFolderBit;

const Boolean kAutoWrap = true;
const Boolean kEraseFirst = true;
const Boolean kSpaceForCaret = true;
const Boolean kPreferOutline = true;

pascal void StdNoRect(GrafVerb verb,
					  const Rect* r);



class UGladUtilities
{
public:
	static void MATextBox(Ptr text,
						  long itsLength,
						  const Rect* box,
						  short itsJust,
						  Boolean autoWrap,
						  WordBreakUPP wordBreak,
						  Boolean eraseFirst,
						  Boolean spaceForCaret,
						  Boolean preferOutline);

	static void MADrawString(ConstStr255Param s,
							 const Rect* box,
							 short justification,
							 Boolean preferOutline);

	static SInt16 Int16Min(SInt16 inA,
						  SInt16 inB);
							
	static SInt32 Int32Min(SInt32 inA,
						  SInt32 inB);

	static Boolean IsColorPort(GrafPtr port);

	static short MAGetFontInfo(FontInfo* theFontInfo);

	static short GetActualJustification(short justification);

	static SignedByte LockHandleHigh(Handle h);

	static short SBUseResFile(short refNum);

	static void BlockSet(void* destPtr,
						 long byteCount,
						 unsigned char setVal);

	static void ZeroBlock(void* block,
						  long blockSize);

	// retrieves resources from the current applications resource fork
	static short AppCount1Resources(ResType rType);
	
	static short AppCountResources(ResType rType);
	
	static Handle AppGet1IndResource(ResType rType,
									 short index);
									 
	static Handle AppGet1NamedResource(ResType rType,
									   ConstStr255Param name);
										 
	static Handle AppGet1Resource(ResType rType,
								  short rID);
									
	static Handle AppGetIndResource(ResType rType,
									short index);
									
	static Handle AppGetNamedResource(ResType rType,
									  ConstStr255Param name);
										
	static Handle AppGetResource(ResType rType,
								 short rID);

	static void PathNameFromAlias(AliasHandle alias,
								  CMyStr255& pathName,
								  Boolean includeAliasName = false,
								  Boolean interactionAllowed = true);

	static void PathNameFromWD(long vRefNum,
							   CMyStr255& pathName);

	static void PathNameFromDirID(long DirID,
								  short vRefNum,
								  CMyStr255& pathName);

	static long StripLong(void* address);

	static OSErr ResolveAliasNoUI(const FSSpec* fromFile,
								  AliasHandle alias,
								  FSSpec* target,
								  Boolean* wasChanged);

	static void PullApplicationToFront();

	static OSErr RemvResourceByID(ResType theType,
								  short theID);

	static Ptr DisposeIfHandle(Ptr p);

	static Handle DisposeIfHandle(Handle h);


	static Boolean SameFile(const FSSpec* file1,
							const FSSpec* file2);

	static OSErr MakeCanonFSSpec(FSSpec* fSpec);
	
	static OSErr MakeNonCanonFSSpec(FSSpec* fSpec);

	static OSErr GetDeskFolderSpec(FSSpec* fSpec,
								   short vRefNum);

	static void GetIndDirectory(short index,
								short vRefNum,
								long dirID,
								MyFileFilterProcPtr fileFilter,
								FSSpec& fileSpec);

	static void GetIndFileSpec(short index,
							   short vRefNum,
							   long dirID,
							   MyFileFilterProcPtr fileFilter,
							   FSSpec& fileSpec);

	static short CountFiles(short vRefNum,
							long dirID,
							MyFileFilterProcPtr fileFilter);

	static OSErr GetFileInfo(const FSSpec& fileSpec,
							 HParamBlockRec* pb);

	static OSErr SetFileInfo(const FSSpec& fileSpec,
							 HParamBlockRec* pb);
							 
	static OSErr FSpGetPhysicalFileSize(const FSSpec& fileSpec,
												long *dataSize,
												long *rsrcSize);
							 
	static OSErr FSpUnlockFile(const FSSpec& fileSpec);

	static void ReadFileChunk(const FSSpec& fileSpec,
							  void* buffer,
							  long offset,
							  long* count,
							  short inDataRefNum = refNum_Undefined);

	static void WriteFileChunk(const FSSpec& fileSpec,
							   void* buffer,
							   long offset,
							   long* count,
							   short inDataRefNum = refNum_Undefined);

	static void CenterRect(Rect* ioRect,
						   const Rect& anchorRect,
						   IconAlignmentType align);

	static void SetStackSpace(long numBytes);

	static OSErr IncreaseApplicationStack(Size incrementSize);

	static Boolean IsThisKeyDown(const short theKey);
	
	static void ErrorAlert(OSErr inErr,
						   char* inErrMsg = nil);
						   
	static OSErr PlotIconID(const Rect& theRect,
							IconAlignmentType align,
							IconTransformType transform,
							short theResID,
							const RGBColor& backColor);
							
	static void Frame3DRect(Rect* inRect);

	static OSErr GenerateUniqueFileName(short volume,
										long dirID,
										Str63 ioFileName);
	static Boolean ConfirmAlert(char* inConfirmText);
	
	static void CopyPStrToCStr(ConstStr255Param inPStr,
									char* ioCStrBuffer,
									short bufferLen);

	static void GetProcessFSSpec(FSSpec* ioProcessFSSpec);
	
	static void LaunchApp(FSSpec& applFSSpec);
	
	static OSErr FindRunningProcess(OSType inProcessSig, OSType inProcessType,ProcessSerialNumber* ioProcessSN); 
	
	static void SendQuitAE(const ProcessSerialNumber& inTargetPSN,Boolean waitForIt = false);
	
	static OSType GetOurCreator();
	
	static OSErr GetFSSpecForCreator(FSSpec *spec,OSType creator,short vRefNum,long dirID);
	
	//Poses a dialog for 4 seconds or until a user keydown or mousedown
	//Takes a dialog ID and DITL indexes for color and B&W representations of whatever graphic
	//is used in the dialog (this allows us to show/hide based upon current pixel depth)
	static void PoseSplashWindow(short inDialogID,
								 short inColorSplashPictItemNum = -1,
								 short inBWSplashPictItemNum = -1);
								 	
	static void FindNewestFileByTypeAndCreator(short inVRefNum,  OSType inType,  OSType inCreator,  FSSpec* iofileSpec);
	
	static void FindFileByTypeAndCreator(short inVRefNum,  long inStartingDirID, OSType inType,  OSType inCreator,  FSSpec* iofileSpec);

	static Boolean FileExists( const FSSpec& inFileSpec );
	static Boolean FileHasResFork( const FSSpec& inFileSpec);
	static Boolean ProcessIsRunning(OSType inProcessSig, Boolean includeCurrentProcess = true);
	static void FSpMakeAlias(FSSpec *file_location,FSSpec *alias_location);
	static Boolean IsValidDBHeader(const DatabaseHdrType& inDbHdr,long inFileSize);
	
	static Boolean ReadFileLine( short fileRef, Str255 outString, UInt8 blockSize=254 );
	static UInt64 MilliSecondCount();

};



#endif //__UGladUtilities__