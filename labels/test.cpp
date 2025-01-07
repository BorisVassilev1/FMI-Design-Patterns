#include <cassert>
#include <sstream>
#include "src/concat.hpp"
#include "src/flaggedptr.hpp"
#include "src/label.hpp"
#include "src/labelDecorators.hpp"
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

		delete (int32_t *)a;
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

		delete (uint64_t *)a;
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
	SUBCASE("Decorate") { CHECK_EQ(DecorateTransformation().apply("asd"), "-={ asd }=-"); }
}

TEST_CASE("Decorators") {
	SUBCASE("LabelTransformDecorator") {
		auto l	= SimpleLabel("asd");
		auto dt = DecorateTransformation();
		auto ct = CapitalizeTransformation();
		auto et = CensorTransformation("asd");

		auto d = TransformDecorator(l, dt);
		CHECK_EQ(d.getText(), "-={ asd }=-");

		auto c = TransformDecorator(l, ct);
		CHECK_EQ(c.getText(), "Asd");

		auto e = TransformDecorator(d, et);
		CHECK_EQ(e.getText(), "-={ *** }=-");
	};
	SUBCASE("LabelRandomTransformationDecorator") {
		auto l	= SimpleLabel("asd");
		auto dt = DecorateTransformation();
		auto ct = CapitalizeTransformation();
		auto et = CensorTransformation("asd");

		auto d = RandomTransformationDecorator(l, {dt, ct});
		for (std::size_t i = 0; i < 10; ++i) {
			std::string text = d.getText();
			CHECK((text == "-={ asd }=-" || text == "Asd"));
		}

		auto e = TransformDecorator(d, et);
		for (std::size_t i = 0; i < 10; ++i) {
			std::string text = e.getText();
			CHECK((text == "-={ *** }=-" || text == "Asd"));
		}
	}
	SUBCASE("LabelCyclingTransformationsDecorator") {
		auto l	= SimpleLabel("asd");
		auto dt = DecorateTransformation();
		auto ct = CapitalizeTransformation();
		auto et = CensorTransformation("sd");
		auto d	= CyclingTransformationsDecorator(l, {dt, ct});
		CHECK_EQ(d.getText(), "-={ asd }=-");
		CHECK_EQ(d.getText(), "Asd");
		CHECK_EQ(d.getText(), "-={ asd }=-");
		CHECK_EQ(d.getText(), "Asd");
		auto e = TransformDecorator(d, et);
		CHECK_EQ(e.getText(), "-={ a** }=-");
		CHECK_EQ(e.getText(), "A**");
	}
}

TEST_CASE("RemoveDecorator") {
	SUBCASE("static") {
		SimpleLabel			   l  = SimpleLabel("asd");
		DecorateTransformation dt = DecorateTransformation();
		DecorateTransformation ct = DecorateTransformation();
		CensorTransformation   et = CensorTransformation("asd");

		auto d = TransformDecorator(l, dt);
		CHECK_EQ(d.getText(), "-={ asd }=-");
		auto c = TransformDecorator(d, ct);
		CHECK_EQ(c.getText(), "-={ -={ asd }=- }=-");
		auto e = TransformDecorator(c, et);
		CHECK_EQ(e.getText(), "-={ -={ *** }=- }=-");

		auto &l1 = removeDecorator<TransformDecorator>(e);
		CHECK_EQ(l1.getText(), "-={ -={ asd }=- }=-");
	}
	SUBCASE("dynamic") {
		Label				*l	= new SimpleLabel("asd");
		LabelTransformation *dt = new DecorateTransformation();
		LabelTransformation *ct = new DecorateTransformation();
		LabelTransformation *et = new CensorTransformation("asd");

		Label *d = new TransformDecorator(*l, *dt);
		CHECK_EQ(d->getText(), "-={ asd }=-");
		Label *c = new TransformDecorator(*d, *ct);
		CHECK_EQ(c->getText(), "-={ -={ asd }=- }=-");
		Label *e = new TransformDecorator(*c, *et);
		CHECK_EQ(e->getText(), "-={ -={ *** }=- }=-");

		auto &l1 = removeDecorator<TransformDecorator>(*e);
		CHECK_EQ(l1.getText(), "-={ -={ asd }=- }=-");
	

		delete l;
		delete dt;
		delete ct;
		delete et;
		delete d;
		delete c;
		delete e;
	}
}
