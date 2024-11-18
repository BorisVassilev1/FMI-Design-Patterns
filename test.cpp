
#include <algorithm>
#include <cstdio>
#include <sstream>
#include <stdexcept>
#include "src/abstract_factory.hpp"
#include "src/figure_factory.hpp"
#include "src/type_registry.hpp"
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>

#include "src/figures.hpp"

TEST_CASE("doctest sanity check") { CHECK(1 == 1); }

TEST_CASE("figure constructors") {
	Triangle  t(3, 4, 5);
	Circle	  c(1);
	Rectangle r(1, 2);
}

TEST_CASE("figure invalid arguments") {
	CHECK_THROWS_AS(Triangle(1, 2, 4), std::invalid_argument);
	CHECK_THROWS_AS(Triangle(1, 2, 0), std::invalid_argument);
	CHECK_THROWS_AS(Triangle(1, 0, 2), std::invalid_argument);
	CHECK_THROWS_AS(Triangle(0, 2, 1), std::invalid_argument);

	CHECK_THROWS_AS(Triangle(1, 2, -1), std::invalid_argument);
	CHECK_THROWS_AS(Triangle(1, -2, 1), std::invalid_argument);
	CHECK_THROWS_AS(Triangle(-1, 2, 1), std::invalid_argument);

	CHECK_THROWS_AS(Circle(0), std::invalid_argument);
	CHECK_THROWS_AS(Circle(-1), std::invalid_argument);

	CHECK_THROWS_AS(Rectangle(1, 0), std::invalid_argument);
	CHECK_THROWS_AS(Rectangle(0, 1), std::invalid_argument);
	CHECK_THROWS_AS(Rectangle(-1, 1), std::invalid_argument);
	CHECK_THROWS_AS(Rectangle(1, -1), std::invalid_argument);
}

TEST_CASE("figure invalid arguments NAN") {
	CHECK_THROWS_AS(Triangle(1, 2, NAN), std::invalid_argument);
	CHECK_THROWS_AS(Triangle(1, NAN, 2), std::invalid_argument);
	CHECK_THROWS_AS(Triangle(NAN, 2, 1), std::invalid_argument);

	CHECK_THROWS_AS(Circle(NAN), std::invalid_argument);

	CHECK_THROWS_AS(Rectangle(1, NAN), std::invalid_argument);
	CHECK_THROWS_AS(Rectangle(NAN, 1), std::invalid_argument);
}

TEST_CASE("figure overflow") {
	CHECK_THROWS_AS(Triangle(1, 2, std::numeric_limits<float>::max()), std::overflow_error);
	CHECK_THROWS_AS(Triangle(1, std::numeric_limits<float>::max(), 2), std::overflow_error);
	CHECK_THROWS_AS(Triangle(std::numeric_limits<float>::max(), 2, 1), std::overflow_error);
	CHECK_THROWS_AS(Triangle(std::numeric_limits<float>::max(), std::numeric_limits<float>::max(),
							 std::numeric_limits<float>::max()),
					std::overflow_error);
}

TEST_CASE("perimeters") {
	CHECK_EQ(Triangle(3, 4, 5).perimeter(), 12);
	CHECK_EQ(Rectangle(1, 2).perimeter(), 6);
	CHECK_LT(std::abs(Circle(1).perimeter() - 2 * M_PI), 0.01);
}

TEST_CASE("clone triangle") {
	Figure* f = new Triangle(3, 4, 5);
	Figure* t = (Figure*)f->clone();
	CHECK_EQ(f->perimeter(), t->perimeter());
	delete f;
	delete t;
}

TEST_CASE("clone Circle") {
	Figure* f = new Circle(1);
	Figure* t = (Figure*)f->clone();
	CHECK_EQ(f->perimeter(), t->perimeter());
	delete f;
	delete t;
}

TEST_CASE("clone rectangle") {
	Figure* f = new Rectangle(1, 2);
	Figure* t = (Figure*)f->clone();
	CHECK_EQ(f->perimeter(), t->perimeter());
	delete f;
	delete t;
}

TEST_CASE("stringify figures") {
	CHECK_EQ((std::string)Triangle(3, 4, 5), "Triangle 3 4 5");
	CHECK_EQ((std::string)Circle(1), "Circle 1");
	CHECK_EQ((std::string)Rectangle(1, 2), "Rectangle 1 2");
}

TEST_CASE("inheritance") {
	auto v = TypeRegistry::getChildren().at("Figure");
	CHECK_NE(std::find(v.begin(), v.end(), "Triangle"), v.end());
	CHECK_NE(std::find(v.begin(), v.end(), "Rectangle"), v.end());
	CHECK_NE(std::find(v.begin(), v.end(), "Circle"), v.end());

	v = TypeRegistry::getChildren().at("Cloneable");
	CHECK_NE(std::find(v.begin(), v.end(), "Figure"), v.end());
	CHECK_NE(std::find(v.begin(), v.end(), "Triangle"), v.end());
	CHECK_NE(std::find(v.begin(), v.end(), "Rectangle"), v.end());
	CHECK_NE(std::find(v.begin(), v.end(), "Circle"), v.end());

	v = TypeRegistry::getChildren().at("FigureFactory");
	CHECK_NE(std::find(v.begin(), v.end(), "RandomFigureFactory"), v.end());
	CHECK_NE(std::find(v.begin(), v.end(), "IstreamFigureFactory"), v.end());
	CHECK_NE(std::find(v.begin(), v.end(), "FileFigureFactory"), v.end());
	CHECK_NE(std::find(v.begin(), v.end(), "STDINFigureFactory"), v.end());

	v = TypeRegistry::getParents().at("STDINFigureFactory");
	CHECK_NE(std::find(v.begin(), v.end(), "FigureFactory"), v.end());
	CHECK_NE(std::find(v.begin(), v.end(), "IstreamFigureFactory"), v.end());
}

TEST_CASE("Random Factory") {
	srand(42);
	RandomFigureFactory ff;
	for (int i = 0; i < 100; ++i) {
		Figure* f = ff.create();
		CHECK(f->perimeter() >= 0);
		CHECK(f->perimeter() <= 63);
		delete f;
	}
}

TEST_CASE("Istream Factory") {
	std::stringstream	 ss("Triangle 3.0 4.00 5e0\nCircle 1\nRectangle 2 3");
	IstreamFigureFactory ff(ss);
	Figure*				 f;

	f = ff.create();
	CHECK_EQ((std::string)*f, "Triangle 3 4 5");
	delete f;

	f = ff.create();
	CHECK_EQ((std::string)*f, "Circle 1");
	delete f;

	f = ff.create();
	CHECK_EQ((std::string)*f, "Rectangle 2 3");
	delete f;
}

TEST_CASE("File Figure Factory") {
	std::ofstream file("../tmp/test_file.txt");
	file << "Triangle 3 4 5\nCircle 1 \n Rectangle 2 3";
	file.close();

	FileFigureFactory ff("../tmp/test_file.txt");
	Figure*			  f;

	f = ff.create();
	CHECK_EQ((std::string)*f, "Triangle 3 4 5");
	delete f;

	f = ff.create();
	CHECK_EQ((std::string)*f, "Circle 1");
	delete f;

	f = ff.create();
	CHECK_EQ((std::string)*f, "Rectangle 2 3");
	delete f;
}

TEST_CASE("Istream Factory exceptions") {
	Figure* f = nullptr;
	CHECK_EQ(f = figureFromString(""), nullptr);
	CHECK_THROWS_AS(f = figureFromString("asdj"), std::runtime_error);
	CHECK_THROWS_AS(f = figureFromString("Random"), std::runtime_error);
	CHECK_THROWS_AS(f = figureFromString("STDIN"), std::runtime_error);

	CHECK_THROWS_AS(f = figureFromString("Triangle "), std::runtime_error);
	CHECK_THROWS_AS(f = figureFromString("Triangle 1 "), std::runtime_error);
	CHECK_THROWS_AS(f = figureFromString("Triangle 1 2"), std::runtime_error);

	CHECK_THROWS_AS(f = figureFromString("Circle "), std::runtime_error);

	CHECK_THROWS_AS(f = figureFromString("Rectangle "), std::runtime_error);
	CHECK_THROWS_AS(f = figureFromString("Rectangle 1"), std::runtime_error);

	CHECK_THROWS_AS(f = figureFromString("Triangle 1 2 3"), std::invalid_argument);
}

TEST_CASE("Abstract Factory") {
	std::ifstream file("../tmp/test_file.txt", std::ios_base::trunc);
	file.close();

	std::istringstream is("Random -1\nSTDIN -1\n File ../tmp/test_file.txt");
	std::ostringstream os;
	os.setstate(std::ios_base::badbit);

	AbstractFigureFactory aff(is);

	FigureFactory* f;
	f = aff.create();
	CHECK_NE(dynamic_cast<RandomFigureFactory*>(f), nullptr);
	delete f;

	f = aff.create();
	CHECK_NE(dynamic_cast<STDINFigureFactory*>(f), nullptr);
	delete f;

	f = aff.create();
	CHECK_NE(dynamic_cast<FileFigureFactory*>(f), nullptr);
	delete f;
}
