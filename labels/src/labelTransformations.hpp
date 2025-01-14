#pragma once
#include <algorithm>
#include <cctype>
#include <functional>
#include <string>
#include <string>
#include <unordered_set>
#include "autoref.hpp"
#include "label.hpp"
#include "utils.hpp"

class LabelTransformation {
   public:
	virtual std::string apply(std::string &&text) const = 0;
	virtual ~LabelTransformation() {}
	virtual bool operator==(const LabelTransformation &other) const = default;
};

class CapitalizeTransformation : public LabelTransformation {
   public:
	std::string apply(std::string &&text) const override {
		if (std::islower(text[0])) { text[0] = std::toupper(text[0]); }
		return text;
	}
};

static std::function<bool(char)> isspace_f = [](char c) { return std::isspace(c); };

class LeftTrimTransformation : public LabelTransformation {
   public:
	std::string apply(std::string &&text) const override {
		auto i = std::find_if(text.begin(), text.end(), std::not_fn(isspace_f));
		if (i != text.end()) text.erase(0, i - text.begin());
		return text;
	}
};

class RightTrimTransformation : public LabelTransformation {
   public:
	std::string apply(std::string &&text) const override {
		auto i = std::find_if(text.rbegin(), text.rend(), std::not_fn(isspace_f));
		if (i != text.rend()) text.erase(text.rend() - i, text.size());
		return text;
	}
};

class NormalizeSpaceTransformation : public LabelTransformation {
   public:
	std::string apply(std::string &&text) const override {
		text.erase(std::unique(text.begin(), text.end(), [](char i, char j) { return i == j && i == ' '; }),
				   text.end());
		return text;
	}
};

class DecorateTransformation : public LabelTransformation {
   public:
	std::string apply(std::string &&text) const override {
		text.insert(0, "-={ ");
		text.append(" }=-");
		return text;
	}
};

class CensorTransformationBase : public LabelTransformation {
   public:
	virtual std::string_view getWord() const = 0;

	std::string apply(std::string &&text) const override {
		std::string_view word = getWord();

		std::size_t curr = 0, next = text.find(word, curr);
		while ((curr = next) != std::string::npos) {
			next = text.find(word, curr + 1);
			for (std::size_t i = 0; curr != next && i < word.size(); ++i, ++curr)
				text[curr] = '*';
		}
		return text;
	}

	bool operator==(const LabelTransformation &other) const noexcept override {
		if (const CensorTransformationBase *t = dynamic_cast<const CensorTransformationBase *>(&other)) {
			return this->getWord() == t->getWord();
		}
		return false;
	}
};

class CensorTransformation : public CensorTransformationBase {
	std::string word;

   public:
	template <class stringType>
	CensorTransformation(stringType &&word) : word(word) {}

	std::string_view getWord() const noexcept override {
		return word;
	}
};

class ReplaceTransformation : public LabelTransformation {
	std::string A, B;

   public:
	template <class stringType1, class stringType2>
	ReplaceTransformation(stringType1 &&A, stringType2 &B) : A(A), B(B) {}

	std::string apply(std::string &&text) const override {
		std::size_t pos = 0;
		while ((pos = text.find(A, pos)) != std::string::npos) {
			text.replace(pos, A.length(), B);
			pos += B.length();
		}
		return text;
	}

	bool operator==(const LabelTransformation &other) const noexcept override {
		if (const ReplaceTransformation *t = dynamic_cast<const ReplaceTransformation *>(&other)) {
			return A == t->A && B == t->B;
		}
		return false;
	}
};

class CompositeTransformation : public LabelTransformation {
	std::vector<SmartRef<LabelTransformation>> ts;

   public:
	CompositeTransformation(std::vector<SmartRef<LabelTransformation>> &&ts) : ts(std::move(ts)) {};

	std::string apply(std::string &&text) const override {
		for (auto &t : ts) {
			text = std::move(t->apply(std::move(text)));
		}
		return text;
	}

	bool operator==(const LabelTransformation &other) const noexcept override {
		if (const CompositeTransformation *t = dynamic_cast<const CompositeTransformation *>(&other)) {
			return ts == t->ts;
		}
		return false;
	}
};

class CensorTransformationFactory {
	std::unordered_set<std::string> cache;

	class ProxyTransformation : public CensorTransformationBase {
		SmartRef<const std::string> word;

	   public:
		ProxyTransformation(const std::string &word) : word(word) {}
		ProxyTransformation(const std::string *word) : word(std::move(word)) {}

		using CensorTransformationBase::operator==;

		std::string_view getWord() const noexcept override {
			return *word;
		}
	};

   public:
	template <class stringType>
	SmartRef<LabelTransformation> create(stringType &&word) {
		std::string curr = std::forward<stringType>(word);
		if (curr.size() <= 4) {
			auto it = cache.find(curr);
			if (it != cache.end()) {
				return new ProxyTransformation(*it);
			} else {
				auto [i, b] = cache.insert(std::move(curr));
				return new ProxyTransformation(*i);
			}
		} else {
		  return new CensorTransformation(std::move(curr));
		}
	}
};
