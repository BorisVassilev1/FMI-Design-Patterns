#pragma once
#include <cmath>
#include <format>
#include <stdexcept>
#include "cloneable.hpp"
#include "stringifiable.hpp"
#include "type_registry.hpp"

INHERIT(Cloneable, Figure);
INHERIT(Stringifiable, Figure);
class Figure : public Cloneable, public Stringifiable {
   public:
	virtual float	perimeter() const	   = 0;
	virtual Figure *clone() const override = 0;
	virtual ~Figure() {};
};

INHERIT(Figure, Triangle);
class Triangle : public Figure {
	float a, b, c;

   public:
	Triangle(float a, float b, float c) : a(a), b(b), c(c) {
		if (std::isnan(a) || std::isnan(b) || std::isnan(c)) throw std::invalid_argument("sides cannot be NaN");
		if (a <= 0 || b <= 0 || c <= 0) throw std::invalid_argument("sides cannot be less than zero");
		if (a > std::numeric_limits<float>::max() - b - c) throw std::overflow_error("overflow error");
		if (b > std::numeric_limits<float>::max() - a - c) throw std::overflow_error("overflow error");
		if (c > std::numeric_limits<float>::max() - a - b) throw std::overflow_error("overflow error");
		if (a + b <= c || b + c <= a || a + c <= b) throw std::invalid_argument("triangle inequality error");
	}
	float	  perimeter() const override { return a + b + c; }
	Triangle *clone() const override { return new Triangle(*this); }
	explicit  operator std::string() const override { return std::format("Triangle {} {} {}", a, b, c); }
};

INHERIT(Figure, Circle);
class Circle : public Figure {
	float r;

   public:
	Circle(float r) : r(r) {
		if (std::isnan(r)) throw std::invalid_argument("radius cannot be NaN");
		if (r <= 0) throw std::invalid_argument("radius cannot be less than zero");
		if (r > std::numeric_limits<float>::max() / 2.f / M_PI) throw std::overflow_error("overflow error");
	}
	float	 perimeter() const override { return 2 * M_PI * r; }
	Circle	*clone() const override { return new Circle(*this); }
	explicit operator std::string() const override { return std::format("Circle {}", r); }
};

INHERIT(Figure, Rectangle);
class Rectangle : public Figure {
	float a, b;

   public:
	Rectangle(float a, float b) : a(a), b(b) {
		if (std::isnan(a) || std::isnan(b)) throw std::invalid_argument("sides cannot be NaN");
		if (a <= 0 || b <= 0) throw std::invalid_argument("sides cannot be less than zero");
		if (a > std::numeric_limits<float>::max() / 2.f - b) throw std::overflow_error("overflow error");
		if (b > std::numeric_limits<float>::max() / 2.f - a) throw std::overflow_error("overflow error");
	}
	float	   perimeter() const override { return 2 * (a + b); }
	Rectangle *clone() const override { return new Rectangle(*this); }
	explicit   operator std::string() const override { return std::format("Rectangle {} {}", a, b); }
};

JOB(register_figure_constructors,{
	TypeRegistry::registerConstructorForType<Triangle, "Triangle", float, float, float>();
	TypeRegistry::registerConstructorForType<Circle, "Circle", float>();
	TypeRegistry::registerConstructorForType<Rectangle, "Rectangle", float, float>();
});
