#include "Stringiness.h"
#include "Str255.h"

#include <sstream>
#ifdef macintosh
#include <UMemoryMgr.h>
#endif

using std::string;
using std::ostringstream;
using std::isspace;
using std::vector;
using std::toupper;

const unsigned long kOneMegabyte = 1024 * 1024;

string
BytesAsString(unsigned long number)
{
	if (number == 0)
		return "zero K";
	if (number < kOneMegabyte)
		return AsString((number + 512) / 1024) + "K";
	
	unsigned long megabytes = number / kOneMegabyte;
	unsigned long remainder = number - megabytes * kOneMegabyte;
	unsigned long tenths = (remainder * 10 + (kOneMegabyte / 2)) / kOneMegabyte;
	if (tenths == 0)
		return AsString(megabytes) + " MB";

	return AsString(megabytes) + "." + AsString(tenths) + " MB";
}

string LottaBytesAsString(SInt64 number)
{
	const SInt64 kOneGigabyte = kOneMegabyte * 1024;

	if (number < kOneGigabyte) {
		return BytesAsString((unsigned long)number);
	}

	unsigned long gigabytes = number / kOneGigabyte;
	unsigned long remainder = number - gigabytes * kOneGigabyte;
	unsigned long tenths = (remainder * 10 + (kOneGigabyte / 2)) / kOneGigabyte;
	if (tenths == 0)
		return AsString(gigabytes) + " GB";

	return AsString(gigabytes) + "." + AsString(tenths) + " GB";
}

string
AsString(short number)
{
	ostringstream out;
	out << number;
	return out.str();
}

string
AsString(unsigned short number)
{
	ostringstream out;
	out << number;
	return out.str();
}

string
AsString(int number)
{
	ostringstream out;
	out << number;
	return out.str();
}

string
AsString(unsigned number)
{
	ostringstream out;
	out << number;
	return out.str();
}

string
AsString(long number)
{
	ostringstream out;
	out << number;
	return out.str();
}

string
AsString(unsigned long number)
{
	ostringstream out;
	out << number;
	return out.str();
}

string
AsString(double number)
{
	ostringstream out;
	out << number;
	return out.str();
}

string
HexAsString(unsigned long integer, int n)
{
	ostringstream result;
	result.width(n);
	result.fill('0');
	result << std::uppercase << std::hex << integer;
	return result.str();
}

string
CharsAsString(unsigned long integer)
{
	return string((char*)&integer, 4);
}

string DateAsString(unsigned long integer)
{
	Str255 dateString;
	DateString(integer, shortDate, dateString, 0);
	return AsString(dateString);
}

string TimeAsString(unsigned long integer)
{
	Str255 timeString;
	TimeString(integer, false, timeString, 0);
	return AsString(timeString);
}

void
Substitute(string& inString, const string& oldString, const string& newString)
{
	if (oldString == newString)
		return;
	
	std::size_t found;
	do {
		found = inString.find(oldString);
		if (found != string::npos)
			inString.replace(found, oldString.size(), newString);
	} while (found != string::npos);
}

std::vector<string>
SplitString(const string& s, const string& delim)
{
	std::vector<string> v;

	std::size_t found = 0;
	std::size_t start = 0;
	std::size_t delim_length = delim.length();
	do {
		found = s.find(delim, start);
		if (found != string::npos) {
			v.push_back(string(&s[start], &s[found] - &s[start]));
			start = found + delim_length;
		}
	} while (found != string::npos);

	if (start < s.length()) {
		v.push_back(string(&s[start], &s[s.length()] - &s[start]));
	}

	return v;
}

string
JoinString(const std::vector<string>& v, const string& delim)
{
	string joinedString;
	for (std::vector<string>::const_iterator i = v.begin(); i != v.end(); i++) {
		if (i != v.begin())
			joinedString.append(delim);
		joinedString.append(*i);
	}
	return joinedString;
}

class IsNotWhitespaceOp {
public:
	bool operator()(char c) {
		return isspace(c);
	}
};

string TrimString(const string& inString)
{
	if (inString.length() == 0)
		return inString;
	
	string::size_type indexFirst = inString.find_first_not_of(" \t");
	if (indexFirst == string::npos)
		return "";

	string::size_type indexLast = inString.find_last_not_of(" \t");
	if (indexLast == string::npos)
		indexLast = inString.length();
	else
		indexLast+=1;
	
	return inString.substr(indexFirst, indexLast - indexFirst);
}

vector<string> Tokenize(const string& inString)
{
	vector<string> r(SplitString(inString, " "));
	for_each(r.begin(), r.end(), TrimStringOp());
	vector<string>::iterator newEnd = remove_if(r.begin(), r.end(), NullStringOp());
	r.erase(newEnd, r.end());
	return r;
}

std::string ToLowerCase(const std::string& s)
{
	string r;
	transform(s.begin(), s.end(), back_inserter(r), ToLowerOp());
	return r;
}

int cmp_nocase(const string &s, const string& s2)
{
	string::const_iterator p = s.begin();
	string::const_iterator p2 = s2.begin();
	
	while (p != s.end() && p2 != s2.end()) {
		if (toupper(*p) != toupper(*p2))
			return (toupper(*p) < toupper(*p2)) ? -1: 1;
		++p;
		++p2;
	}
	return s2.size() - s.size();
}

#ifdef macintosh

string
AsString(Handle handle)
{
	MoveHHi(handle);
	StHandleLocker lock(handle);
	return string(*handle, GetHandleSize(handle));
}

class HandleDestroyer {
  public:
	HandleDestroyer(Handle handle) { mHandle = handle; }
	~HandleDestroyer();
	operator Handle() const { return mHandle; }
  private:
	Handle mHandle;
	HandleDestroyer(const HandleDestroyer&);
	HandleDestroyer& operator =(const HandleDestroyer&);
};

HandleDestroyer::~HandleDestroyer()
{
	DetachResource(mHandle);
	DisposeHandle(mHandle);
}

string
ConvertToString(Handle handle)
{
	return AsString(HandleDestroyer(handle));
}

#endif // macintosh
