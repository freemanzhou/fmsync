#pragma once

#include <vector>

template <class T>
class OpaqueInteger {
public:
	explicit OpaqueInteger(int i = 0) : fValue(i) { }
	
	bool	IsValid() const {return fValue != 0;}
	int		GetValue() const {return fValue;}
	void	SetValue(int i){fValue = i;}
	
	class Secret { };
	typedef Secret const volatile* SecretPointer;
	operator SecretPointer() const { return IsValid() ? reinterpret_cast<SecretPointer>(1) : 0; }
	
	// not defined a full set, < defined for map et al., == defined for == of vectors
	bool operator < (const OpaqueInteger<T>& a) const {return fValue < a.fValue;} 
	bool operator == (const OpaqueInteger<T>& a) const {return fValue == a.fValue;} 

	static std::vector<OpaqueInteger<T> > ConvertVector(const std::vector<int>&);

private:
#if 0
	void operator& () const;
	friend  uninitialized_copy<OpaqueInteger<T> >;
#endif
	int		fValue;
};

template <class T>
std::vector<OpaqueInteger<T> > OpaqueInteger<T>::ConvertVector(const std::vector<int>& integerVector)
{
	typedef std::vector<OpaqueInteger<T> > result;
	result opaqueVector;
	for (std::vector<int>::const_iterator i = integerVector.begin(); 
			i != integerVector.end(); ++i)
		opaqueVector.push_back(OpaqueInteger<T>(*i));
	return opaqueVector;
}
