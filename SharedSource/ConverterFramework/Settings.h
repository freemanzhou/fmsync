#pragma once

#include "stringiness.h"

class CSettings {
public:
			virtual 	~CSettings();

	
	static CSettings&	GetSettings();
	
	Boolean				GetSetting(const string& keyValue, string& settingValue);
	void				SetSetting(const string& keyValue, const string& settingValue);

	Boolean				GetSetting(const string& keyValue, int& settingValue);
	void				SetSetting(const string& keyValue, int settingValue);

	Boolean				GetSetting(const string& keyValue, Boolean& settingValue);
	void				SetSetting(const string& keyValue, Boolean settingValue);

	Boolean				GetSetting(const string& keyValue, SDimension16& settingValue);
	void				SetSetting(const string& keyValue, SDimension16 settingValue);

	Boolean				GetSetting(const string& keyValue, UInt32& settingValue);
	void				SetSetting(const string& keyValue, UInt32 settingValue);

	Boolean				GetSetting(const string& keyValue);

	void				LoadSettings(LStream& inStream);
	void				WriteSettings(LStream& inStream);

	Boolean				IsDirty();

private:
						CSettings();
	
	map<string, string> fSettings;
	Boolean				fDirty;
};

