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

TEST_CASE("recursive iteration ls check") {
	std::ostringstream oss;

	SUBCASE("ls no links") {
		auto res = FSTreeBuilderNoLinks().build(PROJECT_SOURCE_DIR);
		CHECK(res);
		res->accept(LsWriter(oss));

		std::string	 cmd = "ls -RU " + std::filesystem::relative(PROJECT_SOURCE_DIR).string();
		ShellProcess p(cmd.c_str());
		CHECK_EQ(p.wait(), 0);

		CHECK_EQ(getString(p.err()), "");
		CHECK_EQ(getString(p.out()) + '\n', oss.str());
	}

	SUBCASE("ls no links") {
		auto res = FSTreeBuilderWithLinks().build(PROJECT_SOURCE_DIR);
		CHECK(res);
		res->accept(LsWriter(oss));

		std::string	 cmd = "ls -RUL " + std::filesystem::relative(PROJECT_SOURCE_DIR).string();
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
	GNUHashStreamWriter	  writer(calc, oss);

	auto res = FSTreeBuilderNoLinks().build(PROJECT_SOURCE_DIR "/test/asd");
	CHECK(res);
	res->accept(writer);

	std::string	 cmd  = "md5sum " + std::filesystem::relative(PROJECT_SOURCE_DIR "/test/asd/").string() + "/* -b";
	ShellProcess p(cmd.c_str());
	CHECK_EQ(p.wait(), 0);
	CHECK_EQ(getString(p.err()), "");
	CHECK_EQ(getString(p.out()), oss.str());
};
