#pragma once

#include <format>
#include <istream>
#include <ranges>
#include <sstream>

#include "type_registry.hpp"
#include "utils.hpp"

template <class T>
class Factory {
   public:
	virtual std::unique_ptr<T> create() = 0;
	virtual ~Factory() {};
};

template <class T, string_literal T_Name, bool append_name, class Param>
class IstreamFactory : public Factory<T> {
   protected:
	std::istream				&in;
	static constexpr const char *type_name = T_Name;

   public:
	IstreamFactory(std::istream &in) : in(in) {}

	static auto listFactoryTypes() {
		auto &v = TypeRegistry::getChildren().at(type_name);
		auto  k = v | std::views::filter([](const std::string &s) {
					  return TypeRegistry::getConstructors().find(s) != TypeRegistry::getConstructors().end();
				  }) |
				 std::views::transform([](const std::string &s) {
				auto &data = TypeRegistry::getConstructors().at(s);
					 return std::make_tuple(s.substr(0, s.size() - 13), data.argcnt, data.description);
				 });
		return k;
	}

	std::unique_ptr<T> create() override {
		while(std::isspace(in.peek())) in.ignore();
		if (!in || in.eof()) return nullptr;
		std::string figureType;
		in >> figureType;

		std::string input = figureType;
		if (append_name) { figureType += type_name; }

		std::size_t						argcnt = 0;
		TypeRegistry::Constructor		constructor;
		const std::vector<std::string> *parents;

		try {
			parents			= &TypeRegistry::getParents().at(figureType);
			const auto data = TypeRegistry::getConstructors().at(figureType);
			argcnt			= data.argcnt;
			constructor		= data.c;
		} catch (std::exception &e) {
			throw std::runtime_error(std::format("{} not recognized: \"{}\"", type_name, input));
		}

		if (std::find(parents->begin(), parents->end(), std::string(type_name)) == parents->end()) {
			throw std::runtime_error(std::format("figure type \"{}\" is not a {}", input, type_name));
		}

		std::vector<std::any> args;

		for (std::size_t i = 0; i < argcnt; ++i) {
			Param arg;
			in >> arg;
			if (!in) { throw std::runtime_error(std::format("not enough arguments for \"{}\"", figureType)); }
			args.push_back(arg);
			if (in.eof()) break;
		}

		std::unique_ptr<T> ptr;
		ptr.reset(static_cast<T *>(constructor(args)));
		return ptr;
	}
};

template <class T, string_literal T_Name, bool append_name, class Param>
class LineIstreamFactory : public IstreamFactory<T, T_Name, append_name, Param> {
   public:
	using IstreamFactory<T, T_Name, append_name, Param>::IstreamFactory;

	std::unique_ptr<T> create() override {
		if (!this->in || this->in.eof()) return nullptr;
		std::string strArgs;
		std::getline(this->in, strArgs);
		if (strArgs.empty()) return nullptr;
		std::istringstream ss(strArgs);

		std::string figureType;
		ss >> figureType;
		std::string input = figureType;
		if (append_name) { figureType += this->type_name; }

		std::size_t						argcnt = 0;
		TypeRegistry::Constructor		constructor;
		const std::vector<std::string> *parents;

		try {
			parents			= &TypeRegistry::getParents().at(figureType);
			const auto data = TypeRegistry::getConstructors().at(figureType);
			argcnt			= data.argcnt;
			constructor		= data.c;
		} catch (std::exception &e) {
			throw std::runtime_error(std::format("{} not recognized: \"{}\"", this->type_name, input));
		}

		if (std::find(parents->begin(), parents->end(), std::string(this->type_name)) == parents->end()) {
			throw std::runtime_error(std::format("figure type \"{}\" is not a {}", input, this->type_name));
		}

		std::vector<std::any> args;
		while (true) {
			Param p;
			ss >> p;
			if (!ss) break;
			args.push_back(p);
		}
		
		std::unique_ptr<T> ptr;
		ptr.reset(static_cast<T *>(constructor(args)));
		return ptr;
	}
};
