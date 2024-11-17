#pragma once

#include <any>
#include <functional>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <unordered_set>

#include "utils.hpp"

template <typename Ret, typename... Args>
std::function<void *(std::vector<std::any>)> argsToVector(const std::function<Ret(Args...)> &func) {
	std::function<void *(std::vector<std::any>)> result = [=](std::vector<std::any> args) -> void * {
		std::size_t argcnt = sizeof...(Args);
		if (args.size() != argcnt) {
			throw std::runtime_error("function called with wrong number of arguments: expected " +
									 std::to_string(argcnt) + ", but got " + std::to_string(args.size()));
		}
		// clang-format off
		return [&]<std::size_t... p>(std::index_sequence<p...>) {
			return func(
				std::any_cast<std::tuple_element_t<p, std::tuple<Args...>>>(args[p])
				...
			);
		}(std::make_index_sequence<sizeof...(Args)>{});
		// clang-format on
	};
	return result;
}

class TypeRegistry {
   public:
	using Constructor = std::function<void *(std::vector<std::any>)>;
	struct ConstructorData {
		std::size_t argcnt;
		Constructor c;
	};

	static auto &getConstructors() {
		static std::unordered_map<std::string, ConstructorData> constructors;
		return constructors;
	}
	static auto &getChildren() {
		static std::unordered_map<std::string, std::vector<std::string>> iheritance_tree;
		return iheritance_tree;
	}
	static auto &getParents() {
		static std::unordered_map<std::string, std::vector<std::string>> parents;
		return parents;
	}

	template <typename Ret, typename... Args>
	static void registerConstructor(const std::string &typeName, const std::function<Ret(Args...)> &func) {
		getConstructors().insert({typeName, {sizeof...(Args), argsToVector(func)}});
	}

	static void registerSuccessor(const std::string &parent, const std::string &child) {
		getChildren()[parent].push_back(child);
		getParents()[child].push_back(parent);

		for (const auto &p : getParents()[parent]) {
			getParents()[child].push_back(p);
		}
		for (const auto &c : getChildren()[child]) {
			getChildren()[parent].push_back(c);
		}
	}
	
};

#define INHERIT(parent, child) JOB(inherit_##parent##child, TypeRegistry::registerSuccessor(#parent, #child););

template <class Type, class... Args>
static std::function<Type *(Args...)> constructor_wrapper = [](Args... args) { return new Type(args...); };

#define REGISTER_CONSTRUCTOR(Type) \
	JOB(register_constructor_##Type,    \
		{ TypeRegistry::registerConstructor(#Type, constructor_wrapper<Type>); });
#define REGISTER_CONSTRUCTOR_WITH_ARGS(Type, ...) \
	JOB(register_constructor_##Type,    \
		{ TypeRegistry::registerConstructor(#Type, constructor_wrapper<Type, __VA_ARGS__>); });
