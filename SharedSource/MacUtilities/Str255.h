#pragma once
#include <string>
#include <Types.h>

class LString;

class Str255Converter {
  public:
	Str255Converter(const std::string&);
	operator const unsigned char*() const;
  private:
	Str255 mStr255;
};

std::string AsString(const LString&);
std::string AsString(ConstStr255Param);
Str255Converter AsStr255(const std::string&);
void CopyToStr255(const std::string&, Str255);
std::string LoadString(ResIDT resourceID, int index);
std::string LoadTextResource(ResIDT resourceID);

Handle AsHandle(const std::string&);
Handle AsStringHandle(const std::string& s);
