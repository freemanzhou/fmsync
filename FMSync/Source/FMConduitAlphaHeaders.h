#if __POWERPC__
	#include "FMConduitAlphaHeadersPPC"
	
#elif __CFM68K__
	#include "FMConduitAlphaHeadersCFM68K"
	
#else
	#include "FMConduitAlphaHeaders68K"
#endif
