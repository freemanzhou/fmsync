#pragma once

#include <LString.h>

class LStream;

class	CDatabaseFile{

public:
							CDatabaseFile(const FSSpec& inFile);		
							CDatabaseFile(AliasHandle inAlias);		
							CDatabaseFile(LStream& inStream);		
		virtual 			~CDatabaseFile();
		ConstStringPtr		GetName();
		void				GetFSSpec(FSSpec *spec);
		AliasHandle			CreateAliasToFile();
		void				WriteToStream(LStream& inStream);

private:
		void			Initialize();
		FSSpec			fFile;
};

class CompareDatabaseFileByName {
public:
	bool operator() (CDatabaseFile *p1, CDatabaseFile *p2) const
	{
		LStr255 name1(p1->GetName());
		name1.SetCompareFunc(LString::CompareIgnoringCase);
		return name1.CompareTo(p2->GetName()) > 0;
	}
};

