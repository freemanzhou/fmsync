#include "GladPortDefs.h"

#include "syncmgr.h"



class CJFileConduit
{
public:

	static void CheckCanRun();

	static void WasteTime();
	
	static Boolean IsThisKeyDown(const short theKey);

	static long OpenConduit(PROGRESSFN inProgressCallBack, CSyncProperties& inSyncProperties);

	static long GetConduitName(char* ioConduitName, WORD inStrLen);

	static DWORD GetConduitVersion();
	
	static long ConfigureConduit(CSyncPreference& inSyncProperties);

};

void CallConduitProgress(const char *progressString);
void SetConduitProgressCallback(PROGRESSFN proc);

#pragma export on

extern "C" {
	
	long OpenConduit(PROGRESSFN inProgressCallBack, CSyncProperties& inSyncProperties);

	long GetConduitName(char* ioConduitName, WORD inStrLen);

	DWORD GetConduitVersion();
	
	long ConfigureConduit(CSyncPreference& inSyncProperties);

}

#pragma export off