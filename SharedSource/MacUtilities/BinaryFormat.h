#pragma once

#include <map>
#include <string>
#include <set>
#include <utility>
#include <vector>

#include "OpaqueInteger.h"
#ifdef CONDUIT
#include "FMAE.h"
#endif

namespace BinaryFormat {

template <class T> void Write(LStream& s, const T&);
template <class T> void Read(LStream& s, T&);

template <>
void Write(LStream& s, const int& i)
{
	long l(i);
	s << l;
}

template <>
void Read(LStream& s, int& i)
{
	long l(0);
	s >> l;
	i = l;
}

template <>
void Write(LStream& s, const unsigned short& i)
{
	long l(i);
	s << l;
}

template <>
void Read(LStream& s, unsigned short& i)
{
	long l(0);
	s >> l;
	i = l;
}

template <>
void Write(LStream& s, const bool& i)
{
	long l(i);
	s << l;
}

template <>
void Read(LStream& s, bool& i)
{
	long l(0);
	s >> l;
	i = l;
}

template <>
void Write(LStream& s, const RGBColor& c)
{
	Write(s, c.red);
	Write(s, c.green);
	Write(s, c.blue);
}

template <>
void Read(LStream& s, RGBColor& c)
{
	Read(s, c.red);
	Read(s, c.green);
	Read(s, c.blue);
}

inline void Write(LStream& s, Handle h)
{
	#ifdef _DEBUG
	ThrowIfNil_(h);
	#endif
	
	long l(GetHandleSize(h));
	s << l;
	StHandleLocker locker(h);
	s.WriteBlock(*h, l);
}

inline void Read(LStream& s, Handle& h)
{
	long l(0);
	s >> l;
	if (h == 0) {
		h = NewHandle(l);
	} else {
		SetHandleSize(h, l);
	}
	ThrowIfMemFail_(h);
	StHandleLocker locker(h);
	s.ReadBlock(*h, l);
}

inline void Write(LStream& s, AliasHandle h)
{
	Write(s, (Handle)h);
}

template <>
inline void Read(LStream& s, AliasHandle& h)
{
	Handle rh(0);
	Read(s, rh);
	h = (AliasHandle)rh;
}

template <>
inline void Write(LStream& s, const long& l)
{
	s << l;
}

template <>
inline void Read(LStream& s, long& l)
{
	s >> l;
}

template <>
inline void Write(LStream& s, const unsigned long& l)
{
	s << l;
}

template <>
inline void Read(LStream& s, unsigned long& l)
{
	s >> l;
}

template <>
void Write(LStream& s, const SDimension32& p)
{
	Write(s, p.width);
	Write(s, p.height);
}

template <>
void Read(LStream& s, SDimension32& p)
{
	Read(s, p.width);
	Read(s, p.height);
}

template <>
void Write(LStream& s, const std::string& str)
{
	long stringLength(str.length());
	s << str.length();
	s.WriteBlock(str.data(), stringLength);
}

template <>
void Read(LStream& s, std::string& str)
{
	long stringLength;
	s >> stringLength;
	StPointerBlock block(stringLength);
	s.ReadBlock(block, stringLength);
	str = std::string(block, stringLength);
}

#ifdef CONDUIT
template <>
inline void Write(LStream& s, const FMAE::FieldID& fieldID)
{
	FMAE::FieldID::Write(s, fieldID);
}

template <>
inline void Read(LStream& s, FMAE::FieldID& fieldID)
{
	FMAE::FieldID::Read(s, fieldID);
}
#endif

template <class T, class U>
void Write(LStream& s, const std::pair<T,U>& i)
{
	Write(s, i.first);
	Write(s, i.second);
}

template <class T, class U>
void Read(LStream& s, std::pair<T,U>& i)
{
	Read(s, i.first);
	Read(s, i.second);
}

template <class T>
void Write(LStream& s, const OpaqueInteger<T>& i)
{
	Write(s, i.GetValue());
}

template <class T>
void Read(LStream& s, OpaqueInteger<T>& i)
{
	int local;
	Read(s, local);
	i = OpaqueInteger<T>(local);
}

template <class ForwardIterator>
void Write(LStream& s, ForwardIterator begin, ForwardIterator end)
{
	Write(s, distance(begin, end));
	for (ForwardIterator i = begin; i != end; ++i) {
		Write(s, *i);
	}
}

template <class OutputIterator, class T> void Read(LStream&s, OutputIterator i, T)
{
	int count;
	Read(s, count);
	while (count > 0) {
		T value;
		Read(s, value);
		*i = value;
		++i;
		--count;
	}
}

template <class T> void Write(LStream& s, const std::vector<T>& c)
{
	Write(s, c.begin(), c.end());
}

template <class T> void Read(LStream&s, std::vector<T> &c)
{
	Read(s, back_inserter(c), T());
}


template <class T> void Write(LStream& s, const std::set<T>& c)
{
	Write(s, c.begin(), c.end());
}

template <class T> void Read(LStream&s, std::set<T> &c)
{
	Read(s, inserter(c, c.end()), T());
}

template <class T, class U> void Write(LStream& s, const std::map<T,U>& c)
{
	Write(s, c.begin(), c.end());
}

template <class T, class U> void Read(LStream&s, std::map<T, U>& c)
{
	Read(s, inserter(c, c.end()), std::pair<T, U>());
}

}