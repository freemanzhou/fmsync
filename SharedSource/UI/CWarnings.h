/* 
	CWarnings.h

	Author:			Rob Tsuk
	Description:	<describe the CWarnings class here>
*/

#ifndef CWarnings_H
#define CWarnings_H

#include "BinaryFormat.h"

namespace Warnings {

typedef int WarningID;

}

typedef map<Warnings::WarningID, bool> CDisabledMap;

class CWarnings
{
public:
	CWarnings();
	CWarnings(const CDisabledMap& initialSettings);
	friend CWarnings& Get();
	virtual ~CWarnings();
	
	CDisabledMap GetAllDisabled() const;
	void SetAllDisabled(const CDisabledMap&);

	bool Disabled(Warnings::WarningID warning) const;
	void SetDisabled(Warnings::WarningID warning, bool);
	
	void Read(LStream&);
	void Write(LStream&) const;

private:

	CDisabledMap	fDisabled;
};

namespace BinaryFormat {

template <>
inline void Write(LStream& s, const CWarnings& w)
{
	w.Write(s);
}

template <>
inline void Read(LStream& s, CWarnings& w)
{
	w.Read(s);
}

}

#endif	// CWarnings_H
