#pragma once

namespace Version {

typedef struct {
	UInt8	majorVersion;
	UInt8	minorVersion;
	UInt8	releaseStage;
	UInt8	nonFinalRelease;
	UInt16	regionCode;
	UInt8	string1[1];
} VersionRecord, *VersPtr;

enum {
	development = 0x20,
	alpha = 0x40,
	beta = 0x60,
	final = 0x80
}; 

std::string VersionAsString(VersPtr versPtr, bool useString = false);
std::string VersionAsString(Handle h);
std::string VersionAsString(unsigned long version);
std::string FileVersionAsString(const FSSpec& f);
std::string CurrentResourceFileVersionAsString();
bool FileVersion(const FSSpec& f, VersionRecord&);

}