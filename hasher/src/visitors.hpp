#pragma once
#include <FSTree.hpp>
#include <calculators.hpp>
#include "reportData.hpp"

class FileVisitor : public FSVisitor {
   public:
	void visit(const Directory &node) const override {
		for (auto &child : node.children) {
			child->accept(*this);
		}
	}
};

class ReportWriter : public FileVisitor {
   protected:
	mutable std::filesystem::path path;
	std::ostream				 &os;
	std::filesystem::path		  getRelativePath(const FSNode &node) const { return path / node.path.filename(); }

   public:
	ReportWriter(std::ostream &os) : os(os) {}

	virtual void visitDir(const Directory &node) const {
		for (auto &child : node.children) {
			child->accept(*this);
		}
	}

	void visit(const Directory &node) const override {
		if (path == "") path = std::filesystem::relative(node.path);
		else path /= node.path.filename();

		visitDir(node);

		path = path.parent_path();
	}
};

class LsWriter : public ReportWriter {
   public:
	LsWriter(std::ostream &os) : ReportWriter(os) {}

	void visit(const File &node) const override {}
	void visitDir(const Directory &node) const override {
		os << path.string() << ':' << std::endl;
		for (auto &child : node.children) {
			os << child->path.filename().string() << '\n';
		}
		os << '\n';

		for (auto &child : node.children) {
			child->accept(*this);
		}
	}
};

class TreeWriter : public ReportWriter {
	mutable std::vector<bool> b;

	void space() const {
		for (auto x : b)
			os << (x ? "  \u2502" : "   ");
	}

   public:
	TreeWriter(std::ostream &os) : ReportWriter(os) {}

	void visit(const File &node) const override {
		os << "\u2500 " << node.path.filename().string() << " : " << node.size << std::endl;
	}
	void visitDir(const Directory &node) const override {
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

class HashStreamWriter : public ReportWriter {
	ChecksumCalculator &calc;

   public:
	std::string calculateHash(const File &node) const { return calc.calculate(*node.getStream()); }
	HashStreamWriter(ChecksumCalculator &calc, std::ostream &os) : ReportWriter(os), calc(calc) {}
};

class GNUHashStreamWriter : public HashStreamWriter {
   public:
	using HashStreamWriter::HashStreamWriter;

	void visit(const File &node) const override {
		os << calculateHash(node) << " *" << getRelativePath(node).string() << std::endl;
	}
};

class JSONHashStreamWriter : public HashStreamWriter {
   public:
	JSONHashStreamWriter(ChecksumCalculator &calc, std::ostream &os) : HashStreamWriter(calc, os) {
		os << "[" << std::endl;
	}

	~JSONHashStreamWriter() { os << "]" << std::endl; }

	void visit(const File &node) const override {
		os << "\t{" << std::endl;
		os << "\t\t\"mode\": \"binary\"," << std::endl;
		os << "\t\t\"checksum\": \"" << calculateHash(node) << "\"," << std::endl;
		os << "\t\t\"path\": \"" << getRelativePath(node).string() << "\"," << std::endl;
		os << "\t\t\"size\": " << node.size << std::endl;
		os << "\t}," << std::endl;
	}
};

class ReportDataHashStreamWriter : public HashStreamWriter {
	ReportData &data;

   public:
	ReportDataHashStreamWriter(ChecksumCalculator &calc, std::ostream &os, ReportData &data)
		: HashStreamWriter(calc, os), data(data) {}

	void visit(const File &node) const override {
		std::string hash = calculateHash(node);
		data.push_back({getRelativePath(node), hash});
	}

	auto getData() { return data; }
};

using HashStreamWriterFactory = Factory<HashStreamWriter, ChecksumCalculator &, std::ostream &>;

JOB(hash_stream_writer_factory_register, {
	HashStreamWriterFactory::instance().registerType<GNUHashStreamWriter>("gnu");
	HashStreamWriterFactory::instance().registerType<JSONHashStreamWriter>("json");
});
