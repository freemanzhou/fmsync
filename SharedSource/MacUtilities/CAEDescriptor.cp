#include "CAEDescriptor.h"
#include "CAEDescriptorBinaryFormat.h"
#include "Descriptors.h"
#include "MemoryUtilities.h"
#include "Stringiness.h"
#include "Utilities.h"

#include <algorithm>

using std::string;
using std::vector;

CAEDescriptor::CAEDescriptor(
    const CAEDescriptor& inOther)
{
    ThrowIfOSErr_( ::AEDuplicateDesc( &inOther.mDesc, &mDesc) );
}

CAEDescriptor&
CAEDescriptor::operator=(
    const CAEDescriptor& rhs )
{
    CAEDescriptor tmp(rhs);
    swap(tmp);
    return *this;
}

void
CAEDescriptor::swap(
    CAEDescriptor& inOther)
{
    std::swap(mDesc.descriptorType, inOther.mDesc.descriptorType);
    std::swap(mDesc.dataHandle, inOther.mDesc.dataHandle);
}

CAEDescriptor::CAEDescriptor(const AEDesc* inDesc)
{
    ThrowIfOSErr_( ::AEDuplicateDesc( inDesc, &mDesc) );
}

CAEDescriptor::CAEDescriptor()
{
	mDesc.descriptorType = typeNull;
	mDesc.dataHandle = 0;
}

//	Warning:
//
//	This constructor will NOT fail if the indicated parameter does
//	not exist.  Instead, the CAEDescriptor will be initialized to typeNull.
//	This way, the caller can explicitly check to see if the indicated
//	parameter was non-existent (typeNull), or just fail when trying to
//	get data out of the typeNull descriptor.

CAEDescriptor::CAEDescriptor(
	const AEDesc	&inDesc,
	AEKeyword		inKeyword,
	DescType		inDesiredType)
{
	mDesc.descriptorType = typeNull;
	mDesc.dataHandle = 0;
	
	OSErr	err;
	
	switch (inDesc.descriptorType) {

		case typeAERecord:
		case typeAppleEvent: {
			err = ::AEGetParamDesc(&inDesc, inKeyword, inDesiredType, &mDesc);
			//	Don't throw.
			//	The typeNull value assigned above is sufficient.
			break;
		}
		
		case typeNull:
			break;
		
		default: {
			CAEDescriptor	temp;
			
			err = ::AECoerceDesc(&inDesc, typeAERecord, &temp.mDesc);
			ThrowIfOSErr_(err);
			
			err = ::AEGetParamDesc(&temp.mDesc, inKeyword, inDesiredType, &mDesc);
			
			//	Don't throw if the descriptor doesn't exist.
			//	The typeNull value assigned above is sufficient.
			if (err != errAEDescNotFound)
				ThrowIfOSErr_(err);
			break;
		}
	}
}


CAEDescriptor::CAEDescriptor(
	DescType	inType,
	const void	*inData,
	SInt32		inSize)
{
	mDesc.descriptorType = typeNull;
	mDesc.dataHandle = NULL;
	
	OSErr	err = ::AECreateDesc(inType, inData, inSize, &mDesc);
	ThrowIfOSErr_(err);
}


CAEDescriptor::CAEDescriptor(
	Boolean		inValue)
{
	mDesc.descriptorType = typeNull;
	mDesc.dataHandle = NULL;
	
	OSErr	err = ::AECreateDesc(typeBoolean, &inValue, sizeof(inValue), &mDesc);
	ThrowIfOSErr_(err);
}


CAEDescriptor::CAEDescriptor(
	SInt16		inValue)
{
	mDesc.descriptorType = typeNull;
	mDesc.dataHandle = NULL;
	
	OSErr	err = ::AECreateDesc(typeShortInteger, &inValue, sizeof(inValue),
									&mDesc);
	ThrowIfOSErr_(err);
}


CAEDescriptor::CAEDescriptor(
	SInt32		inValue)
{
	mDesc.descriptorType = typeNull;
	mDesc.dataHandle = NULL;
	
	OSErr	err = ::AECreateDesc(typeLongInteger, &inValue, sizeof(inValue),
									&mDesc);
	ThrowIfOSErr_(err);
}


CAEDescriptor::CAEDescriptor(
	OSType		inValue)
{
	mDesc.descriptorType = typeNull;
	mDesc.dataHandle = NULL;
	
	OSErr	err = ::AECreateDesc(typeType, &inValue, sizeof(inValue), &mDesc);
	ThrowIfOSErr_(err);
}


CAEDescriptor::CAEDescriptor(
	ConstStringPtr	inString)
{
	mDesc.descriptorType = typeNull;
	mDesc.dataHandle = NULL;
	
	OSErr	err = ::AECreateDesc(typeChar, inString + 1, inString[0], &mDesc);
	ThrowIfOSErr_(err);
}

CAEDescriptor::CAEDescriptor(
	const string&	inString)
{
	mDesc.descriptorType = typeNull;
	mDesc.dataHandle = NULL;
	
	OSErr	err = ::AECreateDesc(typeChar, inString.data(), inString.length(), &mDesc);
	ThrowIfOSErr_(err);
}

CAEDescriptor::CAEDescriptor(const FSSpec& inSpec)
{
	mDesc.descriptorType = typeNull;
	mDesc.dataHandle = NULL;
	
	OSErr	err = ::AECreateDesc(typeFSS, &inSpec, sizeof(inSpec), &mDesc);
	ThrowIfOSErr_(err);
}


// ---------------------------------------------------------------------------
//		¥ ~CAEDescriptor
// ---------------------------------------------------------------------------
//	Destructor

CAEDescriptor::~CAEDescriptor()
{
	if (mDesc.dataHandle != 0) {
		::AEDisposeDesc(&mDesc);
	}
}

Size CAEDescriptor::GetDescDataSize(const AEDesc* theAEDesc)
{
#if ACCESSOR_CALLS_ARE_FUNCTIONS
	return AEGetDescDataSize(theAEDesc);
#else
	return GetHandleSize(theAEDesc->dataHandle);
#endif
}

OSErr CAEDescriptor::GetDescData(const AEDesc* theAEDesc, void * dataPtr, Size maximumSize)
{
#if ACCESSOR_CALLS_ARE_FUNCTIONS
	return AEGetDescData(theAEDesc, dataPtr, maximumSize);
#else
	StHandleLocker locker(theAEDesc->dataHandle);
	Size theSize(std::max(maximumSize, GetHandleSize(theAEDesc->dataHandle)));
	::BlockMoveData(*theAEDesc->dataHandle, dataPtr, theSize);
	return noErr;
#endif
}


OSErr CAEDescriptor::ReplaceDescData(DescType typeCode, const void* dataPtr, Size dataSize,
                                 AEDesc* theAEDesc)
{
#if ACCESSOR_CALLS_ARE_FUNCTIONS
	return AEReplaceDescData(typeCode, dataPtr, dataSize, theAEDesc);
#else
	StHandleBlock block(dataSize, false);
	if (!block.IsValid())
		return memFullErr;
	
	ForgetHandle(theAEDesc->dataHandle);
	
	theAEDesc->dataHandle = block.Release();
	theAEDesc->descriptorType = typeCode;
	return noErr;
#endif
}

OSErr CAEDescriptor::GetDescData(const AEDesc* theAEDesc, void * dataPtr, Size maximumSize);
OSErr CAEDescriptor::ReplaceDescData(DescType typeCode, const void* dataPtr, Size dataSize,
                            AEDesc* theAEDesc);

std::string CAEDescriptor::AsString() const
{
	CAEDescriptor r(Coerce(typeText));
	long textSize(GetDescDataSize(r));
	vector<char> textBuffer(textSize);
	ThrowIfOSErr_(GetDescData(r, &textBuffer[0], textSize));
	return string(&textBuffer[0], textSize);
}

CAEDescriptor CAEDescriptor::At(int index) const
{
	CAEDescriptor r;
	AEKeyword theAEKeyword;
	ThrowIfOSErr_(AEGetNthDesc(&mDesc, index, typeWildCard, &theAEKeyword, r));
	return r;
}

CAEDescriptor CAEDescriptor::Coerce(DescType desiredType) const
{
	if (mDesc.descriptorType == desiredType)
		return *this;
		
	CAEDescriptor r;
	OSErr err = AECoerceDesc(&mDesc, typeText, r);
	if (err == noErr)
		return r;
	
	if (err != errAECoercionFail)
		Throw_(err);

	AECoercionHandlerUPP handler;
	long handlerRefcon;
	Boolean fromTypeIsDesc;
	err = AEGetCoercionHandler(mDesc.descriptorType, desiredType, &handler, &handlerRefcon, 
		&fromTypeIsDesc, true);
	ThrowIfOSErr_(err);
	
	return *this;
}

int CAEDescriptor::Count() const
{
	if (mDesc.descriptorType != typeAEList)
		return 0;
		
	return CountItems(&mDesc);
}

// ---------------------------------------------------------------------------
//		¥ GetParamDesc
// ---------------------------------------------------------------------------
//	Load Descriptor data from an AppleEvent
//
//	Throws an OSErr exception if it can't get the data

void
CAEDescriptor::GetParamDesc(
	const AppleEvent	&inAppleEvent,
	AEKeyword			inKeyword,
	DescType			inDesiredType)
{
	OSErr	err = ::AEGetParamDesc(&inAppleEvent, inKeyword, inDesiredType,
									&mDesc);
	ThrowIfOSErr_(err);
}

// ---------------------------------------------------------------------------
//		¥ GetOptionalParamDesc
// ---------------------------------------------------------------------------
//	Load optional Descriptor data from an AppleEvent
//
//	Differs from GetParamDesc in that it does not throw an exception
//	if the request fails because the specified keyword data does not
//	exist. Use this function to extract optional parameters from
//	an AppleEvent.

void
CAEDescriptor::GetOptionalParamDesc(
	const AppleEvent&	inAppleEvent,
	AEKeyword			inKeyword,
	DescType			inDesiredType)
{
	OSErr	err = ::AEGetParamDesc(&inAppleEvent, inKeyword, inDesiredType,
									&mDesc);
	if ((err != errAEDescNotFound) && (err != noErr)) ThrowOSErr_(err);
}

DescType CAEDescriptor::GetType() const
{
	return mDesc.descriptorType;
}

bool
CAEDescriptor::HasData() const
{
	return mDesc.dataHandle != 0;
}

bool
CAEDescriptor::IsList() const
{
	return mDesc.descriptorType == typeAEList;
}

bool
CAEDescriptor::IsContainer() const
{
	return mDesc.descriptorType == typeAEList;
}

CAEDescriptor CAEDescriptor::MakeList()
{
	CAEDescriptor r;
	ThrowIfOSErr_(AECreateList(0, 0, false, r));
	return r;
}

CAEDescriptor CAEDescriptor::MakeRecord()
{
	CAEDescriptor r;
	ThrowIfOSErr_(AECreateList(0, 0, true, r));
	return r;
}

CAEDescriptor CAEDescriptor::MakeType(DescType t)
{
	CAEDescriptor r;
	OSErr	err = ::AECreateDesc(typeType, &t, sizeof(t), r);
	ThrowIfOSErr_(err);
	return r;
}

void CAEDescriptor::Put(int index, DescType inType, const void *inData, SInt32 inSize)
{
	ThrowIfOSErr_(AEPutPtr(&mDesc, index, inType, inData, inSize));
}

void CAEDescriptor::Put(int index, const CAEDescriptor& d)
{
	ThrowIfOSErr_(AEPutDesc(&mDesc, index, d));
}

void CAEDescriptor::PutByKey(const AEKeyword& theKey, const CAEDescriptor& d)
{
	ThrowIfOSErr_(AEPutKeyDesc(&mDesc, theKey, d));
}

CAEDescriptor CAEDescriptor::GetByKey(const AEKeyword& theAEKeyword) const
{
	CAEDescriptor r;
	ThrowIfOSErr_(AEGetKeyDesc(&mDesc, theAEKeyword, typeWildCard, r));
	return r;
}

namespace BinaryFormat {

static void WriteContainer(LStream& s, const CAEDescriptor& desc)
{
	Write(s, desc.GetType());
	UInt32 count(desc.Count());
	Write(s, count);
	for (int i = 1; i <=count; ++i) {
		CAEDescriptor desc2(desc.At(i));
		Write(s, desc2);
	}
}

template <>
void Write<CAEDescriptor>(LStream& s, const CAEDescriptor& desc)
{
	if (desc.IsContainer()) {
		WriteContainer(s, desc);
		return;
	}
	
	int descOffset = 0;
	//if (desc.GetType() == typeAERecord)
		//descOffset = 8;
		
	Size descSize(CAEDescriptor::GetDescDataSize(desc));
	vector<char> descBuffer(descSize);
	ThrowIfOSErr_(CAEDescriptor::GetDescData(desc, &descBuffer[0], descSize));
	Write(s, desc.GetType());
	Write(s, descSize - descOffset);
	s.WriteBlock(&descBuffer[descOffset], descSize-descOffset);
}

static void ReadContainer(LStream& s, CAEDescriptor& desc)
{
	desc = CAEDescriptor::MakeList();
	UInt32 count;
	BinaryFormat::Read(s, count);
	for (int i = 1; i <= count; ++i) {
		CAEDescriptor desc2;
		Read(s, desc2);
		desc.Put(i, desc2);
	}
}

template <>
void Read<CAEDescriptor>(LStream& s, CAEDescriptor& desc)
{
	DescType descType;
	Size descSize;
	
	Read(s, descType);
	if (descType == typeAEList) {
		ReadContainer(s, desc);
	} else {
		Read(s, descSize);
		vector<char> descBuffer(descSize);
		s.ReadBlock(&descBuffer[0], descSize);
		Descriptors::ConvertToDescriptor(descType, &descBuffer[0], descSize, desc);
	}
}

}