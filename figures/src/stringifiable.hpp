#pragma once

#include <string>

class Stringifiable {
   public:
	virtual explicit operator std::string() const = 0;
	virtual ~Stringifiable() {};
};
