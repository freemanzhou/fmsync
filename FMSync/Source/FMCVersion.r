#include "Carbon.r"
#include "MacTypes.r"
#include "VersionDefinition.h"

#if defined(BETA_RELEASE)
resource 'vers' (1) {
	0x2,
	0x0,
	beta,
	0x4,
	0,
	"2.0b4",
	"2.0b4, © 1998-2001, FMSync Software"
};

resource 'vers' (2) {
	0x2,
	0x0,
	beta,
	0x4,
	0,
	"2.0b4",
	"FMSync for JFile 5 2.0b4"
};

#elif defined (ALPHA_RELEASE)
resource 'vers' (1) {
	0x2,
	0x0,
	alpha,
	2,
	0,
	"2.0a3",
	"2.0a3, © 1998-2001, FMSync Software"
};

resource 'vers' (2) {
	0x2,
	0x0,
	alpha,
	3,
	0,
	"2.0a3",
	"FMSync for JFile 2.0a3"
};
#elif defined (DEMO_RELEASE)
resource 'vers' (1) {
	0x1,
	0x15,
	final,
	0,
	0,
	"1.1.5 Demo",
	"1.1.5 Demo, © 1998-2000, FMSync Software"
};

resource 'vers' (2) {
	0x1,
	0x15,
	final,
	0,
	0,
	"1.1.5 Demo",
	"FMSync for JFile 5 2.0 Demo"
};
#else
resource 'vers' (1) {
	0x2,
	0x0,
	development,
	4,
	0,
	"2.0d4",
	"2.0d4, © 1998-2001, FMSync Software"
};

resource 'vers' (2) {
	0x2,
	0x0,
	development,
	4,
	0,
	"2.0d4",
	"FMSync for JFile 2.0d4"
};
#endif

