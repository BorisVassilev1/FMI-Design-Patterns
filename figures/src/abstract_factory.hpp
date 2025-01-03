#pragma once

#include <cctype>

#include "figure_factory.hpp"

class AbstractFigureFactory : public LineIstreamFactory<FigureFactory, "FigureFactory", true, std::string> {
	using LineIstreamFactory::LineIstreamFactory;

   public:
};

