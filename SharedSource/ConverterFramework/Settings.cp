#include <cstdlib>

#include "Settings.h"
#include "MemoryUtilities.h"
#include "Stringiness.h"
#include "BinaryFormat.h"

CSettings::CSettings()
{
}

CSettings::~CSettings()
{
}

CSettings&
CSettings::GetSettings()
{
	static CSettings gSettings;
	return gSettings;
}

void
CSettings::LoadSettings(LStream& inStream)
{
	long mapCount;
	inStream >> mapCount;
	for (int i = 0; i < mapCount; i += 1) {
		string key;
		BinaryFormat::Read(inStream, key);
		string valueString;
		BinaryFormat::Read(inStream, valueString);
		fSettings[key] = valueString;
	}

	fDirty = false;
}

void
CSettings::WriteSettings(LStream& inStream)
{
	long mapSize = fSettings.size();
	inStream << mapSize;
	
	fDirty = false;
	
	if (mapSize == 0)
		return;
		
	map<string, string>::iterator i = fSettings.begin();
	while (i != fSettings.end()) {
		BinaryFormat::Write(inStream, (*i).first);
		BinaryFormat::Write(inStream, (*i).second);
		i++;
	}
}

Boolean
CSettings::GetSetting(const string& keyValue, string& settingValue)
{
	map<string, string>::const_iterator i = fSettings.find(keyValue);
	if (i == fSettings.end()) {
		settingValue = "";
		return false;
	}
	settingValue = i->second;
	return true;
}

void
CSettings::SetSetting(const string& keyValue, const string& settingValue)
{
	if (fSettings.find(keyValue) == fSettings.end() 
		|| fSettings[keyValue] != settingValue) {
		fDirty = true;
		fSettings[keyValue] = settingValue;
	}
}

Boolean
CSettings::GetSetting(const string& keyValue, int& settingValue)
{
	string settingAsString;
	Boolean result = GetSetting(keyValue, settingAsString);
	settingValue = atoi(settingAsString.c_str());
	return result;
}

void
CSettings::SetSetting(const string& keyValue, int settingValue)
{
	SetSetting(keyValue, AsString(settingValue));
}

Boolean
CSettings::GetSetting(const string& keyValue, UInt32& settingValue)
{
	string settingAsString;
	Boolean result = GetSetting(keyValue, settingAsString);
	settingValue = strtoul(settingAsString.c_str(), 0, 0);
	return result;
}

void
CSettings::SetSetting(const string& keyValue, UInt32 settingValue)
{
	SetSetting(keyValue, AsString(settingValue));
}

Boolean
CSettings::GetSetting(const string& keyValue, SDimension16& settingValue)
{
	int longValue;
	Boolean result = GetSetting(keyValue, longValue);
	if (result) {
		settingValue.width = longValue >> 16;
		settingValue.height = longValue & 0x0000ffff;
	}
	return result;
}

void
CSettings::SetSetting(const string& keyValue, SDimension16 settingValue)
{
	int v = ((UInt32)settingValue.width) << 16 | settingValue.height;
	SetSetting(keyValue, v);
}

Boolean
CSettings::GetSetting(const string& keyValue)
{
	string settingAsString;
	return GetSetting(keyValue, settingAsString);
}

const string gFalseAsString("FALSE");
const string gTrueAsString("TRUE");

Boolean
CSettings::GetSetting(const string& keyValue, Boolean& settingValue)
{
	string settingAsString;
	Boolean result = GetSetting(keyValue, settingAsString);
	settingValue = (settingAsString == gTrueAsString);
	return result;
}

void
CSettings::SetSetting(const string& keyValue, Boolean settingValue)
{
	string settingValueAsString(gFalseAsString);
	if (settingValue)
		settingValueAsString = gTrueAsString;
	SetSetting(keyValue, settingValueAsString);
}

Boolean
CSettings::IsDirty()
{
	return fDirty;
}
