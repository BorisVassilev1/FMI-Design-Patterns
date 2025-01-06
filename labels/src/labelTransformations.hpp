#pragma once
#include <algorithm>
#include <functional>
#include <string>
#include <string>

class LabelTransformation {
   public:
	virtual std::string apply(std::string &&text) const = 0;
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

class CensorTransformation : public LabelTransformation {
	std::string word;

   public:
	template <class stringType>
	CensorTransformation(stringType &&word) : word(word) {}

	std::string apply(std::string &&text) const override {
		std::size_t curr = 0, next = text.find(word, curr);
		while ((curr = next) != std::string::npos) {
			next = text.find(word, curr+1);
			for (std::size_t i = 0; curr != next && i < word.size(); ++i, ++curr)
				text[curr] = '*';
		}
		return text;
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
};
