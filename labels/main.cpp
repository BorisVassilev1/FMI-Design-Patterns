#include <iostream>
#include <ranges>
#include <vector>
#include <string_view>

int main() { 
	using namespace std::literals;
	std::cout << "Hello World!!" << std::endl;

	std::vector v{"This"sv, "is"sv, "a"sv, "test."sv};
	auto joined = v | std::views::join;
	for (auto c : joined)
        std::cout << c;
    std::cout << '\n';

}
