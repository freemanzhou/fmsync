#pragma once

#include <string>
#include <vector>

#include "BinaryFormat.h"

class FileReference {
public:
	FileReference();
	FileReference(const FileReference& inOriginal);
	FileReference&	operator=(const FileReference& inRhs);

	FileReference(const FSSpec&);
	FileReference(AliasHandle);
	~FileReference();
	
	static void swap(FileReference&, FileReference&);
	
	bool IsGood() const;
	FSSpec GetFSSpec() const;
	std::string Name() const;
	
	void Read(LStream& s);
	void Write(LStream& s) const;

private:
	
	void	ForgetAlias();
	void	MakeAliasFromFSSpec();
	void	MakeFSSpecFromAlias();
	

	AliasHandle fAlias;
	FSSpec fFSSpec;
	bool fAliasIsGood;
};

bool operator!=(const FileReference&, const FileReference&);
	

namespace BinaryFormat {

template <>
inline void Write(LStream& s, const FileReference& f)
{
	f.Write(s);
}

template <>
inline void Read(LStream& s, FileReference& f)
{
	f.Read(s);
}

}