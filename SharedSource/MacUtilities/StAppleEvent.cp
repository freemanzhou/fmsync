#include <LEventDispatcher.h>

#include "StAppleEvent.h"
#include "CAEDescriptor.h"
#include "Stringiness.h"
#include "Utilities.h"
#include "DebugOutput.h"

#ifdef CONDUIT
#include "CConduitDialog.h"
#include "ConduitProgressCallback.h"
#endif

static int gCallCount;

string StAppleEvent::gLastErrorString;	

StAppleEvent::StAppleEvent(AEEventClass theAEEventClass, AEEventID theAEEventID,
	 			const AEAddressDesc *target, short returnID, long transactionID)
{
	mEvent.descriptorType = typeNull;
	mEvent.dataHandle = nil;
	ThrowIfOSErr_(AECreateAppleEvent(theAEEventClass, theAEEventID, target,
	                            returnID, transactionID, &mEvent));
}

StAppleEvent::StAppleEvent()
{
	mEvent.descriptorType = typeNull;
	mEvent.dataHandle = nil;
}

StAppleEvent::~StAppleEvent()
{
	AEDisposeDesc(&mEvent);
}

static Boolean firstTime = true;
#if defined( PALM_BUDDY_PLUGIN)
static pascal Boolean defaultIdleProc(EventRecord *, long *, RgnHandle *)
{
	return false;
}
#elif defined(CONDUIT)
#include "syncmgr.h"
static pascal Boolean defaultIdleProc(EventRecord *eventRecord, long *sleepTime, RgnHandle *mouseRegion)
{
	volatile Boolean quitResult = false;
	if (firstTime) {
		*sleepTime = 0;
		*mouseRegion = 0;
		firstTime = 0;
	}
	try {
		CallConduitProgress(0);
	} catch (...) {
		quitResult = true;
	}
	return quitResult;
}
#else
static pascal Boolean defaultIdleProc(EventRecord *eventRecord, long *sleepTime, RgnHandle *mouseRegion)
{
	if (firstTime) {
		*sleepTime = 30;
		*mouseRegion = 0;
		firstTime = 0;
	}
	LEventDispatcher *eventD = LEventDispatcher::GetCurrentEventDispatcher();
	if (eventD != 0) {
		eventD->DispatchEvent(*eventRecord);
	}
	return false;
}
#endif

static AEIdleUPP	gOurDefaultIdleProc = NewAEIdleUPP(defaultIdleProc);
static AEIdleUPP	gDefaultIdleProc = gOurDefaultIdleProc;


void
StAppleEvent::Send(AppleEvent *reply, AESendMode sendMode, AESendPriority sendPriority, long timeOutInTicks, 
		AEIdleUPP idleProc, AEFilterUPP filterProc)
{	
	ConfirmFreeStackSpace(); 
	firstTime = true;
	StGrafPortSaver saver;
	OSErr err = AESend(&mEvent, reply, sendMode, sendPriority, 
			timeOutInTicks, idleProc?idleProc:gDefaultIdleProc, filterProc);
	if (err != noErr) {
		DumpEvent();
		Throw_(err);
	}
	DescType typeCode;
	long actualSize;
	long errorCode;
	if (reply) {
		OSErr anErr = AEGetParamPtr(reply, keyErrorNumber, typeSInt32, &typeCode, &errorCode, sizeof(errorCode), &actualSize);
		if (anErr == noErr && errorCode != noErr) {
			DumpEvent();
			char buffer[1024];
			OSErr anErr = AEGetParamPtr(reply, keyErrorString, typeChar, &typeCode, buffer, 1024, &actualSize);
			if (anErr == noErr)
				gLastErrorString = string(buffer, actualSize);
			else
				gLastErrorString = AsString(errorCode);
			Throw_(errorCode);
		}
	}
}

AEIdleUPP
StAppleEvent::GetDefaultIdleProc()
{
	return gDefaultIdleProc;
}

AEIdleUPP
StAppleEvent::SetDefaultIdleProc(AEIdleUPP idleProc)
{
	AEIdleUPP ip = gDefaultIdleProc;
	gDefaultIdleProc = idleProc;
	return ip;
}

void
StAppleEvent::DumpEvent()
{
	DumpDescriptor('aevt', &mEvent);
}

void
StAppleEvent::DumpDescriptor(AEKeyword keyword, const AEDesc* descriptor)
{
	string keyWordString((char*)&keyword, 4);
	string theString("Keyword: ");
	theString += keyWordString;
	theString += '\r';
	DebugOutput::Output(theString);
	long theCount;
	AECountItems(descriptor, &theCount);
	if (theCount > 0)
		DebugOutput::Output("-->contains");
	for (long index = 1; index <= theCount; index += 1) {
		CAEDescriptor theDescriptor;
		OSType desiredType = typeWildCard;
		AEKeyword theAEKeyword;
		AEGetNthDesc(descriptor, index, desiredType, &theAEKeyword, theDescriptor);
		DumpDescriptor(theAEKeyword, theDescriptor);
	}
}

void
StAppleEvent::BringToFront(const AEAddressDesc* thisApp)
{
	StAppleEvent activateEvent(kCoreEventClass, kAEActivate, thisApp);
	activateEvent.Send(0, kAENoReply | kAECanInteract | kAEDontRecord);
}

