#include "FieldIDFactory.h"

namespace FieldIDFactory {

CFieldIDFactory& Get()
{
	static CFieldIDFactory factory;
	return factory;
}

int	GetID(const FMAE::FieldID& id)
{
	return Get().GetID(id);
}

}

typedef CIDFactory<FMAE::FieldID> CFieldIDFactory;
