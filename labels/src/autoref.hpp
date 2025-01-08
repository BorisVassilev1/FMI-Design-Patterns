#pragma once

#include <type_traits>
#include <utility>

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
	using Internal = std::remove_pointer_t<std::remove_reference_t<T>>;
	using type = T;

	template <class Q>
	SmartAuto(Q *value) : val(value) {}
	// template <class Q>
	// SmartAuto(Q &&value) : val(value) {}
	template <class Q>
	SmartAuto(Q &value) : val(&value) {}

	~SmartAuto()
		requires std::is_pointer_v<T>
	{
		delete val;
	}
	~SmartAuto()
		requires(!std::is_pointer_v<T>)
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

   private:
	Internal *val;
};

template <class Q>
SmartAuto(const Q &value) -> SmartAuto<const Q &>;
template <class Q>
SmartAuto(Q &value) -> SmartAuto<Q &>;
template <class Q>
SmartAuto(Q *value) -> SmartAuto<Q *>;
