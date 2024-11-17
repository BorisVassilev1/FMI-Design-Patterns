#pragma once

#include <string>
#include <format>

class Stringifiable {
   public:
	virtual explicit operator std::string() const = 0;
	virtual ~Stringifiable() {};
};

template <class... Args>
static auto implement_stringify(Args... args) {
	return ((std::format("{}", args) + ' ') + ...);
}

#define IMPLEMENT_STRINGIFY(type, ...) \
	explicit operator std::string() const override { return std::string(#type " ") + implement_stringify(__VA_ARGS__); }
