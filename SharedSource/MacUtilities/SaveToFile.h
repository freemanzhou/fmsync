#pragma once

void WriteStringAsNewFile(const FSSpec& targetFile, OSType creator, OSType fileType, const std::string& contents);
void WriteStringAsFile(const FSSpec& targetFile, OSType creator, OSType fileType, const std::string& contents);