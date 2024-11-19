#pragma once

#include <cstring>
#include <iostream>
#include <fstream>
#include <sstream>
#include <memory>

#include "figures.hpp"
#include "factory.hpp"
#include "type_registry.hpp"

using FigureFactory = Factory<Figure>;

INHERIT(FigureFactory, RandomFigureFactory);
class RandomFigureFactory : public FigureFactory {
	std::size_t figure_count;

   public:
	RandomFigureFactory(std::size_t figure_count = -1) : figure_count(figure_count) {}

	std::unique_ptr<Figure> create() override {
		if (figure_count == 0) return nullptr;
		const auto &children = TypeRegistry::getChildren().at("Figure");
		int			index	 = rand() % children.size();

		const std::string &figureName	 = children[index];
		const auto [argcnt, _, constructor] = TypeRegistry::getConstructors().at(figureName);

		std::vector<std::any> args;
		for (std::size_t i = 0; i < argcnt; ++i) {
			args.push_back(float(rand() % 50 / 10. + 5));
		}

		auto res = constructor(args);
		if(figure_count != -1) --figure_count;
		std::unique_ptr<Figure> ptr;
		ptr.reset(static_cast<Figure*>(res));
		return ptr;
	}
};

INHERIT(FigureFactory, IstreamFigureFactory);
class IstreamFigureFactory : public IstreamFactory<Figure, "Figure", false, float> {
	using IstreamFactory::IstreamFactory;
};

inline std::unique_ptr<Figure> figureFromString(const std::string &s) {
	std::stringstream	 ss(s);
	IstreamFigureFactory ff(ss);
	return ff.create();
}

INHERIT(IstreamFigureFactory, STDINFigureFactory);
class STDINFigureFactory : public IstreamFigureFactory {
	std::size_t figure_count;
   public:
	STDINFigureFactory(std::size_t figure_count) : IstreamFigureFactory(std::cin), figure_count(figure_count) {}
	std::unique_ptr<Figure> create() override {
		if(!figure_count) return nullptr;
		if(figure_count != -1) --figure_count;
		return IstreamFigureFactory::create();
	}
};

INHERIT(IstreamFigureFactory, FileFigureFactory);
class FileFigureFactory : public IstreamFigureFactory {
	std::ifstream stream;

   public:
	FileFigureFactory(const std::string &fileName) : IstreamFigureFactory(stream) {
		stream.open(fileName);
		if (!stream) { throw std::runtime_error("cannot open file " + fileName + " : " + strerror(errno)); }
	}

	using IstreamFigureFactory::create;
};

JOB(register_figure_factories, {
	TypeRegistry::registerConstructor("RandomFigureFactory", std::function([](std::string cnt) -> FigureFactory * {
										  return new RandomFigureFactory(std::stoi(cnt));
									  }), "Random N  : generate N random figures");
	TypeRegistry::registerConstructor("STDINFigureFactory", std::function([](std::string cnt) -> FigureFactory * {
										  return new STDINFigureFactory(std::stoi(cnt));
									  }), "STDIN N  : input N figures from stdin");
	TypeRegistry::registerConstructor("FileFigureFactory", std::function([](std::string &fileName) -> FigureFactory * {
										  return new FileFigureFactory(fileName);
									  }), "File fileName : read all figures from the specified file");
});
