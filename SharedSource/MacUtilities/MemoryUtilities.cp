#include "MemoryUtilities.h"

const int kMinFreeStackSpace = 4096;
void ConfirmFreeStackSpace()
{
#ifdef CONFIRM_STACK_SPACE
#ifdef CONDUIT
	unsigned long stackFreeSpace;
	ThreadCurrentStackSpace(kCurrentThreadID, &stackFreeSpace);
	if (stackFreeSpace < kMinFreeStackSpace)
		Throw_(insufficientStackErr); 
#if LOG_STACK_SPACE
		DebugOutput::Output( "free stack space = " );
		DebugOutput::Output( AsString((int)stackFreeSpace).c_str());
		DebugOutput::Output( "\r" );
#endif
#endif
#endif
}
