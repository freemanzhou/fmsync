#include "Stringiness.h"
#include "Str255.h"
#include "VersionUtilities.h"

namespace Version {

std::string VersionAsString(unsigned long version)
{
	return VersionAsString((VersPtr)&version);
}

std::string VersionAsString(VersPtr versPtr, bool useString)
{
	if (useString) {
		return AsString(versPtr->string1);
	}
	
	std::string versionString(AsString(versPtr->majorVersion));
	versionString += '.';
	long minorVersion = versPtr->minorVersion >> 4;
	long pointVersion = versPtr->minorVersion & 0x0f;
	versionString += AsString(minorVersion);
	if (pointVersion) {
		versionString += '.';
		versionString += AsString(pointVersion);
	}
	if (versPtr->releaseStage != final) {
		char releaseChar = 0;
		switch(versPtr->releaseStage) {
		case alpha:
			releaseChar = 'a';
			break;
		case beta:
			releaseChar = 'b';
			break;
		case development:
			releaseChar = 'd';
			break;
		}
		if (releaseChar)
			versionString += releaseChar;
		versionString += AsString(versPtr->nonFinalRelease);
	}
	return versionString;

}

std::string VersionAsString(Handle h)
{
	StHandleLocker locker(h);
	VersPtr versPtr = (VersPtr)*h;
	return VersionAsString(versPtr, GetHandleSize(h) > sizeof(VersionRecord));
}

std::string FileVersionAsString(const FSSpec& f)
{
	string s("Version resource not found.");
	LFile file(f);
	file.OpenResourceFork(fsRdPerm);
	return CurrentResourceFileVersionAsString();
}

std::string CurrentResourceFileVersionAsString()
{
	string s("Version resource not found.");
	StResource r('vers', 1);
	Handle h = r;
	if (h) {
		s = VersionAsString(h);
	}
	return s;
}

bool FileVersion(const FSSpec& f, VersionRecord& vr)
{
	LFile file(f);
	file.OpenResourceFork(fsRdPerm);
	StResource r('vers', 1);
	Handle h = r.Get();
	if (h && *h) {
		vr = **(VersionRecord**)h;
		return true;
	}
	return false;
}

}