#include <cassert>
#include <sstream>
#include "src/calculators.hpp"

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

	std::istringstream ss("abc");
	ChecksumCalculatorFactory &factory = ChecksumCalculatorFactory::instance();
	CHECK_EQ(typeid(*factory.create("md5").get()), typeid(MD5ChecksumCalculator));
	CHECK_EQ(typeid(*factory.create("sha256").get()), typeid(SHA256ChecksumCalculator));
	CHECK_EQ(factory.exists("md5"), true);
	CHECK_EQ(factory.exists("sha256"), true);
}
