#include "CUploadRequest.h"
#include "BinaryFormat.h"
#include "MoreFilesExtras.h"
#include "Stringiness.h"
#include "Str255.h"
#include "Utilities.h"

CUploadRequest::CUploadRequest(const string& dbName, bool changes)
	: fName(dbName), fChanges(changes), fAlias(0)
{
	long systemScript = GetScriptManagerVariable(smSysScript);
	fTranslateText = (systemScript == smRoman);
}

CUploadRequest::CUploadRequest()
	: fChanges(false), fAlias(0)
{
}

CUploadRequest::CUploadRequest(const CUploadRequest& i)
	: fChanges(i.fChanges), fName(i.fName), fMacName(i.fMacName), fAlias(0)
{
	Handle h = i.fAlias;
	if (HandToHand(&h) == noErr)
		fAlias = h;
}

CUploadRequest::~CUploadRequest()
{
	if (fAlias)
		DisposeHandle(fAlias);
}

void CUploadRequest::SetLocation(const FSSpec& f)
{
	if (fAlias) {
		DisposeHandle(fAlias);
		fAlias = 0;
	}
	
	fMacName = AsString(f.name);

	FSSpec parentSpec = f;
	parentSpec.name[0] = 0;
	FSSpec normalizedParentSpec;
	Normalize(parentSpec, &normalizedParentSpec);
	AliasHandle alias;
	ThrowIfOSErr_(::NewAliasMinimal(&normalizedParentSpec, &alias));
	fAlias = (Handle)alias;
}

bool CUploadRequest::GetLocation(FSSpec& f)
{
	LString::CopyPStr(AsStr255(fMacName), f.name);

	FSSpec parentSpec;
	Boolean wasChanged;
	int rulesMask = kARMNoUI | kARMSearch;
	short fileCount = 1;
	OSErr err = ::MatchAlias(0, rulesMask, (AliasHandle)fAlias, &fileCount, &parentSpec, &wasChanged, 0, 0);
	if (err == noErr) {
		long dirID;
		Boolean isDir;
		ThrowIfOSErr_(FSpGetDirectoryID(&parentSpec, &dirID, &isDir));
		f.vRefNum = parentSpec.vRefNum;
		f.parID = dirID;
		if (!isDir)
			return false;
	}
	return err == noErr;
}

void
BinaryFormat::Write(LStream& s, const CUploadRequest& r)
{
	BinaryFormat::Write(s, r.fName);
	BinaryFormat::Write(s, r.fChanges);
	BinaryFormat::Write(s, r.fTranslateText);
	BinaryFormat::Write(s, r.fMacName);
	s << r.fAlias;
}

void
BinaryFormat::Read(LStream& s, CUploadRequest& r)
{
	CUploadRequest temp;
	BinaryFormat::Read(s, temp.fName);
	BinaryFormat::Read(s, temp.fChanges);
	BinaryFormat::Read(s, temp.fTranslateText);
	BinaryFormat::Read(s, temp.fMacName);
	s >> temp.fAlias;
	swap(temp, r);
}

CUploadRequest&
CUploadRequest::operator=(
    const CUploadRequest& rhs )
{
    CUploadRequest tmp(rhs);
    swap(tmp);
    return *this;
}

void
CUploadRequest::swap(
    CUploadRequest& inOther)
{
    ::swap(fName, inOther.fName);
    ::swap(fMacName, inOther.fMacName);
    ::swap(fChanges, inOther.fChanges);
    ::swap(fAlias, inOther.fAlias);
}

