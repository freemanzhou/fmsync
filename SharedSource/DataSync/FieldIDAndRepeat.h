#pragma once

#include "FMAE.h"

class FieldIDAndRepeat {
public:
	explicit FieldIDAndRepeat(const FMAE::FieldID& fieldID, int repeatIndex = 0);
	
	friend bool operator < (const FieldIDAndRepeat&, const FieldIDAndRepeat&); 
	friend bool operator == (const FieldIDAndRepeat&, const FieldIDAndRepeat&);
	
	FMAE::FieldID GetFieldID() const {return fFieldID;}
	int GetRepeatIndex() const {return fRepeatIndex;}

private:
	FMAE::FieldID fFieldID;
	int fRepeatIndex;
};

namespace DebugOutput {
	void DoOutput(const FieldIDAndRepeat& item);
}
