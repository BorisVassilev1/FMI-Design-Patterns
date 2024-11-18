#pragma once

#include <any>
#include <functional>
#include <iostream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <map>
#include <utility>
#include <unordered_set>
#include <algorithm>

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
		std::string description;
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
	static void registerConstructor(const std::string &typeName, const std::function<Ret(Args...)> &func,
									const std::string &description = "") {
		getConstructors().insert({typeName, {sizeof...(Args), description, argsToVector(func)}});
	}

	static void registerSuccessor(const std::string &parent, const std::string &child) {
		auto &childrenOfParent = getChildren()[parent];
		auto &parentsOfChild   = getParents()[child];
		auto &parentsOfParent  = getParents()[parent];
		auto &childrenOfChild  = getChildren()[child];

		childrenOfParent.push_back(child);
		parentsOfChild.push_back(parent);

		parentsOfChild.insert(parentsOfChild.end(), parentsOfParent.begin(), parentsOfParent.end());
		childrenOfParent.insert(childrenOfParent.end(), childrenOfChild.begin(), childrenOfChild.end());

		std::function<void(const std::string &)> dfsDown = [&](const std::string &child) {
			for (const auto &c : getChildren()[child]) {
				auto &parentsOfChild = getParents()[c];
				if (std::find(parentsOfChild.begin(), parentsOfChild.end(), parent) == parentsOfChild.end()) {
					parentsOfChild.push_back(parent);
				}
				dfsDown(c);
			}
		};

		std::function<void(const std::string &)> dfsUp = [&](const std::string &pparent) {
			for (const auto &p : getParents()[pparent]) {
				auto &childrenOfParent = getChildren()[p];
				if (std::find(childrenOfParent.begin(), childrenOfParent.end(), child) == childrenOfParent.end()) {
					childrenOfParent.push_back(child);
				}
				dfsUp(p);
			}
		};
		dfsDown(child);
		dfsUp(parent);
	}

	static std::unordered_set<std::string> getDescendants(const std::string &type) {
		std::unordered_set<std::string>			 result;
		std::function<void(const std::string &)> dfs = [&](const std::string &type) {
			result.insert(type);
			for (const auto &child : getChildren().at(type)) {
				dfs(child);
			}
		};
		dfs(type);
		return result;
	}
};

#define INHERIT(parent, child) JOB(inherit_##parent##child, TypeRegistry::registerSuccessor(#parent, #child););

template <class Type, class... Args>
static std::function<Type *(Args...)> constructor_wrapper = [](Args... args) { return new Type(args...); };

#define REGISTER_CONSTRUCTOR(Type) \
	JOB(register_constructor_##Type, { TypeRegistry::registerConstructor(#Type, constructor_wrapper<Type>); });
#define REGISTER_CONSTRUCTOR_WITH_ARGS(Type, ...) \
	JOB(register_constructor_##Type,              \
		{ TypeRegistry::registerConstructor(#Type, constructor_wrapper<Type, __VA_ARGS__>); });
