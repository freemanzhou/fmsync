// ===========================================================================
//	PP_DebugHeaders.h				©1996 Metrowerks Inc. All rights reserved.
// ===========================================================================

	// Use PowerPlant-specific Precompiled header
	
#if __POWERPC__
	#include "FMConduitBetaHeadersPPC"
	
#elif __CFM68K__
	#include "FMConduitBetaHeadersCFM68K"
	
#else
	#include "FMConduitBetaHeaders68K"
#endif
