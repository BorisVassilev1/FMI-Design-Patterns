#pragma once
#include <cstdint>
#include <iostream>
#include <string>
#include "autoref.hpp"

class LabelImp {
   public:
	virtual std::string getText() const = 0;
	virtual ~LabelImp() {}
	virtual bool operator==(const LabelImp &other) const = default;
};

class Label {
public:
	Label(SmartRef<LabelImp> &&imp) : imp(std::move(imp)){}
	
	virtual std::string getText() const {
		return imp->getText();
	}

	Label &operator=(SmartRef<LabelImp> &&imp) {
		this->imp = std::move(imp);
		return *this;
	}

	template<class Decorator, class ... Args>
	Label& addDecorator(Args&& ... args) {
		Decorator* d = new Decorator(&imp, std::forward<Args>(args)...);
		SmartRef<LabelImp> l = std::move(imp);
		this->imp = d;
		l.isRef = true;
		return *this;
	}

	SmartRef<LabelImp> &getImp() { return imp; }

private:
	SmartRef<LabelImp> imp;
};

class HelpLabel : public Label {
	std::string helpText;

   public:
	HelpLabel(SmartRef<LabelImp> &&imp, const std::string &helpText) : Label(std::move(imp)), helpText(helpText) {}

	virtual std::string getHelpText() const { return helpText; }
};

class SimpleLabel : public LabelImp {
   public:
	SimpleLabel(std::string value) : value(value) {}
	std::string getText() const override { return value; }

   private:
	std::string value;
};

struct Color {
	std::uint8_t r, g, b;
};

class RichLabel : public LabelImp {
   public:
	RichLabel(const std::string &value, Color c, const std::string &font = "Arial", uint8_t size = 13)
		: value(value), c(c), font(font), size(size) {}

   private:
	std::string value;
	std::string font;
	Color		c;
	uint8_t		size;

	std::string getText() const override { return value; }
};

class ProxyLabel : public LabelImp {
	mutable std::string text;
	mutable std::size_t i		= 0;
	std::size_t			timeout = -1;
	std::istream	   &in;

   public:
	ProxyLabel(std::istream &in, std::size_t timeout = -1) : in(in), timeout(timeout) {}

	std::string getText() const override {
		if (i == 0) {
			in >> this->text;
			i = timeout;
		}
		--i;
		return text;
	}
};

class LabelPrinter {
   public:
	static void print(const Label &label, std::ostream &out = std::cout) {
		out << "Here is a label: " << label.getText() << std::endl;
	}

	static void printWithHelpText(const HelpLabel &label, std::ostream &out = std::cout) {
		print(label);
		out << "Some help information about this label: " << label.getHelpText() << std::endl;
	}
};
