#pragma once
#include <cstdint>

static constexpr int log2(const int x) {
	int res = 0;
	int n	= x;
	while (n >>= 1)
		++res;
	return res;
}

template <class T>
class FlaggedPtr {
	uint64_t ptr;

   public:
	FlaggedPtr(T* ptr) : ptr((uint64_t)ptr) {}
	static const int space = log2(alignof(T));
	T*		  operator->() { return (T*)(ptr & mask); }
	T&		  operator*() { return *(T*)(ptr & mask); }

	int	 getFlags() const { return ptr & ~mask; }
	void setFlags(int flags) {
		ptr &= mask;
		ptr |= flags & ~mask;
	}

	operator T*() { return (T*)(ptr & mask); }
	operator const T*() const { return (const T*)(ptr & mask); }
	bool operator==(const FlaggedPtr<T>& rhs) const { return (const T*)(*this) == (const T*)rhs; }
	bool operator!=(const FlaggedPtr<T>& rhs) const { return (const T*)(*this) != (const T*)rhs; }

   private:
	static const uint64_t mask = uint64_t(-1) ^ ((1 << space) - 1);
};
