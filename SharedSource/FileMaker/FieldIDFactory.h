#pragma once

#include "IDFactory.h"
#include "FMAE.h"

typedef CIDFactory<FMAE::FieldID> CFieldIDFactory;

namespace FieldIDFactory {
	CFieldIDFactory&	Get();
	int	GetID(const FMAE::FieldID&);
}

