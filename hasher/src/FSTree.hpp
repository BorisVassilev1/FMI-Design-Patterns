#pragma once

#include <filesystem>
#include <iostream>
#include <memory>
#include <numeric>
#include <vector>

class File;
class Directory;

class FSVisitor {
   public:
	virtual void visit(const File &node) const		= 0;
	virtual void visit(const Directory &node) const = 0;
	virtual ~FSVisitor()							= default;
};

class FSAcceptor {
   public:
	virtual void accept(const FSVisitor &v)		  = 0;
	virtual void accept(const FSVisitor &v) const = 0;
	virtual ~FSAcceptor()						  = default;
};

class FSNode : public FSAcceptor {
   public:
	std::filesystem::path path;
	std::uintmax_t		  size;

	FSNode(const std::filesystem::path &path, std::uintmax_t size) : path(path), size(size) {}
	virtual ~FSNode() = default;
};

class File : public FSNode {
   public:
	File(const std::filesystem::path &path, std::uintmax_t size) : FSNode(path, size) {}
};

class RegularFile : public File {
   public:
	RegularFile(const std::filesystem::path &path) : File(path, std::filesystem::file_size(path)) {}

	void accept(const FSVisitor &v) override { v.visit(*this); }
	void accept(const FSVisitor &v) const override { v.visit(*this); }
};

class SymLink : public File {
   public:
	std::filesystem::path target;
	SymLink(const std::filesystem::path &path) : File(path, 0), target(std::filesystem::read_symlink(path)) {
		size = target.native().size();
	}

	void accept(const FSVisitor &v) override { v.visit(*this); }
	void accept(const FSVisitor &v) const override { v.visit(*this); }
};

class Directory : public FSNode {
   public:
	std::vector<std::unique_ptr<FSNode>> children;

	Directory(const std::filesystem::path &path, std::vector<std::unique_ptr<FSNode>> &&children)
		: FSNode(path, 0), children(std::move(children)) {
		for (const auto &child : this->children)
			size += child->size;
	}

	void accept(const FSVisitor &v) override { v.visit(*this); }
	void accept(const FSVisitor &v) const override { v.visit(*this); }
};

class FSTreeBuilder {
   public:
	virtual std::unique_ptr<FSNode> build(const std::filesystem::path &path) = 0;
	virtual ~FSTreeBuilder()												 = default;
};

class FSTreeBuilderNoLinks : public FSTreeBuilder {
   public:
	std::unique_ptr<FSNode> build(const std::filesystem::path &path) override {
		using namespace std::filesystem;
		if (path.filename().string()[0] == '.') { return nullptr; }
		if (is_directory(path) && !is_symlink(path)) {
			std::vector<std::unique_ptr<FSNode>> children;
			for (auto &entry : directory_iterator(path)) {
				auto child = build(entry.path());
				if (child) { children.push_back(std::move(child)); }
			}
			return std::make_unique<Directory>(path, std::move(children));
		} else {
			if (exists(path))
				if (is_symlink(path)) return std::make_unique<SymLink>(path);
				else return std::make_unique<RegularFile>(path);
			else return nullptr;
		}
	}
};

class FSTreeBuilderWithLinks : public FSTreeBuilder {
   public:
	std::unique_ptr<FSNode> build(const std::filesystem::path &path) override {
		using namespace std::filesystem;
		if (path.filename().string()[0] == '.') { return nullptr; }
		if (is_directory(path)) {
			std::vector<std::unique_ptr<FSNode>> children;
			for (auto &entry : directory_iterator(path, directory_options::follow_directory_symlink)) {
				auto child = build(entry.path());
				if (child) { children.push_back(std::move(child)); }
			}
			return std::make_unique<Directory>(path, std::move(children));
		} else {
			if (exists(path)) {
				return std::make_unique<RegularFile>(path);
			} else return nullptr;
		}
	}
};

class FSTreePrinter : public FSVisitor {
	std::ostream			 &os;
	mutable std::vector<bool> b;

   public:
	FSTreePrinter(std::ostream &os) : os(os) {}

	void visit(const File &node) const override {
		os << "\u2500 " << node.path.filename().string() << " : " << node.size << std::endl;
	}
	void visit(const Directory &node) const override {
		os << "\u2500<" << node.path.filename().string() << " : " << node.size << std::endl;
		if (node.children.empty()) return;
		for (int i = 0; i + 1 < node.children.size(); i++) {
			for (auto x : b)
				os << (x ? "  \u2502" : "   ");
			b.push_back(true);
			os << "  \u251c";
			node.children[i]->accept(*this);
			b.pop_back();
		}
		for (auto x : b)
			os << (x ? "  \u2502" : "   ");
		b.push_back(false);
		os << "  \u2514";
		node.children.back()->accept(*this);
		b.pop_back();
	}
};
