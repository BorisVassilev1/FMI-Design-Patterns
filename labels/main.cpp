#include <cassert>
#include <iostream>
#include "src/autoref.hpp"
#include "src/utils.hpp"

int main() { 

	SmartAuto a = new int(5);
	std::cout << type_name<decltype(a)::type>() << std::endl;
	std::cout << sizeof(a) << std::endl;
	a = new int(10);
	auto b = SmartAuto(new int(40));
	a = std::move(b);
	
	int *ptr = a;
	const int *cptr = a;



	int k = 10;
	SmartAuto l = k;
	std::cout << type_name<decltype(l)::type>() << std::endl;
	std::cout << sizeof(l) << std::endl;
	l.operator=(5);
	assert(k == 5);
	int &ref = l;
	assert(&ref == &k);


	SmartAuto u = 1;
	std::cout << type_name<decltype(u)::type>() << std::endl;
	std::cout << sizeof(u) << std::endl;


}
