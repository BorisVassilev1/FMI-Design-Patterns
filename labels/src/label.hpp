#pragma once
#include <cstdint>
#include <iostream>
#include <string>

class Label {
   public:
	virtual std::string getText() const = 0;
	virtual ~Label() {}
	virtual bool operator ==(const Label &other) const = default;
};
class SimpleLabel : public Label {
   public:
	SimpleLabel(std::string value) : value(value) {}
	std::string getText() const override { return value; }

   private:
	std::string value;
};

struct Color {
	std::uint8_t r, g, b;
};

class RichLabel : public SimpleLabel {
   public:
	RichLabel(std::string &value, Color c, const std::string &font = "Arial", uint8_t size = 13)
		: SimpleLabel(value), c(c), font(font), size(size) {}

   private:
	std::string value;
	std::string font;
	Color		c;
	uint8_t		size;

	std::string getText() const override {return value;}
};

class LabelPrinter {
   public:
	static void print(const Label &label, std::ostream &out = std::cout) {
		out << "Here is a label: " << label.getText();
	}
};
