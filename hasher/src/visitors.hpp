#pragma once
#include <FSTree.hpp>
#include <calculators.hpp>
#include <observe.hpp>
#include <reportData.hpp>
#include "nlohmann/json.hpp"

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

class HashStreamWriter : public ReportWriter,
						 public ForwardObservable<std::uintmax_t>,
						 public BasicObservable<std::filesystem::path> {
   protected:
	ChecksumCalculator &calc;

   public:
	std::string calculateHash(const File &node) const {
		BasicObservable<std::filesystem::path>::notifyObservers(node.path);
		return calc.calculate(*node.getStream());
	}
	HashStreamWriter(ChecksumCalculator &calc, std::ostream &os)
		: ReportWriter(os), ForwardObservable<std::uintmax_t>(&calc), calc(calc) {}
};

class GNUHashStreamWriter : public HashStreamWriter {
   public:
	using HashStreamWriter::HashStreamWriter;

	void visit(const File &node) const override {
		os << calculateHash(node) << " *" << getRelativePath(node).string() << std::endl;
	}
};

class JSONHashStreamWriter : public HashStreamWriter {
	mutable nlohmann::json j;

   public:
	JSONHashStreamWriter(ChecksumCalculator &calc, std::ostream &os)
		: HashStreamWriter(calc, os), j(nlohmann::json::array()) {}

	~JSONHashStreamWriter() { os << j; }

	void visit(const File &node) const override {
		j.emplace_back(nlohmann::json{{"mode", "binary"},
									  {"checksum", calculateHash(node)},
									  {"path", getRelativePath(node).string()},
									  {"size", node.size}});
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
