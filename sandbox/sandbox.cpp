#include "../include/ArgFormatter/ArgFormatter.h"

#include <iostream>

int main() {
	std::cout << "Hello From Sandbox!\n";
	std::cout << formatter::format("This Is A Test For The Inclusion Of ArgFormatter.h In Sandbox\n");
	auto msg { "Assuming All Went Well, This Text Should Be Substituted In.\n" };
	std::cout << formatter::format("{}", msg);
}