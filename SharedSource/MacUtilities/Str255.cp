#include "Str255.h"

#include <LString.h>

std::string
AsString(ConstStr255Param in)
{
	return std::string(reinterpret_cast<const char*>(&in[1]), in[0]);
}

std::string
AsString(const LString& in)
{
	ConstStr255Param str255(in);
	return AsString(str255);
}

void
CopyToStr255(const std::string& in, Str255 out)
{
	unsigned size = in.size();
	if (size > 255)
		size = 255;
	out[0] = size;
	in.copy(reinterpret_cast<char *>(&out[1]), size);
}

Str255Converter
AsStr255(const std::string& in)
{
	return in;
}

Str255Converter::Str255Converter(const std::string& in)
{
	CopyToStr255(in, mStr255);
}

Str255Converter::operator const unsigned char*() const
{
	return mStr255;
}

std::string
LoadString(ResIDT resourceID, int index)
{
	Str255 theString;
	GetIndString(theString, resourceID, index);
	ThrowIfResError_();
	return AsString(theString);
}

std::string
LoadTextResource(ResIDT resourceID)
{	
	StResource r('TEXT', resourceID);
	HLock(r);
	return std::string(*Handle(r), GetHandleSize(r));
}

Handle AsHandle(const std::string& s)
{
	StHandleBlock h(s.length());
	::BlockMoveData(s.data(), *h.Get(), s.length());
	return h.Release();
}


Handle AsStringHandle(const std::string& s)
{
	StHandleBlock h(s.length()+1);
	Str255 theString;
	LString::CopyPStr(AsStr255(s), theString);
	::BlockMoveData(theString, *h.Get(), theString[0]+1);
	return h.Release();
}
