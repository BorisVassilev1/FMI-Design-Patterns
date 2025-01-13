#pragma once

#include <cstring>
#include <iostream>
#include <ostream>
#include <type_traits>
#include <utility>
#include "utils.hpp"

namespace std {
// clang-format off
template<class T> struct remove_ref_or_p { using type = T; };
template<class T> struct remove_ref_or_p<T*> { using type = T; };
template<class T> struct remove_ref_or_p<T* const> { using type = T; };
template<class T> struct remove_ref_or_p<T* volatile> { using type = T; };
template<class T> struct remove_ref_or_p<T* const volatile> { using type = T; };

template<class T> struct remove_ref_or_p<T*&> { using type = T*; };
template<class T> struct remove_ref_or_p<T* const &> { using type = T* const; };
template<class T> struct remove_ref_or_p<T* volatile &> { using type = T* const volatile; };
template<class T> struct remove_ref_or_p<T* const volatile &> { using type = T* const volatile; };
// clang-format on

template <class T>
using remove_ref_or_p_t = remove_ref_or_p<T>::type;

template <class T>
using remove_ref_and_p_t = remove_pointer_t<remove_reference_t<T>>;

// clang-format off
template <bool b, class T> struct add_pointer_if { using type = T; };
template <class T> struct add_pointer_if<true, T> { using type = std::add_pointer_t<T>; };
// clang-format on

};	   // namespace std

template <typename T>
struct AutoRef {
	// a reference if possible or a owned object otherwise
	T ref;

	template <class Q>
	AutoRef(Q &value) : ref(value) {}
	template <class Q>
	AutoRef(Q &&value) : ref(std::move(value)) {}

	// removing the reference is needed for cases where T is a reference itself
	std::remove_reference_t<T>		 *operator->() { return &ref; }
	const std::remove_reference_t<T> *operator->() const { return &ref; }

	// reference to reference is illegal and thus T& is always deduced as just
	// a reference even when T is a reference type
	T		&operator*() { return ref; }
	const T &operator*() const { return ref; }

	// allow casting to mimicked type
	operator T &() { return ref; }
	operator const T &() const { return ref; }

	T		&get() { return ref; }
	const T &get() const { return ref; }
};

// template deduction guide so that T can be deduced to Q ot Q&
template <class Q>
AutoRef(Q &value) -> AutoRef<Q &>;

template <class Q>
AutoRef(Q &&value) -> AutoRef<Q>;

template <class T>
class SmartAuto {
   public:
	using Internal = std::remove_ref_and_p_t<T>;
	using type	   = T;

	template <class Q>
	SmartAuto(Q *value) : val(value) {}

	template <class Q>
		requires(!std::is_pointer_v<Q>)
	SmartAuto(Q &value) : val(&value) {}

	~SmartAuto()
		requires std::is_pointer_v<std::remove_reference_t<T>>
	{
		delete val;
	}
	~SmartAuto()
		requires(!std::is_pointer_v<std::remove_reference_t<T>>)
	{}

	SmartAuto &operator=(const T &val)
		requires(!std::is_pointer_v<T>)
	{
		this->val = &val;
		return *this;
	}
	SmartAuto &operator=(std::remove_reference_t<T> &&val)
		requires(!std::is_pointer_v<T>)
	{
		*this->val = val;
		return *this;
	}
	SmartAuto &operator=(SmartAuto &&rhs)
		requires std::is_pointer_v<T>
	{
		std::swap(val, rhs.val);
		return *this;
	}
	SmartAuto &operator=(const SmartAuto &rhs)
		requires std::is_pointer_v<T>
	= delete;
	SmartAuto &operator=(const SmartAuto &rhs)
		requires(!std::is_pointer_v<T>)
	{
		this->val = rhs.val;
		return *this;
	}

	operator T &()
		requires(!std::is_pointer_v<T>)
	{
		return *val;
	}
	operator T()
		requires std::is_pointer_v<T>
	{
		return val;
	}

	T get()
		requires std::is_pointer_v<T>
	{
		return val;
	}
	T get()
		requires(!std::is_pointer_v<T>)
	{
		return *val;
	}
	const T get() const
		requires std::is_pointer_v<T>
	{
		return val;
	}
	const T get() const
		requires(!std::is_pointer_v<T>)
	{
		return *val;
	}

	Internal	   *operator->() { return val; }
	const Internal *operator->() const { return val; }
	T			   &operator*() { return *val; }
	const T		   &operator*() const { return *val; }

   private:
	Internal *val;
};

template <class Q>
SmartAuto(Q *value) -> SmartAuto<Q *>;
template <class Q>
SmartAuto(Q *&value) -> SmartAuto<Q *>;

template <class Q>
SmartAuto(Q &value) -> SmartAuto<Q &>;

class Cloneable {
	virtual Cloneable *clone() const = 0;
};

template <class T>
class SmartRef {
   public:
	SmartRef(T &ref) : ref(&ref), isRef(true) {}
	SmartRef(T *ref) : ref(ref), isRef(false) {}
	SmartRef(const SmartRef &ref) : ref(ref.ref), isRef(true) {}
	SmartRef(SmartRef &&ref) {
		this->ref	= ref.ref;
		this->isRef = ref.isRef;
		ref.isRef	= true;
	}
	SmartRef(T &&ref)
		requires(!std::is_abstract_v<T>)
		: ref(new T(std::move(ref))), isRef(false) {}

	SmartRef(T &&ref)
		requires(std::is_abstract_v<T> && std::is_base_of_v<Cloneable, T>)
		: ref(ref.clone()), isRef(false) {}

	~SmartRef() {
		if (!isRef) { delete ref; }
	}

	T		&operator*() { return *ref; }
	const T &operator*() const { return *ref; }
	T		*operator->() { return ref; }
	const T *operator->() const { return ref; }
	T		*operator&() { return ref; }
	const T *operator&() const { return ref; }
	operator T *() { return ref; }
	operator const T *() const { return ref; }
	operator T &() { return *ref; }
	operator const T &() const { return *ref; }

	bool operator==(const SmartRef<T> &other) const { return typeid(*ref) == typeid(*other.ref) && *ref == *other.ref; }

	SmartRef &operator=(T &&ref) {
		*this->ref = std::move(ref);
		return *this;
	}

	SmartRef &operator=(T &ref) {
		*this->ref = ref;
		return *this;
	}

	SmartRef &operator=(T *ref) {
		if (!isRef) { delete this->ref; }
		this->ref = ref;
		isRef	  = false;
		return *this;
	}

	SmartRef &operator=(SmartRef &ref) {
		if(&ref == this->ref) return *this;
		if (!isRef) { delete this->ref; }
		this->ref = ref.ref;
		isRef	  = true;
		return *this;
	}

	SmartRef &operator=(SmartRef &&ref) {
		if(&ref == this->ref) return *this;
		if (!isRef) { delete this->ref; }
		this->ref = ref.ref;
		isRef	  = ref.isRef;
		ref.isRef = true;
		return *this;
	}

	bool isRef;

   private:
	T *ref;
};
