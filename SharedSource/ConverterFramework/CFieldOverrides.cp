#include "CFieldOverrides.h"
#include "BinaryFormat.h"
#include "Utilities.h"

void
CFieldOverrides::Read(LStream& s, CFieldOverrides& i)
{
	CFieldOverrides temp;
	
	BinaryFormat::Read(s, temp.fName);
	BinaryFormat::Read(s, temp.fType);
	BinaryFormat::Read(s, temp.fWidth);
	BinaryFormat::Read(s, temp.fReadOnly);
	BinaryFormat::Read(s, temp.fPopups);
	BinaryFormat::Read(s, temp.fExtra);
	BinaryFormat::Read(s, temp.fExtra2);
	BinaryFormat::Read(s, temp.fFieldCalcValue1);
	BinaryFormat::Read(s, temp.fFieldCalcValue2);
	swap(temp, i);
}

void
CFieldOverrides::Read115(LStream& s, CFieldOverrides& i)
{
	CFieldOverrides temp;
	
	BinaryFormat::Read(s, temp.fName);
	BinaryFormat::Read(s, temp.fType);
	BinaryFormat::Read(s, temp.fWidth);
	BinaryFormat::Read(s, temp.fReadOnly);
	BinaryFormat::Read(s, temp.fPopups);
	BinaryFormat::Read(s, temp.fExtra);
	BinaryFormat::Read(s, temp.fExtra2);
	swap(temp, i);
}

void
CFieldOverrides::Write(LStream& s, const CFieldOverrides& i)
{
	BinaryFormat::Write(s, i.fName);
	BinaryFormat::Write(s, i.fType);
	BinaryFormat::Write(s, i.fWidth);
	BinaryFormat::Write(s, i.fReadOnly);
	BinaryFormat::Write(s, i.fPopups);
	BinaryFormat::Write(s, i.fExtra);
	BinaryFormat::Write(s, i.fExtra2);
	BinaryFormat::Write(s, i.fFieldCalcValue1);
	BinaryFormat::Write(s, i.fFieldCalcValue2);
}

void
CFieldOverrides::OverrideWith(const CFieldOverrides& o)
{
	ApplyDiffMap(fName, o.fName);
	ApplyDiffMap(fType, o.fType);
	ApplyDiffMap(fWidth, o.fWidth);
	ApplyDiffMap(fReadOnly, o.fReadOnly);
	ApplyDiffMap(fPopups, o.fPopups);
	ApplyDiffMap(fExtra, o.fExtra);
	ApplyDiffMap(fExtra2, o.fExtra2);
	ApplyDiffMap(fFieldCalcValue1, o.fFieldCalcValue1);
	ApplyDiffMap(fFieldCalcValue2, o.fFieldCalcValue2);
}

bool operator==(const CFieldOverrides&	inLhs, const CFieldOverrides& inRhs)
{
	return
		(inLhs.fName == inRhs.fName) && 
		(inLhs.fType == inRhs.fType) && 
		(inLhs.fWidth == inRhs.fWidth) && 
		(inLhs.fReadOnly == inRhs.fReadOnly) && 
		(inLhs.fPopups == inRhs.fPopups) && 
		(inLhs.fExtra == inRhs.fExtra) && 
		(inLhs.fExtra2 == inRhs.fExtra2) && 
		(inLhs.fFieldCalcValue1 == inRhs.fFieldCalcValue1) && 
		(inLhs.fFieldCalcValue2 == inRhs.fFieldCalcValue2);
}
