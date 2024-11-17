#pragma once

#include <istream>
#include <fstream>
#include "figures.hpp"

namespace commands {

static auto list = std::function([](std::vector<Figure *> &data, std::istream &in, std::ostream &out) -> bool {
	for (const auto &figure : data) {
		out << (std::string)*figure << std::endl;
	}
	return true;
});

static auto remove = std::function([](std::vector<Figure *> &data, std::istream &in, std::ostream &out) -> bool {
	std::size_t index;
	in >> index;
	if (index >= data.size()) {
		throw std::out_of_range("Index out of range");
	}
	delete data[index];
	data.erase(data.begin() + index);
	return true;
});

static auto duplicate = std::function([](std::vector<Figure *> &data, std::istream &in, std::ostream &out) -> bool {
	std::size_t index;
	in >> index;
	if (index >= data.size()) {
		throw std::out_of_range("Index out of range");
	}
	data.push_back(data[index]->clone());
	return true;
});

static auto store = std::function([](std::vector<Figure *> &data, std::istream &in, std::ostream &out) -> bool {
	out << "Enter filename: " << std::flush;
	std::string filename;
	in >> filename;
	std::ofstream file((filename));
	if (!file.is_open()) {
		throw std::runtime_error("Cannot open file");
	}
	for(auto *figure : data) {
		file << (std::string)*figure << std::endl;
		if(!file) {
			throw std::runtime_error("Cannot write to file");
		}
	}
	file.close();
	return true;
});


}	  // namespace commands
