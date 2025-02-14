#pragma once

#include <filesystem>
#include <memory>
#include <vector>
#include <fstream>

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
	virtual std::unique_ptr<std::istream> getStream() const = 0;
};

class RegularFile : public File {
   public:
	RegularFile(const std::filesystem::path &path) : File(path, std::filesystem::file_size(path)) {}

	std::unique_ptr<std::istream> getStream() const override {
		return std::unique_ptr<std::istream>(new std::ifstream(path, std::ios::binary));
	}

	void accept(const FSVisitor &v) override { v.visit(*this); }
	void accept(const FSVisitor &v) const override { v.visit(*this); }
};

class SymLink : public File {
   public:
	std::filesystem::path target;
	SymLink(const std::filesystem::path &path) : File(path, 0), target(std::filesystem::read_symlink(path)) {
		size = target.native().size();
	}

	std::unique_ptr<std::istream> getStream() const override {
		return std::unique_ptr<std::istream>(new std::istringstream(target.native()));
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
		if(!exists(path) && !is_symlink(path)) throw std::runtime_error("path does not exist: " + path.string());
		if (path.filename().string()[0] == '.') { return nullptr; }
		if (is_directory(path) && !is_symlink(path)) {
			std::vector<std::unique_ptr<FSNode>> children;
			for (auto &entry : directory_iterator(path)) {
				auto child = build(entry.path());
				if (child) { children.push_back(std::move(child)); }
			}
			return std::make_unique<Directory>(path, std::move(children));
		} else {
			if (is_symlink(path)) return std::make_unique<SymLink>(path);
			else return std::make_unique<RegularFile>(path);
		}
	}
};

class FSTreeBuilderWithLinks : public FSTreeBuilder {
	static const std::size_t max_path_size = 4096;

   public:
	std::unique_ptr<FSNode> build(const std::filesystem::path &path) override {
		if(path.native().size() > max_path_size)
			throw std::runtime_error("reached a path that is too long");

		using namespace std::filesystem;
		if(!exists(path) && !is_symlink(path)) throw std::runtime_error("path does not exist: " + path.string());
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


