#include "charset.h"

class CConversionTable {
public:
			CConversionTable(const char *from, const char *to);
			
	void	Convert(string& inString);
	void	Convert(char *stringPtr);
	void	Convert(StringPtr stringPtr);

char		fTable[256];
};

CConversionTable::CConversionTable(const char *from, const char *to)
{
	for (int i = 0; i < 256; i += 1) {
		fTable[i] = i;
	}
	unsigned char c;
	while ((c = *from++) != 0) {
		fTable[c] = *to++;
	}
}

void
CConversionTable::Convert(string& inString)
{
	string::iterator p = inString.begin();
	string::const_iterator endPtr = inString.end();
	
	while (p != endPtr) {
		unsigned char c = *p;
		*p = fTable[c];
		++p;
	}
}

void
CConversionTable::Convert(char *stringPtr)
{
	int count = ::strlen(stringPtr);
	
	while (count > 0) {
		count -= 1;
		unsigned char c = *stringPtr;
		*stringPtr++ = fTable[c];
	}
}

void
CConversionTable::Convert(StringPtr stringPtr)
{
	int count = *stringPtr++;
	
	while (count > 0) {
		count -= 1;
		unsigned char c = *stringPtr;
		*stringPtr++ = fTable[c];
	}
}

#if 0
const char *
gPilotSpecials   =              "áàäâåãæçñøéèëêíìïîßýÿóòöôõúùüû¿«£¥ÁÀÄÂÅÃÆÇÑØÉÈËÊÍÌÏÎÝŸÓÒÖÔÕÚÙÜÛ¡»‘’“”·x÷øƒ§µß©™®¢ •–—";
const char * gDesktopSpecials = "‡ˆŠ‰Œ‹¾–¿Ž‘’“•”§þØ—˜š™›œŸžÀÇ£´çË€åÌ®‚„¯ƒéèæêíìëßÙîñ…ïÍòô†óÁÈÔÕÒÓáxÖ¿Ä¤µ§©ª¨¢Ê¥ÐÑ";
#endif

const char * gPilotSpecials 	= "€‚ƒ„…†‡ˆ‰Š‹ŒŽ‘’“”•–—˜™š›œžŸ ¡¢£¤¥¦§¨©ª«¬­®¯°±²³´µ¶·¸¹º»¼½¾¿ÀÁÂÃÄÅÆÇÈÉÊËÌÍÎÏÐÑÒÓÔÕÖ×ØÙÚÛÜÝÞßàáâãäåæçèéêëìíîïðñòóôõö÷øùúûüýþ";
const char * gDesktopSpecials   = "Û²âÄãÉ àöä³ÜÎ×·¸¹ÔÕÒÓ¥ÐÑ÷ªºÝÏùúÙÊÁ¢£°´õ¤¬©»ÇÂû¨ø¡±½Ã«µ¦áü­¼ÈÅÆýÀËçåÌ€®‚éƒæèíêëìÚ„ñîïÍ…Þ¯ôòó†ßð§ˆ‡‰‹ŠŒ¾Ž‘“’”•¶–˜—™›šÖ¿œžŸþÿ";

void
ConvertToPilotText(string& inputString)
{
	static CConversionTable table(gDesktopSpecials, gPilotSpecials);
	table.Convert(inputString);
}

void
ConvertToDesktopText(string& inputString)
{
	static CConversionTable table(gPilotSpecials, gDesktopSpecials);
	table.Convert(inputString);
}

void
ConvertToPilotText(char *stringPtr)
{
	static CConversionTable table(gDesktopSpecials, gPilotSpecials);
	table.Convert(stringPtr);
}

void
ConvertToDesktopText(char *stringPtr)
{
	static CConversionTable table(gPilotSpecials, gDesktopSpecials);
	table.Convert(stringPtr);
}

void ConvertToPilotText(StringPtr inputString)
{
	static CConversionTable table(gDesktopSpecials, gPilotSpecials);
	table.Convert(inputString);
}

void ConvertToDesktopText(StringPtr inputString)
{
	static CConversionTable table(gPilotSpecials, gDesktopSpecials);
	table.Convert(inputString);
}

void
ConvertToDesktopReturns(string& inString)
{
	int count = inString.length();
	
	while (count > 0) {
		count -= 1;
		unsigned char c = inString[count];
		if (c == 10)
			inString[count] = 13;
	}
}

void
ConvertToPilotReturns(string& inString)
{
	int count = inString.length();
	
	while (count > 0) {
		count -= 1;
		unsigned char c = inString[count];
		if (c == 13)
			inString[count] = 10;
	}
}

void
ConvertToVerticalTab(string& inString)
{
	int count = inString.length();
	
	while (count > 0) {
		count -= 1;
		unsigned char c = inString[count];
		if (c == 13 || c == 10)
			inString[count] = verticalTab;
	}
}

void
ConvertVerticalTabToPilotReturn(string& inString)
{
	int count = inString.length();
	
	while (count > 0) {
		count -= 1;
		unsigned char c = inString[count];
		if (c == verticalTab)
			inString[count] = 10;
	}
}

void
ConvertVerticalTabToMacReturn(string& inString)
{
	int count = inString.length();
	
	while (count > 0) {
		count -= 1;
		unsigned char c = inString[count];
		if (c == verticalTab)
			inString[count] = returnCharacter;
	}
}

string ConvertToPilotTextAndReturns(const string& inputString)
{
	string s(inputString);
	ConvertToPilotReturns(s);
	ConvertToPilotText(s);
	return s;
}

string ConvertToDesktopTextAndReturns(const string& inputString)
{
	string s(inputString);
	ConvertToDesktopReturns(s);
	ConvertToDesktopText(s);
	return s;
}

