// ===========================================================================
//	PP_DebugHeaders.h				©1996 Metrowerks Inc. All rights reserved.
// ===========================================================================

	// Use PowerPlant-specific Precompiled header
	
#if __POWERPC__
	#include "FMConduitProfHeadersPPC"
	
#elif __CFM68K__
	#include "FMConduitProfHeadersCFM68K"
	
#else
	#include "FMConduitProfHeaders68K"
#endif
