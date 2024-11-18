#pragma once

#include <memory>
#include <type_traits>
#include <cxxabi.h>

#define JOB(name, code)                    \
	static int _job_##name = []() -> int { \
		code;                              \
		return 0;                          \
	}();

template <auto N>
struct string_literal {
	constexpr string_literal(const char (&str)[N]) { std::ranges::copy_n(str, N, value); }

	char value[N];

	constexpr operator const char *() const { return value; }
};

template <class T>
auto f_name() {
	typedef typename std::remove_reference<T>::type TR;

	std::unique_ptr<char, void (*)(void *)> own(abi::__cxa_demangle(typeid(TR).name(), nullptr, nullptr, nullptr),
												std::free);

	std::string r = own != nullptr ? own.get() : typeid(TR).name();
	if (std::is_const<TR>::value) r += " const";
	if (std::is_volatile<TR>::value) r += " volatile";
	if (std::is_lvalue_reference<T>::value) r += "&";
	else if (std::is_rvalue_reference<T>::value) r += "&&";
	return r;
}

inline auto f_demangle(const char *n) {
	std::unique_ptr<char, void (*)(void *)> own(abi::__cxa_demangle(n, nullptr, nullptr, nullptr), std::free);
	std::string								r = own != nullptr ? own.get() : n;
	return r;
}
