#include <iostream>
#include "common/common.hpp"

auto main() -> int
{
	common::test();
	std::cout << "Hello from client" << std::endl;
	return 0;
}