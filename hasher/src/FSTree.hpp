#pragma once

#include <filesystem>
#include <memory>
#include <vector>
class FSNode {
   public:
	std::filesystem::path path;

	FSNode(const std::filesystem::path &path) : path(path) {}
	virtual ~FSNode() = default;
	void print(std::ostream &os) const { print(os, {}); }

	// protected:
	virtual void print(std::ostream &os, std::vector<bool> b) const = 0;
};

class File : public FSNode {
   public:
	File(const std::filesystem::path &path) : FSNode(path) {}

   protected:
	virtual void print(std::ostream &os, std::vector<bool> b) const override {
		os << "\u2500\u2500 " << path.filename().string() << std::endl;
	}
};

class Directory : public FSNode {
   public:
	Directory(const std::filesystem::path &path) : FSNode(path) {}
	std::vector<std::unique_ptr<FSNode>> children;

   protected:
	virtual void print(std::ostream &os, std::vector<bool> b) const override {
		os << "\u2500\u2500 " << path.filename().string() << std::endl;
		if (children.empty()) return;
		for (int i = 0; i + 1 < children.size(); i++) {
			for (auto x : b)
				os << (x ? "   \u2502" : "    ");
			b.push_back(true);
			os << "   \u251c";
			children[i]->print(os, b);
			b.pop_back();
		}
		for (auto x : b)
			os << (x ? "   \u2502" : "    ");
		b.push_back(false);
		os << "   \u2514";
		children.back()->print(os, b);
		b.pop_back();
	}
};

class FSTreeBuilder {
   public:
	virtual std::unique_ptr<FSNode> build(const std::filesystem::path &path) = 0;
};

class FSTreeBuilderNoLinks : public FSTreeBuilder {
   public:
	std::unique_ptr<FSNode> build(const std::filesystem::path &path) override {
		if (std::filesystem::is_symlink(path)) { return nullptr; }
		if (path.filename().string()[0] == '.') { return nullptr; }
		if (std::filesystem::is_directory(path)) {
			auto dir = std::make_unique<Directory>(path);
			for (auto &entry : std::filesystem::directory_iterator(path)) {
				auto child = build(entry.path());
				if (child) { dir->children.push_back(std::move(child)); }
			}
			return dir;
		} else {
			return std::make_unique<File>(path);
		}
	}
};
