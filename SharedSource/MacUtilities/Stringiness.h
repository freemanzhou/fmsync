#pragma once
#include <cctype>
#include <vector>
#include <string>
#ifdef macintosh
#include <MacMemory.h>
#endif

const std::string kEmptyString;

std::string AsString(short);
std::string AsString(unsigned short);
std::string AsString(int);
std::string AsString(unsigned);
std::string AsString(long);
std::string AsString(unsigned long);
	// simple integer
std::string BytesAsString(unsigned long);
	// 1K, 2 MB, etc.
std::string LottaBytesAsString(SInt64 number);
std::string HexAsString(unsigned long, int n = 8);
	// n-digit, 0-filled, uppercase hex
std::string CharsAsString(unsigned long integer);
	// four character constant

std::string AsString(double);
	// floating point

std::string DateAsString(unsigned long integer);
	// four character constant
std::string TimeAsString(unsigned long integer);
	// four character constant
#ifdef macintosh

std::string AsString(Handle);
	// Put a whole handle into a string.
std::string ConvertToString(Handle);
	// Destroys handle.

#endif // macintosh

void Substitute(std::string&, const std::string& oldString, const std::string& newString);
std::vector<std::string> SplitString(const std::string&, const std::string& delim);
std::string JoinString(const std::vector<std::string>&, const std::string& delim);
std::vector<std::string> Tokenize(const std::string&);
std::string TrimString(const std::string&);
std::string ToLowerCase(const std::string&);

int cmp_nocase(const std::string &s, const std::string& s2);

class TrimStringOp {
public:
	void operator()(std::string& s) {s = TrimString(s);}
};


class ToLowerOp {
public:
	char operator()(const char s) {return std::tolower(s);}
};


class NullStringOp {
public:
	bool operator()(const std::string& s) {return s.length() == 0;}
};
