#include <iostream>
#include "src/commands.hpp"

int main() {
	std::string inputMethod;

	std::vector<std::unique_ptr<Figure>> figures;

	auto load = std::make_unique<commands::Command>();
	load.reset(commands::load());
	(*load)(figures, std::cin, std::cout);

	std::string				 command;
	commands::CommandFactory cf(std::cin);
	bool					 lastCommandSuccess = true;

	do {
		try {
			std::cout << (lastCommandSuccess ? "--> " : "-<< ") << std::flush;
			std::unique_ptr<commands::Command> cmd = cf.create();
			if (cmd == nullptr) continue;

			bool shouldBreak = (*cmd)(figures, std::cin, std::cout);
			if (!shouldBreak) break;
			lastCommandSuccess = true;
		} catch (const std::exception &e) {
			std::cerr << "error: " << e.what() << std::endl;
			lastCommandSuccess = false;
		}
	} while (true);

	return 0;
}
