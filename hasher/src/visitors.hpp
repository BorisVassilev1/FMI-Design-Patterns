#pragma once
#include <FSTree.hpp>
#include <calculators.hpp>

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

class ReportWriter : public FileVisitor {
   protected:
	std::ostream &os;

   public:
	ReportWriter(std::ostream &os) : os(os) {}
};

class HashStreamWriter : public ReportWriter {
	ChecksumCalculator &calc;

   public:
	std::string calculateHash(const File &node) const { return calc.calculate(*node.getStream()); }
	HashStreamWriter(ChecksumCalculator &calc, std::ostream &os) : ReportWriter(os), calc(calc){}
};

class GNUHashStreamWriter : public HashStreamWriter {
   public:
	using HashStreamWriter::HashStreamWriter;

	void visit(const File &node) const override {
		os << calculateHash(node) << " *" << std::filesystem::relative(node.path).string() << std::endl;
	}
};

class XMLHashStreamWriter : public HashStreamWriter {
   public:
	XMLHashStreamWriter(ChecksumCalculator &calc, std::ostream &os) : HashStreamWriter(calc, os) {
		os << "<checksums>" << std::endl;
	}

	~XMLHashStreamWriter() { os << "</checksums>" << std::endl; }

	void visit(const File &node) const override {
		os << "  <item>" << std::endl;
		os << "    <mode>binary</mode>" << std::endl;
		os << "    <checksum>" << calculateHash(node) << "</checksum>" << std::endl;
		os << "    <path>" << std::filesystem::relative(node.path).string() << "</path>" << std::endl;
		os << "    <size>" << node.size << "</size>" << std::endl;
		os << "  </item>" << std::endl;
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
		os << "\t\t\"path\": \"" << std::filesystem::relative(node.path).string() << "\"," << std::endl;
		os << "\t\t\"size\": " << node.size << std::endl;
		os << "\t}," << std::endl;
	}
};

using HashStreamWriterFactory = Factory<HashStreamWriter, ChecksumCalculator&, std::ostream &>;

JOB(hash_stream_writer_factory_register, {
	HashStreamWriterFactory::instance().registerType<GNUHashStreamWriter>("gnu");
	HashStreamWriterFactory::instance().registerType<XMLHashStreamWriter>("xml");
	HashStreamWriterFactory::instance().registerType<JSONHashStreamWriter>("json");
});

class LsWriter : public ReportWriter {
	mutable std::filesystem::path path;
   public:
	LsWriter(std::ostream &os) : ReportWriter(os) {}

	void visit(const File &node) const override {}
	void visit(const Directory &node) const override {
		if(path == "") path = std::filesystem::relative(node.path);
		else path /= node.path.filename();
		os << path.string() << ':' << std::endl;
		for(auto &child : node.children) {
			os << child->path.filename().string() << '\n';
		}
		os << '\n';

		for (auto &child : node.children) {
			child->accept(*this);
		}
		path = path.parent_path();
	}
};
