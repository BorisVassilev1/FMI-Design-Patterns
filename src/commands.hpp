#pragma once


#include <istream>
#include "factory.hpp"
#include "type_registry.hpp"

class Command {

};

class CommandFactory : Factory<Command> {

	std::istream &in;
	
public:
	CommandFactory(std::istream &in) : in(in) {}

	Command *create() override {
		if (!in) { throw std::runtime_error("not enough arguments"); }
		std::string commandType;
		in >> commandType;
		if (!in) { throw std::runtime_error("not enough arguments for " + commandType); }
		const auto [argcnt, constructor] = TypeRegistry::getConstructors().at(commandType);

		std::vector<std::any> args;
		std::string arg;
		for (std::size_t i = 0; i < argcnt; ++i) {
			in >> arg;
			if (!in) { throw std::runtime_error("not enough arguments for " + commandType); }
			args.push_back(arg);
		}

		return static_cast<Command *>(constructor(args));
	}
};
