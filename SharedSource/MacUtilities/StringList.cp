#include "StringList.h"
#include "Stringiness.h"
#include "Str255.h"

CStringList::CStringList(ResIDT resID)
{
	int index = 1;
	Str255 theString;
	
	do {
		GetIndString(theString, resID, index);
		if (theString[0] > 0) {
			push_back(AsString(theString));
		}
		++index;
	} while(theString[0] > 0);
}
