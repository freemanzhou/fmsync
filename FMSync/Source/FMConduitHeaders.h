// ===========================================================================
//	PP_DebugHeaders.h				©1996 Metrowerks Inc. All rights reserved.
// ===========================================================================

	// Use PowerPlant-specific Precompiled header
	
#if __POWERPC__
	#include "FMConduitHeadersPPC"
	
#elif __CFM68K__
	#include "FMConduitHeadersCFM68K"
	
#else
	#include "FMConduitHeaders68K"
#endif
