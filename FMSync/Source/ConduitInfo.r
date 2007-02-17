
#include "Types.r"
#include "ConduitTypes.r"


//Conduit Info
resource 'CInf' (0)
{
	2,
	0x0300,						// the version number for this conduit
	'JFi5',						// Conduit "Creator" ID
	"FMSync for JFile X",		// user visible conduit name
	"FMSyncX",				// settings directory
	defaultPriority,			// The conduit's priority
	wantsUserInterface,			//  Does the conduit have any UI?
	{
	},
	{
	},							//  Local database name
};
