#pragma once
#include <vector>
#include <fstream>

namespace RAM{
	extern std::vector<unsigned char> RAM;

	char read(char16_t);
	char16_t readSeq(char16_t address, unsigned int count);
	bool write(char16_t, unsigned char);
	bool copy(char16_t, char16_t, unsigned int);
	bool move(char16_t, char16_t, unsigned int);	
	bool insert(std::vector<unsigned char>, char16_t, unsigned int);
	bool dump(std::string);
}