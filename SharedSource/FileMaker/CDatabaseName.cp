#include "CDatabaseName.h"
#include "CFileMaker.h"
#include "BinaryFormat.h"

CDatabaseName::CDatabaseName()
	: fCreator(0)
{
}

CDatabaseName::CDatabaseName(const string& name, OSType creator)
	: fName(name), fCreator(creator)
{
}

CDatabaseName::~CDatabaseName()
{
}

string
CDatabaseName::GetName() const
{
	return fName;
}

OSType
CDatabaseName::GetCreator() const
{
	return fCreator;
}

CDatabaseName&
CDatabaseName::operator=(
    const CDatabaseName& rhs )
{
    CDatabaseName tmp(rhs);
    swap(tmp);
    return *this;
}

void
CDatabaseName::swap(
    CDatabaseName& inOther)
{
    ::swap(fName, inOther.fName);
    ::swap(fCreator, inOther.fCreator);
}


namespace BinaryFormat {

void Write(LStream& s, const CDatabaseName& theName)
{
	BinaryFormat::Write(s, theName.GetName());
	BinaryFormat::Write(s, theName.GetCreator());
}

void Read(LStream& s, CDatabaseName& theName)
{
	string aName;
	OSType creator;
	BinaryFormat::Read(s, aName);
	BinaryFormat::Read(s, creator);
	theName = CDatabaseName(aName,  creator);
}

}