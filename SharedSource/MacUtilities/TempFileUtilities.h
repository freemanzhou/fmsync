#pragma once

void GetTemporaryFile(FSSpec *outSpec);
void GetTemporaryFileOnVolume(short vRefNum, FSSpec *outSpec);
void GetTemporaryFileNear(const FSSpec& inSpec, FSSpec *outSpec);
bool GetTempFileNamed(ConstStringPtr fileName, FSSpec *outSpec);
void SwapFiles(const FSSpec& inSpecA, const FSSpec& inSpecB);