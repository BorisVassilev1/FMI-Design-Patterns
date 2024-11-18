#include <iostream>
#include "src/abstract_factory.hpp"
#include "src/figure_factory.hpp"
#include "src/commands.hpp"

int main() {
	std::string inputMethod;

	AbstractFigureFactory abstractff(std::cin);

	auto types = abstractff.listFactoryTypes();
	std::cout << "Enter input method: [ " << std::endl;
	for (const auto &[type, argcnt, desc] : types) {
		std::cout << "    " << desc << '\n';
	}
	std::cout << "]" << std::endl;

	FigureFactory *factory = nullptr;
	do {
		try {
			std::cout << "==> " << std::flush;
			factory = abstractff.create();
		} catch (const std::exception &e) { std::cerr << "error: " << e.what() << std::endl; }
	} while (factory == nullptr);
	std::size_t figureCount = -1;

	std::vector<Figure *> figures;

	while (true) {
		try {
			Figure *f = factory->create();
			if (f == nullptr) break;
			figures.push_back(f);
		} catch (const std::exception &e) { std::cerr << e.what() << std::endl; }
	}

	std::string				 command;
	commands::CommandFactory cf(std::cin);
	bool					 result = true;

	do {
		try {
			std::cout << (result ? "--> " : "-<< ") << std::flush;
			commands::Command *cmd = cf.create();
			if (cmd == nullptr) continue;

			bool res = (*cmd)(figures, std::cin, std::cout);
			if (!res) break;
			result = true;
		} catch (const std::exception &e) {
			std::cerr << "error: " << e.what() << std::endl;
			result = false;
		}
	} while (true);

	return 0;
}
