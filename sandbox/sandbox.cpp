#include "../include/ArgFormatter/ArgFormatter.h"

#include <iostream>

//! NOTE: This looks to be working as intended AND is truly a header-only library now with the movement of the 
//!              ArgFormatter.cpp functions into the ArgFormatterImpl.h file. Now there's only a couple steps left to do.
// TODO: Port over the test suite and ensure that it all works still. Organize the code a little bit and then call it done. 

int main() {

	std::cout << "Hello From Sandbox!\n\n";

	std::cout << formatter::format("This Is A Test For The Inclusion Of ArgFormatter.h In Sandbox\n");
	std::string msg{ "Assuming All Went Well, This Text Should Be Substituted In.\n" };
	std::cout << formatter::format("{}", msg);
	std::cout << "\n";

	std::vector<char> cont;
	formatter::format_to(std::back_inserter(cont), "This Should Be A Formatted Message *INTO* A Container:\n{}", msg);
	std::cout << std::string_view(cont.begin(), cont.end());

}