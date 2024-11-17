#pragma once

#include <cstring>
#include <iostream>
#include <fstream>
#include <sstream>

#include "figures.hpp"
#include "factory.hpp"
#include "type_registry.hpp"

using FigureFactory = Factory<Figure>;

INHERIT(FigureFactory, RandomFigureFactory);
class RandomFigureFactory : public FigureFactory {
   public:
	Figure *create() override {
		const auto &children = TypeRegistry::getChildren().at("Figure");
		int			index	 = rand() % children.size();

		const std::string &figureName	 = children[index];
		const auto [argcnt, constructor] = TypeRegistry::getConstructors().at(figureName);

		std::vector<std::any> args;
		for (std::size_t i = 0; i < argcnt; ++i) {
			args.push_back(float(rand() % 50 / 10. + 5));
		}

		auto res = constructor(args);
		return static_cast<Figure *>(res);
	}
};

INHERIT(FigureFactory, IstreamFigureFactory);
class IstreamFigureFactory : public FigureFactory {
	std::istream &in;

   public:
	IstreamFigureFactory(std::istream &in) : in(in) {}

	Figure *create() override {
		if (!in || in.eof()) { throw std::runtime_error("not enough arguments"); }
		std::string figureType;
		in >> figureType;

		std::size_t						argcnt = 0;
		TypeRegistry::Constructor		constructor;
		const std::vector<std::string> *parents;

		try {
			parents			= &TypeRegistry::getParents().at(figureType);
			const auto data = TypeRegistry::getConstructors().at(figureType);
			argcnt			= data.argcnt;
			constructor		= data.c;

		} catch (const std::out_of_range &e) {
			throw std::runtime_error(std::format("figure type \"{}\" is not registered correctly", figureType));
		}

		if (std::find(parents->begin(), parents->end(), "Figure") == parents->end()) {
			throw std::runtime_error(std::format("figure type \"{}\" is not a Figure", figureType));
		}

		std::vector<std::any> args;
		for (std::size_t i = 0; i < argcnt; ++i) {
			float arg = 0;
			in >> arg;
			if (!in) { throw std::runtime_error(std::format("not enough arguments for \"{}\"", figureType)); }
			args.push_back(arg);
			if (in.eof()) break;
		}

		return static_cast<Figure *>(constructor(args));
	}
};

inline Figure *figureFromString(const std::string &s) {
	std::stringstream	 ss(s);
	IstreamFigureFactory ff(ss);
	return ff.create();
}

INHERIT(IstreamFigureFactory, STDINFigureFactory);
class STDINFigureFactory : public IstreamFigureFactory {
   public:
	STDINFigureFactory() : IstreamFigureFactory(std::cin) {}
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
	TypeRegistry::registerConstructor("RandomFigureFactory",
									  std::function([]() -> FigureFactory * { return new RandomFigureFactory(); }));
	TypeRegistry::registerConstructor("STDINFigureFactory",
									  std::function([]() -> FigureFactory * { return new STDINFigureFactory(); }));
	TypeRegistry::registerConstructor("FileFigureFactory",
									  std::function([](std::istream *in, std::ostream *out) -> FigureFactory * {
										  std::string fileName;
										  *out << "Please enter the file name: " << std::flush;
										  *in >> fileName;
										  if (!*in) throw std::runtime_error("could read file name");
										  return new FileFigureFactory(fileName);
									  }));
});
