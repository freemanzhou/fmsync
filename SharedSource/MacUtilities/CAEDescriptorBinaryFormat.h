#pragma once

#include "BinaryFormat.h"
#include "CAEDescriptor.h"

namespace BinaryFormat {

template <> void Write<CAEDescriptor>(LStream& s, const CAEDescriptor& desc);

template <> void Read<CAEDescriptor>(LStream& s, CAEDescriptor& desc);

}