#pragma once

#include <string>

class	StAppleEvent {
public:
	static AEIdleUPP SetDefaultIdleProc(AEIdleUPP idleProc);
	static AEIdleUPP GetDefaultIdleProc();
	static std::string gLastErrorString;	
	AppleEvent	mEvent;

			//	These operators sometimes let you appear to pass
			//	the StAppleEvent object rather than having to pass the
			//	object.mEvent.
			operator	AppleEvent*() { return &mEvent; }
			operator	AppleEvent&() { return mEvent; }
	
			StAppleEvent(AEEventClass theAEEventClass, AEEventID theAEEventID,
	 			const AEAddressDesc *target, short returnID = kAutoGenerateReturnID, 
	 			long transactionID = kAnyTransactionID);
			StAppleEvent();
			~StAppleEvent();
			
	void	Send(AppleEvent *reply = 0, AESendMode sendMode = kAENoReply | kAENeverInteract | kAEDontRecord, 
					AESendPriority sendPriority = kAEHighPriority, 
					long timeOutInTicks = 180*60, AEIdleUPP idleProc = 0, AEFilterUPP filterProc = 0);

	void	DumpEvent();
	void	DumpDescriptor(AEKeyword keyword, const AEDesc* descriptor);
	
	static void BringToFront(const AEAddressDesc* thisApp);
};
