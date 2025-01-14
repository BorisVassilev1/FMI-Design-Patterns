#include <cassert>
#include <ostream>
#include <sstream>
#include "src/concat.hpp"
#include "src/flaggedptr.hpp"
#include "src/label.hpp"
#include "src/labelDecorators.hpp"
#include "src/labelTransformations.hpp"
#include "src/utils.hpp"

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

TEST_CASE("SmartAuto") {
	SUBCASE("SmartAuto") {
		SUBCASE("ptr") {
			SmartAuto a = new int(5);
			CHECK_EQ(type_name<decltype(a)::type>(), "int*");
			CHECK_EQ(sizeof(a), 8);
			a	   = new int(10);
			auto b = SmartAuto(new int(40));
			a	   = std::move(b);

			int		  *ptr	= (int *)a;
			const int *cptr = (const int *)a;
		}
		SUBCASE("ref") {
			int		  k = 10;
			SmartAuto l = k;
			CHECK_EQ(type_name<decltype(l)::type>(), "int&");
			CHECK_EQ(sizeof(l), 8);
			l.operator=(5);
			CHECK_EQ(k, 5);
			int &ref = (int &)l;
			CHECK_EQ(&ref, &k);

			int		  v = 20;
			SmartAuto u = v;
			l			= u;
			CHECK_EQ(&(int &)l, &v);
		}
		SUBCASE("value") {
			int k = 1;
			// SmartAuto<int&> u = std::move(k);
			// CHECK_EQ(type_name<decltype(u)::type>(), "int");
			// CHECK_EQ(sizeof(u), 4);
		}
	}
}

TEST_CASE("SmartRef") {
	SUBCASE("ptr") {
		SmartRef a = new int(5);
		a		   = new int(10);
		SmartRef b = new int(40);
		a		   = std::move(b);
		CHECK_EQ(*b, 40);
		CHECK_EQ(*a, 40);
		CHECK_EQ(a.isRef, false);
		CHECK_EQ(b.isRef, true);

		int		  *ptr	= (int *)a;
		const int *cptr = (const int *)a;
	}
	SUBCASE("ref") {
		int		 k = 10;
		SmartRef l = k;
		l.operator=(5);
		CHECK_EQ(k, 5);
		int &ref = (int &)l;
		CHECK_EQ(&ref, &k);

		int		 v = 20;
		SmartRef u = v;
		l		   = u;
		CHECK_EQ(&(int &)l, &v);
	}
	SUBCASE("value") {
		int			  k = 1;
		SmartRef<int> u = std::move(k);
		k				= 2;
		CHECK_EQ(*u, 1);
	}
	SUBCASE("Vector") {
		int						   a = 1, b = 2, c = 3;
		std::vector<SmartRef<int>> v = {a, b, c};
		CHECK_EQ(*v[0], 1);
		CHECK_EQ(*v[1], 2);
		CHECK_EQ(*v[2], 3);
		auto u = std::move(v);
		CHECK_EQ(*u[0], 1);
		CHECK_EQ(*u[1], 2);
		CHECK_EQ(*u[2], 3);
	}
	SUBCASE("Vector") {
		std::vector<SmartRef<int>> v;
		v.emplace_back(new int(1));
		CHECK_EQ(v[0].isRef, false);
		CHECK_EQ(*v[0], 1);
		v.emplace_back(new int(2));
		CHECK_EQ(v[0].isRef, false);
		CHECK_EQ(v[1].isRef, false);
		CHECK_EQ(*v[0], 1);
		CHECK_EQ(*v[1], 2);
		v.emplace_back(new int(3));
		CHECK_EQ(v[0].isRef, false);
		CHECK_EQ(v[1].isRef, false);
		CHECK_EQ(v[2].isRef, false);
		CHECK_EQ(*v[0], 1);
		CHECK_EQ(*v[1], 2);
		CHECK_EQ(*v[2], 3);
		auto u = std::move(v);
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

TEST_CASE("LabelImp Creation") {
	SUBCASE("SimpleLabel") {
		SimpleLabel		   label("HELLO");
		std::ostringstream ss;
		LabelPrinter::print(Label(label), ss);
		CHECK_EQ(ss.str(), "Here is a label: HELLO\n");
	}
	SUBCASE("ProxyLabel read once") {
		std::istringstream iss("HELLO");
		auto			   label = ProxyLabel(iss);
		std::ostringstream oss;
		LabelPrinter::print(Label(label), oss);
		CHECK_EQ(oss.str(), "Here is a label: HELLO\n");

		LabelPrinter::print(Label(label), oss);
		CHECK_EQ(oss.str(), "Here is a label: HELLO\nHere is a label: HELLO\n");
	}
	SUBCASE("ProxyLabel repeat read") {
		std::istringstream iss("HELLO THERE");
		auto			   label = ProxyLabel(iss, 1);
		std::ostringstream oss;
		LabelPrinter::print(Label(label), oss);
		CHECK_EQ(oss.str(), "Here is a label: HELLO\n");

		LabelPrinter::print(Label(label), oss);
		CHECK_EQ(oss.str(), "Here is a label: HELLO\nHere is a label: THERE\n");
	}
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
	SUBCASE("Composite") {
		auto cap = CapitalizeTransformation();
		auto dec = DecorateTransformation();
		auto rep = ReplaceTransformation("abc", "def");

		CHECK_EQ(CompositeTransformation({cap, dec, rep}).apply("abc def"), "-={ Abc def }=-");
		CHECK_EQ(CompositeTransformation({rep, cap, dec}).apply("abc def"), "-={ Def def }=-");
	}
}

TEST_CASE("Decorators") {
	SUBCASE("LabelTransformDecorator") {
		SUBCASE("Static") {
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
		}
		SUBCASE("dynamic") {
			auto l	= new SimpleLabel("asd");
			auto dt = new DecorateTransformation();
			auto ct = new CapitalizeTransformation();
			auto et = new CensorTransformation("asd");

			auto d = new TransformDecorator(l, dt);
			CHECK_EQ(d->getText(), "-={ asd }=-");

			auto c = TransformDecorator(*l, ct);
			CHECK_EQ(c.getText(), "Asd");

			auto e = TransformDecorator(d, et);
			CHECK_EQ(e.getText(), "-={ *** }=-");
		}
	}
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

		Label label(e);
		CHECK(d == c);
		CHECK(c != e);

		SUBCASE("remove templated") {
			removeDecorator<TransformDecorator>(label);
			CHECK_EQ(label.getText(), "-={ -={ asd }=- }=-");
		}
		SUBCASE("remove by value") {
			removeDecorator(label, &c);
			CHECK_EQ(label.getText(), "-={ *** }=-");
		}
		SUBCASE("remove by value") {
			removeDecorator(label, &e);
			CHECK_EQ(label.getText(), "-={ -={ asd }=- }=-");
		}
		SUBCASE("remove by type") {
			removeDecoratorByType(label, &c);
			CHECK_EQ(label.getText(), "-={ -={ asd }=- }=-");
		}
	}
	SUBCASE("dynamic") {
		LabelImp			*l	= new SimpleLabel("asd");
		LabelTransformation *dt = new DecorateTransformation();
		LabelTransformation *ct = new DecorateTransformation();
		LabelTransformation *et = new CensorTransformation("asd");

		l = new TransformDecorator(l, dt);
		CHECK_EQ(l->getText(), "-={ asd }=-");
		l = new RandomTransformationDecorator(l, {*dt});
		l = new TransformDecorator(l, ct);
		CHECK_EQ(l->getText(), "-={ -={ -={ asd }=- }=- }=-");
		l = new TransformDecorator(l, et);
		CHECK_EQ(l->getText(), "-={ -={ -={ *** }=- }=- }=-");

		Label label(l);

		SUBCASE("remove templated") {
			removeDecorator<TransformDecorator>(label);
			CHECK_EQ(label.getText(), "-={ -={ -={ asd }=- }=- }=-");

			removeDecorator<RandomTransformationDecorator>(label);
			CHECK_EQ(label.getText(), "-={ -={ asd }=- }=-");
		}
		SUBCASE("remove by type") {
			SmartRef toRemove = new RandomTransformationDecorator(nullptr, {});
			removeDecoratorByType(label, &toRemove);
			CHECK_EQ(label.getText(), "-={ -={ *** }=- }=-");
		}
	}
	SUBCASE("demonstration") {
		Label l(new SimpleLabel("abcd efgh ijkl mnop"));
		l.addDecorator<TransformDecorator>(new CensorTransformation("abcd"));
		l.addDecorator<TransformDecorator>(new CensorTransformation("mnop"));
		CHECK_EQ(l.getText(), "**** efgh ijkl ****");

		SmartRef<LabelDecoratorBase> whatToRemove = new TransformDecorator(nullptr, new CensorTransformation("abcd"));
		removeDecorator(l, &*whatToRemove);
		CHECK_EQ(l.getText(), "abcd efgh ijkl ****");
	}
	SUBCASE("demonstration no template") {
		Label l(new SimpleLabel("abcd efgh ijkl mnop"));
		l = new TransformDecorator(std::move(l.getImp()), new CensorTransformation("abcd"));
		l = new TransformDecorator(std::move(l.getImp()), new CensorTransformation("mnop"));
		CHECK_EQ(l.getText(), "**** efgh ijkl ****");

		SmartRef<LabelDecoratorBase> whatToRemove = new TransformDecorator(nullptr, new CensorTransformation("abcd"));
		removeDecorator(l, &*whatToRemove);
		CHECK_EQ(l.getText(), "abcd efgh ijkl ****");
	}
}

TEST_CASE("Label") {
	SUBCASE("SimpleLabel") {
		SimpleLabel l("asd");
		Label		label(l);
		CHECK_EQ(label.getText(), "asd");
	}
	SUBCASE("HelpLabel") {
		HelpLabel l(new SimpleLabel("asd"), "help");
		CHECK_EQ(l.getText(), "asd");
		CHECK_EQ(l.getHelpText(), "help");
	}
	SUBCASE("RichLabel") {
		RichLabel l("asd", Color{1, 2, 3}, "Arial", 10);
		HelpLabel label(l, "some helpful text");
		CHECK_EQ(label.getText(), "asd");
		CHECK_EQ(label.getHelpText(), "some helpful text");
	}
}

TEST_CASE("LabelBuilder") {
	LabelBuilder b;
	SUBCASE("no imp") { CHECK_THROWS_AS(b.build(), std::runtime_error); }
	SUBCASE("no label") {
		b.setImp(new SimpleLabel("asd"));
		auto label = b.build();
		CHECK_EQ(label->getText(), "asd");
	}
	SUBCASE("with label") {
		b.setImp(new SimpleLabel("asd"));
		b.setLabel(Label(nullptr));
		auto label = b.build();
		CHECK_EQ(label->getText(), "asd");
	}
	SUBCASE("with decorators") {
		b.setImp(new SimpleLabel("asd"));
		b.addDecorator<TransformDecorator>(new CapitalizeTransformation());
		b.addDecorator<TransformDecorator>(new DecorateTransformation());
		b.addDecorator<TransformDecorator>(new CensorTransformation("sd"));
		auto label = b.build();
		CHECK_EQ(label->getText(), "-={ A** }=-");
	}
}

TEST_CASE("CensorTransformationFactory") {
	CensorTransformationFactory f;
	SUBCASE("basic") {
		SmartRef<LabelTransformation> t = f.create("abc");
		CHECK_EQ(t->apply("abc def abcdef"), "*** def ***def");
		CHECK_EQ(t->apply("aba def ababa"), "aba def ababa");

		SmartRef<LabelTransformation> t2 = new CensorTransformation("abc");
		CHECK_EQ(*t, *t2);
		CHECK_EQ(t, t2);
	}

	SUBCASE("check comparision") {
		Label l(new SimpleLabel("abcd efgh ijkl mnop"));
		l = new TransformDecorator(std::move(l.getImp()), f.create("abcd"));
		l = new TransformDecorator(std::move(l.getImp()), f.create("mnop"));
		CHECK_EQ(l.getText(), "**** efgh ijkl ****");

		SmartRef<LabelDecoratorBase> whatToRemove = new TransformDecorator(nullptr, new CensorTransformation("abcd"));
		removeDecorator(l, &*whatToRemove);
		CHECK_EQ(l.getText(), "abcd efgh ijkl ****");
	}
};
