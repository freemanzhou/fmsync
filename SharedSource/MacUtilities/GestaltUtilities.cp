#include "GestaltUtilities.h"

unsigned long GetQuickTimeVersion()
{
	long			response;

	if (Gestalt(gestaltQuickTimeVersion, &response) != noErr)
		return 0;
	return response;
}
