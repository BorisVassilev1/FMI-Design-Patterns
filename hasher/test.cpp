#include <cassert>
#include <sstream>
#include <FSTree.hpp>
#include <calculators.hpp>
#include <visitors.hpp>
#include "pipes.hpp"

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>

TEST_CASE("MD5") {
	SUBCASE("asd") {
		std::istringstream	  ss("abc");
		MD5ChecksumCalculator calc;
		std::string			  result;
		result = calc.calculate(ss);
		CHECK_EQ(result, "900150983cd24fb0d6963f7d28e17f72");
	}

	SUBCASE("Hello World!") {
		std::istringstream	  ss("Hello world!");
		MD5ChecksumCalculator calc;
		std::string			  result;
		result = calc.calculate(ss);
		CHECK_EQ(result, "86fb269d190d2c85f6e0468ceca42a20");
	}
}

TEST_CASE("SHA256") {
	SUBCASE("asd") {
		std::istringstream		 ss("abc");
		SHA256ChecksumCalculator calc;
		std::string				 result;
		result = calc.calculate(ss);
		CHECK_EQ(result, "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad");
	}

	SUBCASE("Hello World!") {
		std::istringstream		 ss("Hello world!");
		SHA256ChecksumCalculator calc;
		std::string				 result;
		result = calc.calculate(ss);
		CHECK_EQ(result, "c0535e4be2b79ffd93291305436bf889314e4a3faec05ecffcbb7df31ad9e51a");
	}
}

TEST_CASE("Calculator Factory") {
	std::istringstream		   ss("abc");
	ChecksumCalculatorFactory &factory = ChecksumCalculatorFactory::instance();

	CHECK_EQ(factory.exists("md5"), true);
	CHECK_EQ(factory.exists("sha256"), true);

	SUBCASE("md5") {
		auto				calc = factory.create("md5");
		ChecksumCalculator &c	 = *calc;
		CHECK_EQ(typeid(c), typeid(MD5ChecksumCalculator));
	}

	SUBCASE("sha256") {
		auto				calc = factory.create("sha256");
		ChecksumCalculator &c	 = *calc;
		CHECK_EQ(typeid(c), typeid(SHA256ChecksumCalculator));
	}
}

TEST_CASE("fs") {
	std::ostringstream oss;

	SUBCASE("No links") {
		auto res = FSTreeBuilderNoLinks().build(PROJECT_SOURCE_DIR "/test");
		CHECK(res);
		res->accept(FSTreePrinter(oss));

		CHECK_EQ(oss.str(),
				 "─<test : 74\n"
				 "  ├─<asd : 62\n"
				 "  │  ├─ 1 : 40\n"
				 "  │  ├─ 2 : 9\n"
				 "  │  └─ 3 : 13\n"
				 "  ├─<bbb : 0\n"
				 "  │  └─ bb : 0\n"
				 "  ├─ b : 0\n"
				 "  ├─ a : 7\n"
				 "  └─ c : 5\n");
	}

	SUBCASE("with links") {
		auto res = FSTreeBuilderWithLinks().build(PROJECT_SOURCE_DIR "/test");
		CHECK(res);
		res->accept(FSTreePrinter(oss));
		CHECK_EQ(oss.str(),
				 "─<test : 137\n"
				 "  ├─<asd : 62\n"
				 "  │  ├─ 1 : 40\n"
				 "  │  ├─ 2 : 9\n"
				 "  │  └─ 3 : 13\n"
				 "  ├─<bbb : 0\n"
				 "  │  └─ bb : 0\n"
				 "  ├─ b : 0\n"
				 "  ├─ a : 13\n"
				 "  └─<c : 62\n"
				 "     ├─ 1 : 40\n"
				 "     ├─ 2 : 9\n"
				 "     └─ 3 : 13\n");
	}
}

TEST_CASE("recursive iteration") {
	std::ostringstream oss;

	SUBCASE("ls no links") {
		auto res = FSTreeBuilderNoLinks().build(PROJECT_SOURCE_DIR "/test");
		CHECK(res);
		res->accept(LsWriter(oss));

		std::string	 cmd = "ls -RU " + std::filesystem::relative(PROJECT_SOURCE_DIR "/test/").string();
		ShellProcess p(cmd.c_str());
		CHECK_EQ(p.wait(), 0);

		CHECK_EQ(getString(p.err()), "");
		CHECK_EQ(getString(p.out()) + '\n', oss.str());
	}

	SUBCASE("ls no links") {
		auto res = FSTreeBuilderWithLinks().build(PROJECT_SOURCE_DIR "/test");
		CHECK(res);
		res->accept(LsWriter(oss));

		std::string	 cmd = "ls -RUL " + std::filesystem::relative(PROJECT_SOURCE_DIR "/test/").string();
		ShellProcess p(cmd.c_str());
		CHECK_EQ(p.wait(), 0);

		CHECK_EQ(getString(p.err()), "");
		CHECK_EQ(getString(p.out()) + '\n', oss.str());
	}
};

TEST_CASE("md5 checksum directory") {
	FSTreeBuilderNoLinks  builder;
	MD5ChecksumCalculator calc;
	std::ostringstream	  oss;
	GNUHashStreamWriter	  printer(calc, oss);
	ReportWriter		  report(std::cout);

	auto res = FSTreeBuilderNoLinks().build(PROJECT_SOURCE_DIR "/test/asd");
	CHECK(res);
	res->accept(printer);

	std::string	 path = std::filesystem::relative(PROJECT_SOURCE_DIR "/test/asd/");
	std::string	 cmd  = "md5sum " + path + "/* -b";
	ShellProcess p(cmd.c_str());
	CHECK_EQ(p.wait(), 0);
	CHECK_EQ(getString(p.err()), "");
	CHECK_EQ(getString(p.out()), oss.str());
};
