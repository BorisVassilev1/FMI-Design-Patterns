#pragma once

#include <functional>
#include <memory>
#include <string>
#include <unordered_map>

template <class T, class ...Args>
class Factory {
	std::unordered_map<std::string, std::function<std::unique_ptr<T>(Args&&...)>> m_factories;

   public:
	template <class U>
	void registerType(const std::string &name) {
		m_factories[name] = [](Args&&... args) -> std::unique_ptr<T> { return std::make_unique<U>(std::forward<Args>(args)...); };
	}

	std::unique_ptr<T> create(const std::string &name, Args &&...args) { return m_factories[name](std::forward<Args>(args)...); }

	bool exists(const std::string &name) { return m_factories.find(name) != m_factories.end(); }

	static Factory &instance() {
		static Factory instance;
		return instance;
	}

	std::vector<std::string> getKeys() {
		std::vector<std::string> keys;
		for (const auto &pair : m_factories) {
			keys.push_back(pair.first);
		}
		return keys;
	}
};
