#ifdef CONDUIT
#include "UDebugOut.h"
#endif

#include "DebugOutput.h"

namespace DebugOutput {

void DoOutput(const char* item)
{
#ifdef CONDUIT
	DEBUG_OUT_DEF(item);
#endif
}

void DoOutput(const string& item)
{
	DoOutput(item.c_str());
}

void DoOutput(const int& item)
{
	if (item >= 'AAAA') {
		string output("'");
		output += CharsAsString(item).c_str();
		output += "'";
		DoOutput(output.c_str());
	} else {
		DoOutput(AsString(item).c_str());
	}
}

}
