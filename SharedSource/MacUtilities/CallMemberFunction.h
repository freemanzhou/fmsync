/*
   Copyright (C) 2000 Coola, Inc. 
   All rights reserved.
 */
#pragma once

template <class T, class D>
class CallMemberFunction {
public:
	typedef void (T::*Handler)(const D&);

	CallMemberFunction(T* p, Handler h) : fTarget(p), fHandler(h) {}
	
	void operator()(const D& d) {
		(fTarget->*fHandler)(d);
	}
	
	T *fTarget;
	Handler fHandler;
};

template <class R, class T, class D>
class CallMemberFunctionR {
public:
	typedef R (T::*Handler)(const D&);

	CallMemberFunctionR(T* p, Handler h) : fTarget(p), fHandler(h) {}
	
	R operator()(const D& d) {
		return (fTarget->*fHandler)(d);
	}
	
	T *fTarget;
	Handler fHandler;
};

template <class T, class D1, class D2>
class CallMemberFunction2 {
public:
	typedef void (T::*Handler)(const D1&, const D2&);

	CallMemberFunction2(T* p, Handler h) : fTarget(p), fHandler(h) {}
	
	void operator()(const D1& d1, const D2& d2) {
		(fTarget->*fHandler)(d1, d2);
	}
	
	T *fTarget;
	Handler fHandler;
};

