#pragma once

#include <LPeriodical.h>

class CSyncIdler : public LPeriodical
{
public:
					CSyncIdler();
	virtual			~CSyncIdler();
	
	virtual	void	SpendTime(
							const EventRecord&		inMacEvent);
	
	void			ResetTimeout();
	
private:

	unsigned long	fNextIdleTick;
	unsigned long	fTimeoutTick;
};

