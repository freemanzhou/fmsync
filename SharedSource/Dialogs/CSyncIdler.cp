#include "CSyncIdler.h"
#include "SyncMgr.h"


CSyncIdler::CSyncIdler()
	: fNextIdleTick(0)
{
	ResetTimeout();
}

CSyncIdler::~CSyncIdler()
{
}

void
CSyncIdler::SpendTime(const EventRecord&)
{
	unsigned long ticksNow = TickCount();
	if (ticksNow > fNextIdleTick && ticksNow < fTimeoutTick) {
		fNextIdleTick = TickCount() + 20*60;
		long dateTime;
		SyncReadSysDateTime(dateTime);
	}
}

void
CSyncIdler::ResetTimeout()
{
	fTimeoutTick = TickCount() + 60*120;
}
