// ===========================================================================
//	FMConduitDemoHeaders.h				©1996 Metrowerks Inc. All rights reserved.
// ===========================================================================

	// Use PowerPlant-specific Precompiled header
	
#if __POWERPC__
	#include "FMConduitDemoHeadersPPC"
	
#elif __CFM68K__
	#include "FMConduitDemoHeadersCFM68K"
	
#else
	#include "FMConduitDemoHeaders68K"
#endif
