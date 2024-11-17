#pragma once

#include <cctype>
#include <any>
#include <ranges>
#include <stdexcept>
#include <vector>

#include "figure_factory.hpp"
#include "type_registry.hpp"

class AbstractFigureFactory {
   public:
	auto listFactoryTypes() const {
		auto v = TypeRegistry::getChildren().at("FigureFactory");
		v.erase(std::remove_if(v.begin(), v.end(), [](const std::string &s) {
			return TypeRegistry::getConstructors().find(s) == TypeRegistry::getConstructors().end();
		}), v.end());
		return v;
	}

	FigureFactory *create(std::istream &in, std::ostream &out) {
		std::string name;
		in >> name;
		if (!in) throw std::runtime_error("not enough arguments");
		name += "FigureFactory";

		std::size_t						argcnt = 0;
		TypeRegistry::Constructor	   *constructor;
		const std::vector<std::string> *parents;

		try {
			parents		= &TypeRegistry::getParents().at(name);
			auto &data	= TypeRegistry::getConstructors().at(name);
			argcnt		= data.argcnt;
			constructor = &data.c;
		} catch (...) { throw std::runtime_error(std::format("type not recognized: \"{}\"", name)); }

		if (std::find(parents->begin(), parents->end(), "FigureFactory") == parents->end()) {
			throw std::runtime_error(std::format("\"{}\" is not a FigureFactory", name));
		}

		std::vector<std::any> args;
		if (argcnt == 2) {
			args.push_back(std::any(&in));
			args.push_back(std::any(&out));
		}

		return static_cast<FigureFactory *>((*constructor)(args));
	}
};
