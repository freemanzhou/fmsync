/*
 *	TestConduitHeaders.h
 *
 *	Processor independant interface to the TestConduitHeaders<xxx> files ...
 */


#define _DEBUG
#define Debug_Throw
#define Debug_Signal

#ifdef __cplusplus

#if __POWERPC__
	#include <MacHeadersPPC++>
#elif __CFM68K__
	#include <MacHeadersCFM68K++>
#else
	#include <MacHeaders68K++>
#endif

#else

#if __POWERPC__
	#include <MacHeadersPPC>
#elif __CFM68K__
	#include <MacHeadersCFM68K>
#else
	#include <MacHeaders68K>
#endif

#endif

#include "GladPortDefs.h"
