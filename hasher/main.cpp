#include <cassert>
#include <iostream>

#include <tclap/SwitchArg.h>
#include <tclap/ValueArg.h>
#include <tclap/ValuesConstraint.h>
#include <tclap/CmdLine.h>

#include <FSTree.hpp>
#include <thread>
#include <utils.hpp>
#include <visitors.hpp>
#include "calculators.hpp"
#include "factory.hpp"

int main(int argc, char **argv) {
	try {
		std::vector<std::string>			 algorithms = ChecksumCalculatorFactory::instance().getKeys();
		std::vector<std::string>			 formats	= HashStreamWriterFactory::instance().getKeys();
		TCLAP::ValuesConstraint<std::string> allowedAlgs(algorithms);
		TCLAP::ValuesConstraint<std::string> allowedFormats(formats);

		TCLAP::CmdLine cmd("Checksum Calculator", ' ', "0.0.1");

		TCLAP::ValueArg<std::string> pathArg("p", "path", "path to calculate checksums for", false, ".", "string", cmd);
		TCLAP::ValueArg<std::string> algorithmArg("a", "algorithm", "which hashing algorithm to use", false, "md5",
												  &allowedAlgs, cmd);
		TCLAP::SwitchArg			 linksArg("l", "link", "if specified, follow symbolic links", cmd);
		TCLAP::ValueArg<std::string> formatArg("f", "format", "output format", false, "gnu", &allowedFormats, cmd);
		TCLAP::SwitchArg			 checksums("c", "checksums", "verify checksums in directory", cmd, false);

		cmd.parse(argc, argv);

		bool		mode		= checksums.getValue();
		bool		followLinks = linksArg.getValue();
		std::string path		= pathArg.getValue();
		std::string algorithm	= algorithmArg.getValue();
		std::string format		= formatArg.getValue();

		std::cout << "Calculating checksums for " << path << " using " << algorithm << " algorithm" << std::endl;
		std::cout << "Following symbolic links: " << (followLinks ? "yes" : "no") << std::endl;

		if (!mode) {
			// create scanner
			std::unique_ptr<FSTreeBuilder> builder = nullptr;
			if (followLinks) builder = std::make_unique<FSTreeBuilderWithLinks>();
			else builder = std::make_unique<FSTreeBuilderNoLinks>();

			// scan directory
			auto tree = builder->build(path);
			if(!tree) throw std::runtime_error("failed to build tree");

			// calculate checksums
			auto calculator = ChecksumCalculatorFactory::instance().create(algorithm);
			auto writer		= HashStreamWriterFactory::instance().create(format, *calculator, std::cout);

			auto thread = std::thread([&] { tree->accept(*writer); });
			//... can cancel
			thread.join();
		}

	} catch (TCLAP::ArgException &e)	 // catch exceptions
	{
		std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl;
	} catch (std::exception &e) { std::cerr << "error: " << e.what() << std::endl; }
}
