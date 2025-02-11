#pragma once
#include <FSTree.hpp>
#include "calculators.hpp"

class FSTreePrinter : public FSVisitor {
	std::ostream			 &os;
	mutable std::vector<bool> b;

	void space() const {
		for (auto x : b)
			os << (x ? "  \u2502" : "   ");
	}

   public:
	FSTreePrinter(std::ostream &os) : os(os) {}

	void visit(const File &node) const override {
		os << "\u2500 " << node.path.filename().string() << " : " << node.size << std::endl;
	}
	void visit(const Directory &node) const override {
		os << "\u2500<" << node.path.filename().string() << " : " << node.size << std::endl;
		if (node.children.empty()) return;
		for (int i = 0; i + 1 < node.children.size(); i++) {
			space();
			b.push_back(true);
			os << "  \u251c";
			node.children[i]->accept(*this);
			b.pop_back();
		}
		space();
		b.push_back(false);
		os << "  \u2514";
		node.children.back()->accept(*this);
		b.pop_back();
	}
};

class FileVisitor : public FSVisitor {
   public:
	void visit(const Directory &node) const override {
		for (auto &child : node.children) {
			child->accept(*this);
		}
	}
};

class HashStreamWriter : public FileVisitor {
	ChecksumCalculator &calc;

   public:
	HashStreamWriter(ChecksumCalculator &calc) : calc(calc) {}

	void visit(const File &node) const override {
		std::unique_ptr<std::istream> file = node.getStream();
		std::cout << calc.calculate(*file) << " *" << node.path.native() << std::endl;
	}
};

class ReportWriter : public FileVisitor {
	std::ostream &os;

   public:
	ReportWriter(std::ostream &os) : os(os) {}

	void visit(const File &node) const override { os << node.path << " : " << node.size << std::endl; }
};
