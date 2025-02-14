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
#include <reportData.hpp>
#include "progress.hpp"

int main(int argc, char **argv) {
	using namespace std::literals;
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
		TCLAP::ValueArg<std::string> checksums("c", "checksums", "verify checksums in directory", false, "", "string",
											   cmd);
		TCLAP::ValueArg<std::string> output("o", "output", "output file", false, "", "string", cmd);

		cmd.parse(argc, argv);

		std::string outputPath	  = output.getValue();
		std::string checksumsPath = checksums.getValue();
		bool		mode		  = !checksumsPath.empty();
		bool		followLinks	  = linksArg.getValue();
		std::string path		  = pathArg.getValue();
		std::string algorithm	  = algorithmArg.getValue();
		std::string format		  = formatArg.getValue();

		// std::cout << "Calculating checksums for " << path << " using " << algorithm << " algorithm" << std::endl;
		// std::cout << "Following symbolic links: " << (followLinks ? "yes" : "no") << std::endl;

		// create scanner
		std::unique_ptr<FSTreeBuilder> builder = nullptr;
		if (followLinks) builder = std::make_unique<FSTreeBuilderWithLinks>();
		else builder = std::make_unique<FSTreeBuilderNoLinks>();

		// scan directory
		auto tree = builder->build(path);
		if (!tree) throw std::runtime_error("failed to build tree");

		auto calculator = ChecksumCalculatorFactory::instance().create(algorithm);

		if (!mode) {
			// calculate checksums
			std::unique_ptr<ProgressViewer> progress = nullptr;
			std::ostream				   *os		 = &std::cout;
			std::ofstream					ofs;
			if (!outputPath.empty()) {
				ofs.open(outputPath);
				if (!ofs) throw std::runtime_error("failed to open file: "s + strerror(errno));
				os = &ofs;
			}
			auto writer = HashStreamWriterFactory::instance().create(format, *calculator, *os);

			if (outputPath != "") { progress = std::make_unique<ProgressViewer>(tree.get(), writer.get(), std::cout); }

			auto thread = std::thread([&] { tree->accept(*writer); });
			//... can cancel
			thread.join();
		} else {
			// verify checksums

			// read old checksums
			std::ifstream is(checksumsPath);
			if (!is) throw std::runtime_error("failed to open file: "s + strerror(errno));
			auto	   reportBuilder = ReportDataBuilderFactory::instance().create(format);
			ReportData oldTreeData	 = reportBuilder->build(is);

			// calculate new checksums
			ReportData newTreeData;
			auto	   treeWriter = ReportDataHashStreamWriter(*calculator, std::cerr, newTreeData);
			auto progress = ProgressViewer(tree.get(), &treeWriter, std::cout);
			auto	   thread	  = std::thread([&] { tree->accept(treeWriter); });
			// ... can cancel
			thread.join();

			// compare
			compare(oldTreeData, newTreeData, std::cout);
		}

	} catch (TCLAP::ArgException &e)	 // catch exceptions
	{
		std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl;
		exit(1);
	} catch (std::exception &e) { std::cerr << "error: " << e.what() << std::endl; exit(1); }
}
