#pragma once

#include <istream>
#include <fstream>
#include <stdexcept>
#include "factory.hpp"
#include "figures.hpp"

namespace commands {

using Command = std::function<bool(std::vector<Figure *> &, std::istream &, std::ostream &)>;

class CommandFactory : public LineIstreamFactory<commands::Command, "Command", false, std::string> {
   public:
	using LineIstreamFactory::LineIstreamFactory;
};

static auto list = []() -> Command * {
	return new Command([](std::vector<Figure *> &data, std::istream &in, std::ostream &out) -> bool {
		for (std::size_t i = 0; i < data.size(); ++i) {
			out << i << ": " << (std::string)*data[i] << std::endl;
		}
		return true;
	});
};

static auto remove = [](std::string id) -> Command * {
	std::size_t index = std::stoi(id);
	return new Command([index](std::vector<Figure *> &data, std::istream &in, std::ostream &out) -> bool {
		if (index >= data.size()) { throw std::out_of_range("Index out of range"); }
		delete data[index];
		data.erase(data.begin() + index);
		std::cout << "removed " << index << std::endl;
		return true;
	});
};

static auto erase = [](std::string begin, std::string end) -> Command * {
	std::size_t i = std::stoi(begin);
	std::size_t j = std::stoi(end);
	return new Command([i, j](std::vector<Figure *> &data, std::istream &in, std::ostream &out) -> bool {
		if (i >= data.size() || j >= data.size()) throw std::out_of_range("Indices out of range");
		if (i > j) throw std::invalid_argument("i > j");
		data.erase(data.begin() + i, data.begin() + (j + 1));
		std::cout << "removed indices [" << i << ", " << j << "]" << std::endl;
		return true;
	});
};

static auto duplicate = [](std::string id) -> Command * {
	std::size_t index = std::stoi(id);
	return new Command([index](std::vector<Figure *> &data, std::istream &in, std::ostream &out) -> bool {
		if (index >= data.size()) { throw std::out_of_range("Index out of range"); }
		data.push_back(data[index]->clone());
		std::cout << "duplicated " << index << std::endl;
		return true;
	});
};

static auto store = [](std::string filename) -> Command * {
	return new Command([filename](std::vector<Figure *> &data, std::istream &in, std::ostream &out) -> bool {
		std::ofstream file((filename));
		if (!file.is_open()) { throw std::runtime_error("Cannot open file"); }
		for (auto *figure : data) {
			file << (std::string)*figure << std::endl;
			if (!file) { throw std::runtime_error("Cannot write to file"); }
		}
		file.close();
		std::cout << "writen figures to \"" << filename << "\"" << std::endl;
		return true;
	});
};

static auto exit = []() -> Command * {
	return new Command([](std::vector<Figure *> &data, std::istream &in, std::ostream &out) -> bool { return false; });
};

static auto help = []() -> Command * {
	return new Command([](std::vector<Figure *> &data, std::istream &in, std::ostream &out) -> bool {
		auto v = CommandFactory::listFactoryTypes();
		out << "Help menu: " << std::endl;
		for (const auto &[name, argcnt, desc] : v) {
			out << "    " << desc << std::endl;
		}
		return true;
	});
};

static auto clear = []() -> Command * {
	return new Command([](std::vector<Figure *> &data, std::istream &in, std::ostream &out) -> bool {
		out << "\x1B[2J\x1B[H" << std::flush;
		return true;
	});
};

JOB(register_commands, {
	TypeRegistry::registerConstructor("list", std::function(list), "list :  all figures");
	TypeRegistry::registerConstructor("remove", std::function(remove), "remove i  : remove the figure at index i");
	TypeRegistry::registerConstructor("erase", std::function(erase),
									  "erase i j  : remove all figures i the index range [i, j]");
	TypeRegistry::registerConstructor("duplicate", std::function(duplicate),
									  "duplicate i  : duplicate figure with index i");
	TypeRegistry::registerConstructor("store", std::function(store), "store fileName  : write all figures to file");
	TypeRegistry::registerConstructor("exit", std::function(exit), "exit : exit the shell");
	TypeRegistry::registerConstructor("help", std::function(help), "help : display help information");
	TypeRegistry::registerConstructor("clear", std::function(clear), "clear : clears the screen");

	TypeRegistry::registerSuccessor("Command", "list");
	TypeRegistry::registerSuccessor("Command", "remove");
	TypeRegistry::registerSuccessor("Command", "erase");
	TypeRegistry::registerSuccessor("Command", "duplicate");
	TypeRegistry::registerSuccessor("Command", "store");
	TypeRegistry::registerSuccessor("Command", "exit");
	TypeRegistry::registerSuccessor("Command", "help");
	TypeRegistry::registerSuccessor("Command", "clear");
	return 0;
});
}	  // namespace commands
