#pragma once

#include <string>

class	CAEDescriptor {
public:
	AEDesc	mDesc;

			//	These operators sometimes let you appear to pass
			//	the CAEDescriptor object rather than having to pass the
			//	object.mDesc.
			operator	AEDesc*() { return &mDesc; }
			operator	AEDesc&() { return mDesc; }
			operator    const AEDesc*() const { return &mDesc; }
			operator    const AEDesc&() const { return mDesc; }
	
			CAEDescriptor(const AEDesc* inDesc);
			CAEDescriptor();
			CAEDescriptor(
					const AEDesc	&inDesc,
					AEKeyword		inKeyword,
					DescType		inDesiredType = typeWildCard);
			CAEDescriptor(DescType inType, const void *inData, SInt32 inSize);
			CAEDescriptor(Boolean inValue);
			CAEDescriptor(SInt16 inValue);
			CAEDescriptor(SInt32 inValue);
			CAEDescriptor(OSType inValue);
			CAEDescriptor(ConstStringPtr inString);
			CAEDescriptor(const std::string& inString);
			CAEDescriptor(const FSSpec&);
			CAEDescriptor(const CAEDescriptor& inOther);
			~CAEDescriptor();
	
	static CAEDescriptor MakeList();		
	static CAEDescriptor MakeRecord();		
	static CAEDescriptor MakeType(DescType);

	static Size GetDescDataSize(const AEDesc* theAEDesc);
	static OSErr GetDescData(const AEDesc* theAEDesc, void * dataPtr, Size maximumSize);
	static OSErr ReplaceDescData(DescType typeCode, const void* dataPtr, Size dataSize,
                                 AEDesc* theAEDesc);
                                 
                                
	CAEDescriptor&	operator=(const CAEDescriptor& rhs );
	void			swap(CAEDescriptor& inOther);
	
	bool	HasData() const;
	bool	IsList() const;
	bool	IsContainer() const;
	int		Count() const;
	DescType GetType() const;
	CAEDescriptor At(int index) const;
	CAEDescriptor GetByKey(const AEKeyword&) const;
	std::string AsString() const;
	
	void	Put(int index, DescType inType, const void *inData, SInt32 inSize);
	void	Put(int index, const CAEDescriptor&);
	void	PutByKey(const AEKeyword& theKey, const CAEDescriptor& d);
	CAEDescriptor Coerce(DescType inType) const;

	void	GetParamDesc(const AppleEvent& inAppleEvent, AEKeyword inKeyword,
							DescType inDesiredType);
	void	GetOptionalParamDesc(const AppleEvent& inAppleEvent,
							AEKeyword inKeyword, DescType inDesiredType);
};

