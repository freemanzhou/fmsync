#pragma once

void ConfirmFreeStackSpace();

template <typename T> inline void Clear(T &foo)
{
	BlockZero(&foo, sizeof(foo));
}

template <typename T> inline Handle AsHandle(T** typedHandle)
{
	Handle h = reinterpret_cast<Handle>(typedHandle);
	return h;
}

template <typename T> inline T** DuplicateHandle(T** typedHandle)
{
	Handle h = reinterpret_cast<Handle>(typedHandle);
	ThrowIfNil_(h);
	ThrowIfOSErr_(HandToHand(&h));
	return reinterpret_cast<T**>(h);
}

template <typename T> inline void DisposeHandle(T** typedHandle)
{
	if (typedHandle) {
		Handle h = reinterpret_cast<Handle>(typedHandle);
		DisposeHandle(h);
		ThrowIfMemError_();
	}
}

template <typename T> inline void ForgetHandle(T** &typedHandle)
{
	DisposeHandle(typedHandle);
	typedHandle = 0;
}

template <class T>
inline void ForgetObject(T* & x)
{
	T *temp = x;
	x = 0;
	delete temp;
}
