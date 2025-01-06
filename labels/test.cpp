#include <cassert>
#include <sstream>
#include "src/concat.hpp"
#include "src/flaggedptr.hpp"
#include "src/label.hpp"
#include "src/labelTransformations.hpp"

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>

TEST_CASE("FlaggedPtr") {
	SUBCASE("int") {
		FlaggedPtr a = new int32_t(10);
		assert(a.space == 2);
		a.setFlags(0b11);
		CHECK_EQ(*a, 10);
		a.setFlags(0b1111);
		CHECK_EQ(*a, 10);
		*a = 20;
		CHECK_EQ(*a, 20);
	}
	SUBCASE("int64_t") {
		FlaggedPtr a = new uint64_t(10);
		assert(a.space == 3);
		a.setFlags(0b11);
		CHECK_EQ(*a, 10);
		a.setFlags(0b1111);
		CHECK_EQ(*a, 10);
		*a = 20;
		CHECK_EQ(*a, 20);
	}
}

TEST_CASE("Concat") {
	SUBCASE("two containers") {
		std::vector<int> v1 = {1, 2, 3};
		std::vector<int> v2 = {4, 5, 6};
		auto			 r	= Concat(v1, v2);

		auto i = r.begin();
		CHECK_EQ(*i, 1);
		++i;
		CHECK_EQ(*i, 2);
		++i;
		CHECK_EQ(*i, 3);
		++i;
		CHECK_EQ(*i, 4);
		++i;
		CHECK_EQ(*i, 5);
		++i;
		CHECK_EQ(*i, 6);
		++i;
		CHECK_EQ(i, r.end());
	}

	SUBCASE("more strings") {
		std::string s1 = "HELLO ";
		std::string s2 = "World ";
		std::string s3 = "!!!";

		auto		r = Concat(s1, Concat(s2, s3));
		std::string s;
		for (auto c : r) {
			s.push_back(c);
		}
		CHECK_EQ(s, "HELLO World !!!");
	}
}

TEST_CASE("Label Creation") {
	SimpleLabel		   label("HELLO");
	std::ostringstream ss;
	LabelPrinter::print(label, ss);
	CHECK_EQ(ss.str(), "Here is a label: HELLO");
}

TEST_CASE("Transformations") {
	SUBCASE("Capitalize") {
		CHECK_EQ(CapitalizeTransformation().apply("some text"), "Some text");
		CHECK_EQ(CapitalizeTransformation().apply(" some text"), " some text");
	}
	SUBCASE("LeftTrim") {
		CHECK_EQ(LeftTrimTransformation().apply("   some text"), "some text");
		CHECK_EQ(LeftTrimTransformation().apply("some text"), "some text");
	}
	SUBCASE("RightTrim") {
		CHECK_EQ(RightTrimTransformation().apply("some text   "), "some text");
		CHECK_EQ(RightTrimTransformation().apply("some text"), "some text");
	}
	SUBCASE("NormalizeSpace") {
		CHECK_EQ(NormalizeSpaceTransformation().apply(" some text "), " some text ");
		CHECK_EQ(NormalizeSpaceTransformation().apply("  some   text   "), " some text ");
	}
	SUBCASE("Censor") {
		CHECK_EQ(CensorTransformation("abc").apply(" abc def abcdef"), " *** def ***def");
		CHECK_EQ(CensorTransformation("abc").apply(" abc def abcdef"), " *** def ***def");
		CHECK_EQ(CensorTransformation("aba").apply(" aba def ababa"), " *** def *****");
	}
	SUBCASE("Replace") {
		CHECK_EQ(ReplaceTransformation("abc", "d").apply(" abc abcdef"), " d ddef");
		CHECK_EQ(ReplaceTransformation("abc", "abd").apply(" abd abddef"), " abd abddef");
		CHECK_EQ(ReplaceTransformation("abc", "abd").apply(" abc abcdef"), " abd abddef");
		CHECK_EQ(ReplaceTransformation("abc", "abde").apply(" abc abcdef"), " abde abdedef");
	}
}
