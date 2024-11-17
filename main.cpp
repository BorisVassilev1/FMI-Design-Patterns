#include <iostream>
#include "src/abstract_factory.hpp"
#include "src/figure_factory.hpp"
#include "src/commands.hpp"

int main() {
	std::string inputMethod;

	AbstractFigureFactory abstractff(std::cin, std::cout);

	auto types = abstractff.listFactoryTypes();
	std::cout << "Enter input method: [ ";
	for (const auto &type : types) {
		std::cout << type << " ";
	}
	std::cout << "]" << std::endl;

	FigureFactory *factory = nullptr;
	do {
		try {
			factory = abstractff.create();
		} catch (const std::exception &e) { std::cerr << e.what() << std::endl; }
	} while (factory == nullptr);
	std::size_t figureCount;

	std::cout << "Enter number of figures: " << std::flush;
	std::cin >> figureCount;
	std::cin.ignore();

	std::vector<Figure *> figures;

	for (std::size_t i = 0; i < figureCount; ++i) {
		try {
			figures.push_back(factory->create());
		} catch (const std::exception &e) { std::cerr << e.what() << std::endl; }
	}

	std::string command;
	do {
		try {
			std::cout << "--> " << std::flush;
			std::getline(std::cin, command);
			if(command.size() == 0) {
				std::cout << '\r';
				continue;
			}
			if (command == "list") {
				commands::list(figures, std::cin, std::cout);
			} else if (command == "store") {
				commands::store(figures, std::cin, std::cout);
			} else if (command == "remove") {
				commands::remove(figures, std::cin, std::cout);
			} else if (command == "duplicate") {
				commands::duplicate(figures, std::cin, std::cout);
			} else if (command == "exit") {
				break;
			} else {
				throw std::runtime_error("unknown command: \"" + command + "\"");
			}
		} catch (const std::exception &e) { std::cerr << "error: " << e.what() << std::endl; }
	} while (true);

	return 0;
}
