#include "JFile1.h"
#include "WritePDB.h"
#include "Writer.h"
#include "CDatabase.h"

CWriter::CWriter(CDataSource* sourceData, Boolean translateText)
	: fDatabase(sourceData), fTranslateText(translateText)
{
}

CWriter::~CWriter()
{
}

void
CWriter::Write(LStream* targetStream, ConstStringPtr name)
{
	try {
		Handle h;
		OSErr err;
		h = TempNewHandle(0, &err);
		ThrowIfOSErr_(err);
		ThrowIfNil_(h);
		LHandleStream hStream(h);
		DoWrite(&hStream, name);
		int handleLength = hStream.GetLength();
		h = hStream.GetDataHandle();
		HLock(h);
		targetStream->WriteBlock(*h, handleLength);
		return;
	} catch (...) {
	}
	DoWrite(targetStream, name);
}
