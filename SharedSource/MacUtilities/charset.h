#pragma once
#include <string>

const char returnCharacter = 13;
const char verticalTab = 11;
const char groupSeparator = 29;

void ConvertToPilotText(string& inputString);
void ConvertToDesktopText(string& inputString);
string ConvertToPilotTextAndReturns(const string& inputString);
string ConvertToDesktopTextAndReturns(const string& inputString);


void ConvertToPilotText(char *inputString);
void ConvertToDesktopText(char *inputString);

void ConvertToPilotText(StringPtr inputString);
void ConvertToDesktopText(StringPtr inputString);

void ConvertToDesktopReturns(string& inString);
void ConvertToPilotReturns(string& inString);

void ConvertToVerticalTab(string& inString);
void ConvertVerticalTabToPilotReturn(string& inString);
void ConvertVerticalTabToMacReturn(string& inString);
