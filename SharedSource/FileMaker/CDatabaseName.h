#pragma once

class CDatabaseName {
public:
							CDatabaseName();
							CDatabaseName(const string&, OSType);
							~CDatabaseName();
	
	string					GetName() const;
	OSType					GetCreator() const;

	CDatabaseName&	operator=(const CDatabaseName& rhs );
	void			swap(CDatabaseName&);

private:
	string	fName;
	OSType	fCreator;
};

inline
Boolean operator==(
				const CDatabaseName&	inLhs,
				const CDatabaseName&	inRhs)
		{
			return (inLhs.GetCreator() == inRhs.GetCreator()) && 
				(inLhs.GetName() == inRhs.GetName());
		}

inline
Boolean operator<(
				const CDatabaseName&	inLhs,
				const CDatabaseName&	inRhs)
		{
			if (inLhs.GetName() == inRhs.GetName())
				return (inLhs.GetCreator() < inRhs.GetCreator()); 
			return (inLhs.GetName() < inRhs.GetName()); 
		}


namespace BinaryFormat {

void Write(LStream& s, const CDatabaseName& theName);
void Read(LStream& s, CDatabaseName& theName);

}
