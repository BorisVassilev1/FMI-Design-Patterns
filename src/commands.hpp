#pragma once

#include <istream>
#include <fstream>
#include <stdexcept>
#include "abstract_factory.hpp"
#include "factory.hpp"
#include "figures.hpp"

namespace commands {

using Command = std::function<bool(std::vector<std::unique_ptr<Figure>> &, std::istream &, std::ostream &)>;

class CommandFactory : public LineIstreamFactory<commands::Command, "Command", false, std::string> {
   public:
	using LineIstreamFactory::LineIstreamFactory;
};

static auto list = []() -> Command * {
	return new Command([](std::vector<std::unique_ptr<Figure>> &data, std::istream &in, std::ostream &out) -> bool {
		for (std::size_t i = 0; i < data.size(); ++i) {
			out << i << ": " << (std::string)*data[i] << std::endl;
		}
		return true;
	});
};

static auto remove = [](std::string id) -> Command * {
	std::size_t index = std::stoi(id);
	return new Command(
		[index](std::vector<std::unique_ptr<Figure>> &data, std::istream &in, std::ostream &out) -> bool {
			if (index >= data.size()) { throw std::out_of_range("Index out of range"); }
			data.erase(data.begin() + index);
			std::cout << "removed " << index << std::endl;
			return true;
		});
};

static auto erase = [](std::string begin, std::string end) -> Command * {
	std::size_t i = std::stoi(begin);
	std::size_t j = std::stoi(end);
	return new Command([i, j](std::vector<std::unique_ptr<Figure>> &data, std::istream &in, std::ostream &out) -> bool {
		if (i >= data.size() || j >= data.size()) throw std::out_of_range("Indices out of range");
		if (i > j) throw std::invalid_argument("i > j");
		data.erase(data.begin() + i, data.begin() + (j + 1));
		std::cout << "removed indices [" << i << ", " << j << "]" << std::endl;
		return true;
	});
};

static auto duplicate = [](std::string id) -> Command * {
	std::size_t index = std::stoi(id);
	return new Command(
		[index](std::vector<std::unique_ptr<Figure>> &data, std::istream &in, std::ostream &out) -> bool {
			if (index >= data.size()) { throw std::out_of_range("Index out of range"); }
			data.emplace_back(data[index]->clone());
			std::cout << "duplicated " << index << std::endl;
			return true;
		});
};

static auto store = [](std::string filename) -> Command * {
	return new Command(
		[filename](std::vector<std::unique_ptr<Figure>> &data, std::istream &in, std::ostream &out) -> bool {
			std::ofstream file((filename));
			if (!file.is_open()) { throw std::runtime_error("Cannot open file"); }
			for (auto &figure : data) {
				file << (std::string)*figure << std::endl;
				if (!file) { throw std::runtime_error("Cannot write to file"); }
			}
			file.close();
			std::cout << "writen figures to \"" << filename << "\"" << std::endl;
			return true;
		});
};

static auto exit = []() -> Command * {
	return new Command(
		[](std::vector<std::unique_ptr<Figure>> &data, std::istream &in, std::ostream &out) -> bool { return false; });
};

static auto help = []() -> Command * {
	return new Command([](std::vector<std::unique_ptr<Figure>> &data, std::istream &in, std::ostream &out) -> bool {
		auto v = CommandFactory::listFactoryTypes();
		out << "Help menu: " << std::endl;
		for (const auto &[name, argcnt, desc] : v) {
			out << "    " << desc << std::endl;
		}
		return true;
	});
};

static auto clear = []() -> Command * {
	return new Command([](std::vector<std::unique_ptr<Figure>> &data, std::istream &in, std::ostream &out) -> bool {
		out << "\x1B[2J\x1B[H" << std::flush;
		return true;
	});
};

static auto load = []() -> Command * {
	return new Command([](std::vector<std::unique_ptr<Figure>> &figures, std::istream &in, std::ostream &out) -> bool {
		AbstractFigureFactory abstractff(std::cin);

		auto types = abstractff.listFactoryTypes();
		std::cout << "Enter input method: [ " << std::endl;
		for (const auto &[type, argcnt, desc] : types) {
			std::cout << "    " << desc << '\n';
		}
		std::cout << "]" << std::endl;

		std::unique_ptr<FigureFactory> factory = nullptr;
		do {
			try {
				std::cout << "==> " << std::flush;
				factory = abstractff.create();
			} catch (const std::exception &e) { std::cerr << "error: " << e.what() << std::endl; }
		} while (factory == nullptr);
		std::size_t figureCount = -1;

		while (true) {
			try {
				std::unique_ptr<Figure> f = factory->create();
				if (f == nullptr) break;
				figures.push_back(std::move(f));
			} catch (const std::exception &e) { std::cerr << "error" << e.what() << std::endl; }
		}

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
	TypeRegistry::registerConstructor("load", std::function(load), "load : load figures with a custom input method");

	TypeRegistry::registerSuccessor("Command", "list");
	TypeRegistry::registerSuccessor("Command", "remove");
	TypeRegistry::registerSuccessor("Command", "erase");
	TypeRegistry::registerSuccessor("Command", "duplicate");
	TypeRegistry::registerSuccessor("Command", "store");
	TypeRegistry::registerSuccessor("Command", "exit");
	TypeRegistry::registerSuccessor("Command", "help");
	TypeRegistry::registerSuccessor("Command", "clear");
	TypeRegistry::registerSuccessor("Command", "load");
	return 0;
});
}	  // namespace commands
