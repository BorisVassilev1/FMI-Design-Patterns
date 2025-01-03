#pragma once
#include <algorithm>
#include <any>
#include <cctype>
#include <functional>
#include <iostream>
#include <ranges>
#include <string>
#include <string>

class LabelTransformation {
   public:
	virtual std::string apply(const std::string &text) const = 0;
};

class Capitalize : public LabelTransformation {
   public:
	std::string apply(const std::string &text) const override {
		std::string res = std::string(text);
		if (std::islower(res[0])) { res[0] = std::toupper(res[0]); }
		return res;
	}
};

static std::function<bool(char)> isspace_f = [](char c) { return std::isspace(c); };

class LeftTrim : public LabelTransformation {
   public:
	std::string apply(const std::string &text) const override {
		auto i = std::find_if(text.begin(), text.end(), std::not_fn(isspace_f));
		if (i != text.end()) return std::string(text.substr(i - text.begin()));
		else return std::string(text);
	}
};

class RightTrim : public LabelTransformation {
   public:
	std::string apply(const std::string &text) const override {
		auto i = std::find_if(text.rbegin(), text.rend(), std::not_fn(isspace_f));
		if (i != text.rend()) return std::string(text.substr(0, text.rend() - i));
		else return std::string(text);
	}
};

class NormalizeSpace : public LabelTransformation {
	std::string apply(const std::string &text) const override {

	}
};
