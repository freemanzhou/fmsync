#include <LDataStream.h>

#include "Reader.h"

CReader::CReader(const FSSpec& sourceFile, CReaderProgress* progress)
	: fSourceFile(sourceFile), fProgress(progress)
{
}

CReader::~CReader()
{
}

void
CReader::DoProgress(const string& progressString)
{
	if (fProgress)
		fProgress->DoProgress(progressString);
}

CDatabase*
CReader::GetDatabase()
{
	return &fDatabase;
}

void
CReader::Read()
{
	LFileStream file(fSourceFile);
	file.OpenDataFork(fsRdPerm);

	int fileLength = file.GetLength();
	
	StPointerBlock block(fileLength, false);
	if (Ptr(block) == 0) {
		DoRead(file);
	} else {
		file.ReadBlock(block, fileLength);
		LDataStream dataStream(Ptr(block), fileLength);
		DoRead(dataStream);
	}
}

CReaderProgress::CReaderProgress()
{
}

CReaderProgress::~CReaderProgress()
{
}

