#include "DebugOutput.h"
#include "FieldIDAndRepeat.h"

FieldIDAndRepeat::FieldIDAndRepeat(const FMAE::FieldID& fieldID, int repeatIndex)
	: fFieldID(fieldID), fRepeatIndex(repeatIndex)
{
}

bool operator < (const FieldIDAndRepeat& a, const FieldIDAndRepeat& b)
{
	if (a.fFieldID < b.fFieldID)
		return true;
	return a.fRepeatIndex < b.fRepeatIndex;
}

bool operator == (const FieldIDAndRepeat& a, const FieldIDAndRepeat& b)
{
	return (a.fFieldID == b.fFieldID) && (a.fRepeatIndex == b.fRepeatIndex);
}
	

namespace DebugOutput {

void DoOutput(const FieldIDAndRepeat& item)
{
	Output("fFieldID", item.GetFieldID());
	Output("fRepeatIndex", item.GetRepeatIndex());
}

}
