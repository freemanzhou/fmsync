#pragma once

#include "CAEDescriptor.h"
#include "OpaqueInteger.h"
#include "Utilities.h"

#include <string>
#include <vector>

namespace Descriptors {

void ConvertToDescriptor(DescType theType, const void* theData, Size theDataSize, AEDesc* desc);
std::string PropertyAsString(LModelObject* mo, DescType theProperty);

template <class T> T convert(AEDesc*, int index);

template <class T>
void ExtractList(AEDesc* desc, std::vector<T>& list)
{
	long listCount = CountItems(desc);
	list.reserve(listCount);
	for (int i = 1; i <= listCount; i += 1) {
		list.push_back(convert<T>(desc, i));
	}
}

template <class T>
inline
T convert(AEDesc* desc, int i)
{
	return T(convert<int>(desc, i));
}

template <typename T>
void ConvertByType(const AEDesc* desc, T& v)
{
	ThrowIfOSErr_(CAEDescriptor::GetDescData(desc, &v, sizeof(v)));
}

template <>
void ConvertByType<std::string>(const AEDesc* desc, std::string& v)
{
	Size stringSize(CAEDescriptor::GetDescDataSize(desc));
	std::vector<char> stringBuffer(stringSize);
	ThrowIfOSErr_(CAEDescriptor::GetDescData(desc, &stringBuffer[0], stringSize));
	v = std::string(&stringBuffer[0], stringSize);
}

template <typename T>
void DoCoerceAndConvertByType(DescType desiredType, const AEDesc* desc, T& v)
{
	if (desc->descriptorType == desiredType) {
		ConvertByType(desc, v);
		return;
	}
	
	CAEDescriptor coerced;
	ThrowIfOSErr_(::AECoerceDesc(desc, desiredType, coerced));
	ConvertByType(coerced, v);
}

template <typename T> DescType GetDescriptorType(const T& v);

template <> DescType GetDescriptorType<bool>(const bool&)
{
	return typeBoolean;
}

template <> DescType GetDescriptorType<double>(const double&)
{
	return typeFloat;
}

template <> DescType GetDescriptorType<long>(const long&)
{
	return typeLongInteger;
}

template <> DescType GetDescriptorType<unsigned long>(const unsigned long&)
{
	return typeMagnitude;
}

template <> DescType GetDescriptorType<Rect>(const Rect&)
{
	return typeQDRectangle;
}

template <> DescType GetDescriptorType<std::string>(const std::string&)
{
	return typeText;
}

template <> DescType GetDescriptorType<RGBColor>(const RGBColor&)
{
	return typeRGBColor;
}

template <typename T> void CoerceAndConvertByType(const AEDesc* desc, T& v)
{
	DoCoerceAndConvertByType(GetDescriptorType(v), desc, v);
}

template <typename T> void ConvertToDescriptor(const T& v, AEDesc* desc)
{
	ConvertToDescriptor(GetDescriptorType(v), &v, sizeof(v), desc);
}

template <> void ConvertToDescriptor<std::string>(const std::string& v, AEDesc* desc)
{
	ConvertToDescriptor(typeText, v.data(), v.length(), desc);
}

template <>
bool convert<bool>(AEDesc* desc, int i)
{
	AEKeyword keyWord;
	DescType typeCode;
	long actualSize;
	Boolean theBool;
	ThrowIfOSErr_(AEGetNthPtr(desc, i, typeBoolean, &keyWord, &typeCode, &theBool, sizeof(theBool), &actualSize));
	return theBool;
}

template <>
int convert<int>(AEDesc* desc, int i)
{
	AEKeyword keyWord;
	DescType typeCode;
	long actualSize;
	long theInt;
	if (AEGetNthPtr(desc, i, typeInteger, &keyWord, &typeCode, &theInt, sizeof(theInt), &actualSize) != noErr)
		ThrowIfOSErr_(AEGetNthPtr(desc, i, typeEnumerated, &keyWord, &typeCode, &theInt, sizeof(theInt), &actualSize));
	return theInt;
}

template <>
std::string convert<std::string>(AEDesc* desc, int i)
{
	AEKeyword keyWord;
	DescType typeCode;
	long actualSize;
	ThrowIfOSErr_(AEGetNthPtr(desc, i, typeChar, &keyWord, &typeCode, 0, 0, &actualSize));
	std::vector<char> buffer(actualSize);
	ThrowIfOSErr_(AEGetNthPtr(desc, i, typeChar, &keyWord, &typeCode, &buffer[0], buffer.size(), &actualSize));
	return std::string(&buffer[0], actualSize);
}

template <>
std::vector<std::string> convert< std::vector<std::string> >(AEDesc* desc, int i)
{
	AEKeyword keyWord;
	CAEDescriptor theList;
	ThrowIfOSErr_(AEGetNthDesc(desc, i, typeAEList, &keyWord, theList));
	std::vector<std::string> strings;
	ExtractList(theList, strings);
	return strings;
}

}
