#pragma once
#include <iostream>
#include <string>

class Label {
   public:
	virtual std::string getText() const = 0;
	virtual ~Label() {}
};
class SimpleLabel : public Label {
   public:
	SimpleLabel(std::string value) { this->value = value; }
	std::string getText() const { return value; }

   private:
	std::string value;
};

class LabelPrinter {
   public:
	static void print(const Label &label, std::ostream &out = std::cout) {
		out << "Here is a label: " << label.getText();
	}
};
