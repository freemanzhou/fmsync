#include "BinaryFormat.h"
#include "FileReference.h"
#include "FolderUtilities.h"
#include "MemoryUtilities.h"

FileReference::FileReference(const FSSpec& f)
	: fFSSpec(f), fAliasIsGood(false), fAlias(0)
{
	MakeAliasFromFSSpec();
}

FileReference::FileReference(AliasHandle ah)
	: fAlias(ah), fAliasIsGood(false)
{
	MakeFSSpecFromAlias();
}

FileReference::FileReference()
	: fAlias(0), fAliasIsGood(false)
{
	Clear(fFSSpec);
}

FileReference::FileReference(const FileReference& inOriginal)
{
	if (inOriginal.fAliasIsGood)
		fAlias = DuplicateHandle(inOriginal.fAlias);
	else
		fAlias = 0;
	MakeFSSpecFromAlias();
}

FileReference&	FileReference::operator=(const FileReference& inRhs)
{
	FileReference fr(inRhs);
	swap(*this, fr);
	return *this;
}

FileReference::~FileReference()
{
	ForgetAlias();
}

void FileReference::swap(FileReference& a, FileReference& b)
{
	std::swap(a.fAlias, b.fAlias);
	std::swap(a.fFSSpec, b.fFSSpec);
	std::swap(a.fAliasIsGood, b.fAliasIsGood);
}

bool FileReference::IsGood() const
{
	return fAliasIsGood;
}

FSSpec FileReference::GetFSSpec() const
{
	ThrowIfNot_(fAliasIsGood);
	return fFSSpec;
}

void FileReference::MakeAliasFromFSSpec()
{
	ForgetAlias();
	fAlias = Folders::AliasFromFSSpec(fFSSpec);
	fAliasIsGood = true;
}

void FileReference::MakeFSSpecFromAlias()
{
	fAliasIsGood = false;
	try {
		if (fAlias && GetHandleSize(AsHandle(fAlias)) > 0) {
			fFSSpec = Folders::FSSpecFromAlias(fAlias);
			fAliasIsGood = true;
		}
	} catch (...) {
	}
}

void FileReference::Read(LStream& s)
{
	BinaryFormat::Read(s, fAlias);
	MakeFSSpecFromAlias();
}

void FileReference::Write(LStream& s) const
{
	BinaryFormat::Write(s, fAlias);
}

void FileReference::ForgetAlias()
{
	ForgetHandle(fAlias);
}

std::string FileReference::Name() const
{
	return Folders::NameFromAlias(fAlias);
}

bool operator!=(const FileReference& a, const FileReference& b)
{
	if (a.IsGood() != b.IsGood())
		return true;
	
	if (a.IsGood()) {
		return !LFile::EqualFileSpec(a.GetFSSpec(), b.GetFSSpec());
	}

	return false;
}
