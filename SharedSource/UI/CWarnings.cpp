/* 
	CWarnings.cpp

	Author:			Rob Tsuk
	Description:	<describe the CWarnings class here>
*/

#include "CWarnings.h"

using namespace Warnings;

CWarnings::CWarnings()
{
}

CWarnings::CWarnings(const map<WarningID, bool>& initialSettings)
	: fDisabled(initialSettings)
{
}

CWarnings::~CWarnings()
{
}

map<WarningID, bool> CWarnings::GetAllDisabled() const
{
	return fDisabled;
}

void CWarnings::SetAllDisabled(const map<WarningID, bool>& disabled)
{
	fDisabled = disabled;
}

bool CWarnings::Disabled(WarningID warning) const
{
	CDisabledMap::const_iterator f = fDisabled.find(warning);
	if (f == fDisabled.end())
		return false;
	return f->second;
}

void CWarnings::SetDisabled(WarningID warning, bool dis)
{
	fDisabled[warning] = dis;
}

void CWarnings::Read(LStream& s)
{
	BinaryFormat::Read(s, fDisabled);
}

void CWarnings::Write(LStream& s) const
{
	BinaryFormat::Write(s, fDisabled);
}
