#pragma once

#include <format>

#include <filesystem>
#include <observe.hpp>
#include <FSTree.hpp>
#include <visitors.hpp>
#include <chrono>

class ProgressViewer : public Observer<std::filesystem::path>, public Observer<std::uintmax_t> {
	std::filesystem::path							   current_path;
	std::uintmax_t									   current_bytes;
	std::uintmax_t									   current_file_bytes;
	std::uintmax_t									   total_bytes;
	std::chrono::time_point<std::chrono::steady_clock> start_time;
	std::ostream									  &os;

   public:
	ProgressViewer(FSNode *tree, HashStreamWriter *writer, std::ostream &os)
		: current_bytes(0),
		  current_file_bytes(0),
		  total_bytes(tree->size),
		  start_time(std::chrono::steady_clock::now()),
		  os(os) {
		writer->BasicObservable<std::filesystem::path>::addObserver(this);
		writer->ForwardObservable<std::uintmax_t>::addObserver(this);
	}

	void update(const std::filesystem::path &path) override {
		current_bytes += current_file_bytes;
		current_file_bytes = 0;
		current_path	   = path;
		redraw();
	}

	void update(const std::uintmax_t &bytes) override {
		current_file_bytes = bytes;
		redraw();
	}

	void redraw() {
		auto now	 = std::chrono::steady_clock::now();
		auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - start_time).count();

		auto curr_bytes = current_bytes + current_file_bytes;
		auto eta		= std::chrono::milliseconds((total_bytes - curr_bytes) * elapsed / (curr_bytes + 1));

		std::cout << std::format("\rProcessing file {:30} | Total {}/{} byte(s) ({}%) | Est. {:%T}                   ",
								 current_path.string(), curr_bytes, total_bytes, current_bytes * 100 / total_bytes, eta)
				  << std::flush;
	}
};

inline void executeHashStreamWriter(FSNode &tree, HashStreamWriter &writer, std::ostream &os) {
	ProgressViewer progress(&tree, &writer, os);
	tree.accept(writer);
}
