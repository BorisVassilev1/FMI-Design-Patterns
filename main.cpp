#include <iostream>
#include "src/abstract_factory.hpp"
#include "src/figure_factory.hpp"

int main() {
	std::string inputMethod;

	auto types = AbstractFigureFactory().listFactoryTypes();
	std::cout << "Enter input method: [";
	for(const auto &type : types) {
		std::cout << type << " ";
	}
	std::cout << "]" << std::endl;

	FigureFactory *factory = nullptr;
	do {
		try {
			factory = AbstractFigureFactory().create(std::cin, std::cout);
		} catch (const std::exception &e) { std::cerr << e.what() << std::endl; }
	} while (factory == nullptr);
	std::size_t figureCount;
	std::cout << "Enter number of figures: " << std::flush;
	std::cin >> figureCount;

	std::vector<Figure *> figures;

	for (std::size_t i = 0; i < figureCount; ++i) {
		figures.push_back(factory->create());
		std::cout << (std::string)*figures.back() << std::endl;
	}

	return 0;
}
