#include "Descriptors.h"
#include "CAEDescriptor.h"

#if SELF_TESTS
#include "CppUnit.h"
#endif

namespace Descriptors {

static void test()
{
	CAEDescriptor desc;
	int i;
	ConvertByType(desc, i);
	ConvertToDescriptor(i, desc);
	ConvertToDescriptor('a', desc);
	ConvertToDescriptor(3.2, desc);
}

std::string PropertyAsString(LModelObject* mo, DescType theProperty)
{
	CAEDescriptor theDesc;
	mo->GetAEProperty(theProperty, CAEDescriptor::MakeType(typeText), theDesc);
	std::string s;
	Descriptors::CoerceAndConvertByType(theDesc, s);
	return s;
}


void ConvertToDescriptor(DescType theType, const void* theData, Size theDataSize, AEDesc* desc)
{
	if (desc->descriptorType == typeNull) {
		if (theType == typeAERecord) {
			ThrowIfOSErr_(AECreateList(0, 0, true, desc));
			ThrowIfOSErr_(CAEDescriptor::ReplaceDescData(theType, theData, theDataSize, desc));
		} else {
			ThrowIfOSErr_ (::AECreateDesc(theType, theData, theDataSize, desc));
		}
	} else {
		ThrowIfOSErr_(CAEDescriptor::ReplaceDescData(theType, theData, theDataSize, desc));
	}
}

}

