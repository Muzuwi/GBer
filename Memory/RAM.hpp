#pragma once
#include <vector>
#include <fstream>

namespace RAM{
	extern std::vector<char> RAM;

	char read(char16_t);
	bool write(char16_t, char);
	bool copy(char16_t, char16_t, unsigned int);
	bool move(char16_t, char16_t, unsigned int);	
	bool insert(std::vector<char>, char16_t, unsigned int);
	bool dump(std::string);
}